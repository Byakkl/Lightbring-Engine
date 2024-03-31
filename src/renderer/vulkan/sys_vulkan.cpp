#define STB_IMAGE_IMPLEMENTATION
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <stdexcept>
#include <iostream>
#include <cstdlib>
#include <vector>
#include <cstring>
#include <optional>
#include <set>
#include <limits>
#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <chrono>

#include "debug_vulkan.h"
#include "sys_vulkan.h"
#include "util_vulkan.h"
#include "../../fileio/util_io.h"
#include "structs_model.h"
#include "../../core/structs.h"

void VulkanRenderer::initialize(){
    initWindow();
    initVulkan();
}

bool VulkanRenderer::render(Camera* camera, std::vector<Object*> objects){
    if(glfwWindowShouldClose(window))
        return false;

    glfwPollEvents();

    //Wait for the presentation fence for the frame to complete
    vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

    //TODO: Promote this to class member
    //Create the batch render fence
    VkFence renderFence;
    //TODO: Move this to sync object creation method
    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    vkCreateFence(device, &fenceInfo, nullptr, &renderFence);

    //Update camera buffers

    size_t arrSize = objects.size();
    std::vector<VkWriteDescriptorSet> descriptorWrites;
    for(int i = 0; i < arrSize; i += MAX_OBJECT_DESCRIPTOR_SETS){
        //Wait for Fence to indicate renders are done
        vkWaitForFences(device, 1, &renderFence, VK_TRUE, UINT64_MAX);
        
        //Generate Descriptor Set updates
        for(int idx = 0; idx < MAX_OBJECT_DESCRIPTOR_SETS && (i * MAX_OBJECT_DESCRIPTOR_SETS + idx) < arrSize; idx++)
            updateDescriptorSet(descriptorWrites, objectDescriptorSets[idx], objects[i * MAX_OBJECT_DESCRIPTOR_SETS + idx]);
        
        //Apply the updates
        vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()),descriptorWrites.data(), 0, nullptr);
        
        //TODO: Record command buffer

        //Submit the render buffer to queue
        if(vkQueueSubmit(graphicsQueue, 1, &submitInfo, renderFence) != VK_SUCCESS)
            throw std::runtime_error("Failed to submit draw command buffer");
    }
    //Wait for the final render to finish
    vkWaitForFences(device, 1, &renderFence, VK_TRUE, UINT64_MAX);
    //TODO: Present the frame
    drawFrame();
    
    return true;
}

void VulkanRenderer::cleanup(){
    //Wait for the logical device to finish operations before exiting main loop
    vkDeviceWaitIdle(device);

    //Clean up the swap chain and its dependent objects
    cleanupSwapChain();

    //Clean up the texture sampler
    vkDestroySampler(device, textureSampler, nullptr);

    //Clean up the uniform buffers
    for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++){
        vkDestroyBuffer(device, uniformBuffers[i], nullptr);
        vkFreeMemory(device, uniformBuffersMemory[i], nullptr);
    }

    //Clean up the descriptor pool
    vkDestroyDescriptorPool(device, descriptorPool, nullptr);

    //Clean up the descriptor set layout
    vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);

    //Clean up sync objects
    for(int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++){
        vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
        vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
        vkDestroyFence(device, inFlightFences[i], nullptr);
    }

    //Clean up the command pools
    vkDestroyCommandPool(device, graphicsCommandPool, nullptr);
    vkDestroyCommandPool(device, transferCommandPool, nullptr);

    //Clean up the graphics pipeline
    vkDestroyPipeline(device, graphicsPipeline, nullptr);

    //Clean up the graphics pipeline layout
    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
        
    //Clean up the render pass
    vkDestroyRenderPass(device, renderPass, nullptr);

    //Clean up the logical device
    vkDestroyDevice(device, nullptr);

    //If validation layers are enabled clean up the debug messenger instance
    if(enableValidationLayers)
        DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);

    //Clean up with window surface
    vkDestroySurfaceKHR(instance, surface, nullptr);

    //Clean up the Vulkan instance
    vkDestroyInstance(instance, nullptr);
        
    //Clean up the created window
    glfwDestroyWindow(window);
        
    //Clean up GLFW
    glfwTerminate();
}

void VulkanRenderer::createTexture(Texture* image){
    //Create the container for the Vulkan handles
    ImageData imageData;
    //Create the Vulkan image
    createTextureImage(image, &imageData);
    //Create the image view for the texture
    createImageView(&imageData, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);

    //Pass the Vulkan handle container to the image object
    image->rendererData = &imageData;
}

void VulkanRenderer::unloadTexture(Texture* image){
    if(image->rendererData == nullptr)
        return;

    //Cast to the Vulkan data container and invoke the cleanup method to release the memory
    static_cast<ImageData*>(image->rendererData)->cleanup(device);

    //Null out the pointer as all data is cleaned
    image->rendererData = nullptr;
}

void VulkanRenderer::uploadMesh(Mesh* mesh){
    //Create the container for the Vulkan handles
    MeshData meshData;
    //Create a vertex buffer
    createVertexBuffer(mesh, &meshData);
    //Create an index buffer
    createIndexBuffer(mesh, &meshData);

    //Pass the Vulkan handle container to the mesh object
    mesh->rendererData = &meshData;
}

void VulkanRenderer::unloadMesh(Mesh* mesh){
    if(mesh->rendererData == nullptr)
        return;

    //Cast to the Vulkan data container and invoke the cleanup method to release the memory
    static_cast<MeshData*>(mesh->rendererData)->cleanup(device);

    //Null out the pointer as all data is cleaned
    mesh->rendererData = nullptr;
}

void VulkanRenderer::registerCamera(Camera* camera){
    //If there is already data tracked then this camera is already registered
    if(camera->rendererData != nullptr)
        return;
    
    //TODO: Allocate buffer data set
    //TODO: Allocate descriptor set
    //TODO: Assign CameraData to camera->rendererData
}

void VulkanRenderer::unregisterCamera(Camera* camera){
    //If there is no data tracked then this camera is already unregistered
    if(camera->rendererData == nullptr)
        return;
    
    //TODO: Free descriptor set from pool


    //Free buffer data
    ((CameraData*)camera->rendererData)->cleanup(device);

    //Clear the pointer to the cleared data structure
    camera->rendererData = nullptr;
}

std::vector<const char*> VulkanRenderer::getRequiredExtensions(){
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    //If validation layers are enabled add the Debug Utilities extension to the list
    if(enableValidationLayers){
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    #ifdef DEBUG_EXT_NAME_LOGGING
    //DEBUG; outputs the names of the extensions to console
    for(const auto& name : extensions)
    {
        std::cout << "Extension: " << name << std::endl;
    }
    #endif

    return extensions;
}

bool VulkanRenderer::checkValidationLayerSupport(){
    //Get the count of the available layers
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    //Get the list of available layers
    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    #ifdef DEBUG_VALIDATION_LAYER_NAME_LOGGING
    //DEBUG; outputs the names of the available layers to console
    for(const auto& layers : availableLayers)
    {
        std::cout << "Layer: " << layers.layerName << std::endl;
    }
    #endif

    //Iterate through the predefined list of desired validation layers
    for(const char* layerName : validationLayers){
        bool layerFound = false;

        //Iterate through the list of available validatino layers, exiting if a match is found
        for(const auto& layerProperties : availableLayers){
            if(strcmp(layerName, layerProperties.layerName) == 0){
                layerFound = true;
                break;
            }
        }

        //If any desired layers are missing then we return false
        if(!layerFound)
            return false;
    }

    return true;
}

void VulkanRenderer::initWindow(){
        //Initialize GLFW
        glfwInit();
        //Disable creation of OpenGL context
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        //Create a new window with resolution 800x600 and name Vulkan
        //4th parameter can specify a monitor to open on
        //5th parameter is relevant to OpenGL
        window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
        //Set a pointer to this instance of VulkanRenderer that can be used in the callback
        glfwSetWindowUserPointer(window, this);
        //Set a callback to be invoked by GLFW when the window is resized
        glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
    }

void VulkanRenderer::createInstance(){
    //If validation layers are enabled but any are unsupported then throw an error
    if(enableValidationLayers && !checkValidationLayerSupport()){
        throw std::runtime_error("Validation layers requested, but not available");
    }

    //Create an info structure about the application; technically optional
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Application Name";
    appInfo.applicationVersion = VK_MAKE_VERSION(0,0,1);
    appInfo.pEngineName = "Lightbring Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(0,1,0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    //Required structure to inform the Vulkan driver about gloabl extensions and validation layers
    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    //Stores the number of extensions required and what they are
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;

    //Fetch the required extensions
    auto extensions = getRequiredExtensions();

    //Pass the extension info to the Vulkan info structure
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());;
    createInfo.ppEnabledExtensionNames = extensions.data();;

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    if(enableValidationLayers){
        //Set the validation layer information
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();

        //Tells Vulkan to create a debug messenger instance that will be created alongside the vulkan instance
        //It will be automatically cleaned up when the instance is destroyed.
        //This allows messages to be cause during the instance creation/cleanup steps
        populateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
    }
    else {
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = nullptr;
    }

    //Create the Vulkan instance; 
    //Passes the creation info, no custom allocator callback and the pointer ref to the instance
    VkResult instanceResult = vkCreateInstance(&createInfo, nullptr, &instance);
    if(instanceResult != VK_SUCCESS){
        throw std::runtime_error("Failed to create Vulkan instance. Result was "+ std::to_string(instanceResult));
    }
}
 
void VulkanRenderer::initVulkan(){
    createInstance();
    setupDebugMessenger();
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
    createSwapChain();
    createImageViews();
    createRenderPass();
    createDescriptorSetLayout();
    createGraphicsPipeline();
    createCommandPool(graphicsCommandPool, queueFamilies[0]);
    createCommandPool(transferCommandPool, queueFamilies[1]);
    createDepthResources();
    createFrameBuffers();
    createTextureSampler();
    createUniformBuffers();

    //Create the descriptor pool for object descriptor sets
    createDescriptorPool(objectDescriptorPool, MAX_OBJECT_DESCRIPTOR_SETS, std::vector<VkDescriptorPoolSize>{
        //One transform uniform per object descriptor set
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        static_cast<uint32_t>(MAX_OBJECT_DESCRIPTOR_SETS)},
        //One texture sampler per object descriptor set
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        static_cast<uint32_t>(MAX_OBJECT_DESCRIPTOR_SETS)}
    });
    //Pre-allocate the descriptor sets for objects in the scene
    //createDescriptorSets(objectDescriptorPool, MAX_MODEL_DESCRIPTOR_SETS, std::vector<VkDescriptorSetLayout>{MAX_MODEL_DESCRIPTOR_SETS, objectDescriptorSetLayout}, objectDescriptorSets);

    //Create the descriptor pool for camera descriptor sets
    createDescriptorPool(cameraDescriptorPool, MAX_CAMERA_DESCRIPTOR_SETS, std::vector<VkDescriptorPoolSize>{
        //One matrix uniform buffer per camera descriptor set
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        static_cast<uint32_t>(MAX_CAMERA_DESCRIPTOR_SETS)}
    });
    
    //createDescriptorSets(cameraDescriptorPool, MAX_CAMERA_DESCRIPTOR_SETS, std::vector<VkDescriptorSetLayout>{MAX_CAMERA_DESCRIPTOR_SETS, cameraDescriptorSetLayout}, cameraDescriptorSets);

    createCommandBuffers(graphicsCommandPool, graphicsCommandBuffers);
    createCommandBuffers(transferCommandPool, transferCommandBuffers);
    createSyncObjects();
}

bool VulkanRenderer::hasStencilComponent(VkFormat format){
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

VkFormat VulkanRenderer::findDepthFormat(){
    return findSupportedFormat(
        {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
    );
}

VkFormat VulkanRenderer::findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features){
    for(VkFormat format : candidates){
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

        if(tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
            return format;
        else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
            return format;
    }

    throw std::runtime_error("Failed to find supported format");
}

void VulkanRenderer::createDepthResources(){
    ImageData depthImage;
    VkFormat depthFormat = findDepthFormat();
    createImage(swapChainExtent.width, 
        swapChainExtent.height, 
        depthFormat, 
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        &depthImage);

    createImageView(&depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);

    transitionImageLayout(depthImage.image, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
}

void VulkanRenderer::createTextureSampler(){
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    //Specify how to interpret texels that are magnified
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    //Specify how to interpret texels that are minified
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    //Specify the sampling mode when outside of image borders
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    //Enable Anisotropic filtering
    samplerInfo.anisotropyEnable = VK_TRUE;
    
    //Fetch the physical device properties
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(physicalDevice, &properties);
    //Use the physical device properties to determine the max anisotropy filtering
    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;

    //Specify the color that is returned when samping beyond image border
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    //Spcify if texture coordinates should be [0, width/height] or [0,1]
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    //Specify if a comparison operation is enabled
    samplerInfo.compareEnable = VK_FALSE;
    //Specify the type ofcomparison operation
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    //Specify mipmapping settings
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;

    if(vkCreateSampler(device, &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS)
        throw std::runtime_error("Failed to create texture sampler");
}

void VulkanRenderer::createImageView(ImageData* imageData, VkFormat format, VkImageAspectFlags aspectFlags){
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = imageData->image;
    //Determines what it treats the image as, in this case 2D. Can be used to treat it as 1D, 2D, 3D and cube maps
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    //Allows for swizzling of color channels
    viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    //Subresource Range field informs of what the image's purpose is.
    //Used as a color target
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    //No mip mapping
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    //No layers
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;
    
    VkImageView imageView;
    if(vkCreateImageView(device, &viewInfo, nullptr, &imageView) != VK_SUCCESS)
        throw std::runtime_error("Failed to create texture image view");

    imageData->imageViews.push_back(imageView);
}

void VulkanRenderer::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height){
    //Create and Begin the command buffer
    VkCommandBuffer commandBuffer = beginSingleTimeCommands(transferCommandPool);

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    region.imageOffset = {0, 0, 0};
    region.imageExtent = {
        width,
        height,
        1
    };

    //Add a CopyBufferToImage command
    vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    //End and submit the buffer to the queue
    endSingleTimeCommands(commandBuffer, transferQueue, transferCommandPool);
}

void VulkanRenderer::transitionImageLayout(VkImage& image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout){
    VkAccessFlags srcAccessMask, dstAccessMask;
    VkPipelineStageFlags sourceStage, destinationStage;
    VkCommandPool commandPool;
    VkQueue commandQueue;
    VkImageAspectFlags aspectMask;

    if(newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL){
        aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

        if(hasStencilComponent(format))
            aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
    }
    else
        aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    //Determine layout access masks, stages and which command pool/queue to use
    if(oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL){
        srcAccessMask = 0;
        dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;

        //Use the transfer pool as the target layout is for moving data
        commandPool = transferCommandPool;
        commandQueue = transferQueue;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL){
        srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

        //Use the graphics pool as the target layout is for shader usage
        commandPool = graphicsCommandPool;
        commandQueue = graphicsQueue;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL){
        srcAccessMask = 0;
        dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;

        commandPool = graphicsCommandPool;
        commandQueue = graphicsQueue;
    }
    else
        throw std::invalid_argument("Unsupported layout transition");

    //Create and Begin the command buffer
    VkCommandBuffer commandBuffer = beginSingleTimeCommands(commandPool);

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    //Specify the layout to transition from
    //Can use VK_IMAGE_LAYOUT_UNDEFINED if existing contents are irrelevant 
    barrier.oldLayout = oldLayout;
    //Specify the layout to transition to
    barrier.newLayout = newLayout;
    //No queue family transfer is occuring so these indices can be left untouched
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    //Specify the target image
    barrier.image = image;
    //Specify which parts of the image are effected
    barrier.subresourceRange.aspectMask = aspectMask;
    //No mipmapping
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    //No array layers
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    barrier.srcAccessMask = srcAccessMask;
    barrier.dstAccessMask = dstAccessMask;

    vkCmdPipelineBarrier(
        commandBuffer,
        sourceStage, //Specify which pipeline stage the operations occur before the barrier
        destinationStage, //Specify the pipeline stage in which operations will wait on the barrier
        0, //Either 0 or VK_DEPENDENCY_BY_REGION_BIT to allow per-region condition
        0, nullptr, //Memory barrier count and array pointer
        0, nullptr, //Buffer memory barrier count and array pointer
        1, &barrier); //Image memory barrier count and arrya pointer

    endSingleTimeCommands(commandBuffer, commandQueue, commandPool);
}

VkCommandBuffer VulkanRenderer::beginSingleTimeCommands(VkCommandPool& commandPool){
    //Create the command buffer allocation info
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    //Allocate the command buffer in the pool
    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

    //Populate the command buffer with the Begin command
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    //Return the created command buffer
    return commandBuffer;
}

void VulkanRenderer::endSingleTimeCommands(VkCommandBuffer commandBuffer, VkQueue& queue, VkCommandPool& commandPool){
    //Populate the command buffer with an End command
    vkEndCommandBuffer(commandBuffer);

    //Create the command buffer submition info
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    //Submit the command to the queue
    vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
    //Wait for the queue to finish
    vkQueueWaitIdle(queue);

    //Clean up the completed command
    vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

void VulkanRenderer::createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling,
    VkImageUsageFlags usage, VkMemoryPropertyFlags properties, ImageData* imageData){
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    //Type of image; 1D, 2D or 3D
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    //Image dimensions
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    //Mipmapping levels
    imageInfo.mipLevels = 1;
    //Texture layering
    imageInfo.arrayLayers = 1;
    //Image format; should match what is in the staging buffer or will likely fail to copy
    imageInfo.format = format;
    //Texel access type
    imageInfo.tiling = tiling;
    //Specify if the first transition will preserve or discard texels
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    //Specify usage of the image buffer
    imageInfo.usage = usage;
    //Specify queue family sharing
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    //Single sample
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    //Multisampling flags
    imageInfo.flags = 0;

    //Create the image handle
    if(vkCreateImage(device, &imageInfo, nullptr, &imageData->image) != VK_SUCCESS)
        throw std::runtime_error("Failed to create image");

    //Get the memory requirements of the image
    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device, imageData->image, &memRequirements);

    //Generate the allocation info
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

    //Allocate the image memory
    if(vkAllocateMemory(device, &allocInfo, nullptr, &imageData->memory) != VK_SUCCESS)
        throw std::runtime_error("Failed to allocate image memory");

    //Bind the memory to a stored handle
    vkBindImageMemory(device, imageData->image, imageData->memory, 0);
}

void VulkanRenderer::createTextureImage(const Texture* texture, ImageData* output){
    //Determine the size of the image in bytes
    VkDeviceSize imageSize = texture->width * texture->height * 4;

    if(imageSize == 0)
        throw std::runtime_error("Faield to create texture. Size is 0");

    //Create the staging buffer to move the data to device local memory
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(imageSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer,
        stagingBufferMemory);
    
    //Transfer the image data into the staging buffer if any is present
    if(!texture->rawData){
        void* data;
        vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);
        memcpy(data, texture->rawData, static_cast<size_t>(imageSize));
        vkUnmapMemory(device, stagingBufferMemory);
    }

    createImage(texture->width,
    texture->height,
    VK_FORMAT_R8G8B8A8_SRGB,
    VK_IMAGE_TILING_OPTIMAL,
    VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
    output);

    //Transition the destination image to a transfer destination layout
    transitionImageLayout(output->image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    //Copy the staging buffer data into the image
    copyBufferToImage(stagingBuffer, output->image, static_cast<uint32_t>(texture->width), static_cast<uint32_t>(texture->height));

    //Transition the image to a shader read only layout
    transitionImageLayout(output->image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    //Clean up the staging buffer
    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
}

void VulkanRenderer::updateDescriptorSet(std::vector<VkWriteDescriptorSet>& descriptorWrites, VkDescriptorSet& descriptorSet, Object* object){
    //Create descriptor write for transform
    Component* transformComp = object->getComponent(ComponentType::COMP_TRANSFORM);
    if(transformComp){
        TransformData* transformData = (TransformData*)(((Transform*)transformComp)->rendererData);
        VkDescriptorBufferInfo bufferInfo;
        bufferInfo.buffer = transformData->transformMatrixBuffer;
        bufferInfo.offset = 0;
        //16 float entries for mat4;
        bufferInfo.range = 16;
        createDescriptorWrite(descriptorSet, 0, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, &bufferInfo);
    }
    
    //Create descriptor write for material
    Component* matComp = object->getComponent(ComponentType::COMP_MATERIAL);
    if(matComp){
        //Albedo
        ImageData* albedoData = (ImageData*)(((Material*)matComp)->albedo->rendererData);
        VkDescriptorImageInfo imageInfo;
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;
        imageInfo.imageView = albedoData->imageViews[0];
        imageInfo.sampler = textureSampler;
        createDescriptorWrite(descriptorSet, 2, 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, nullptr, &imageInfo);
    }
}

VkWriteDescriptorSet VulkanRenderer::createDescriptorWrite(VkDescriptorSet& descriptorSet, int binding, int arrayElement, VkDescriptorType descriptorType,
    int descriptorCount, VkDescriptorBufferInfo* bufferInfo, VkDescriptorImageInfo* imageInfo, VkBufferView* texelView){
    VkWriteDescriptorSet write;
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstSet = descriptorSet;
    write.dstBinding = binding;
    write.dstArrayElement = arrayElement;
    write.descriptorType = descriptorType;
    write.descriptorCount = descriptorCount;
    write.pBufferInfo = bufferInfo;
    write.pImageInfo = imageInfo;
    write.pTexelBufferView = texelView;

    return write;
}

void VulkanRenderer::createDescriptorSets(VkDescriptorPool descriptorPool, uint32_t numberOfSets, std::vector<VkDescriptorSetLayout> descriptorLayouts, std::vector<VkDescriptorSet>& descriptorSets){
    //There must be a descirptor layout entry for each descriptor set that is going to be created
    if(descriptorLayouts.size() != numberOfSets)
        throw std::runtime_error("Failed to allocate descriptor sets. Layout array size does not match number of sets to create");

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    //Specify the descriptor pool to allocate from
    allocInfo.descriptorPool = descriptorPool;
    //Specify the number of sets to allocate
    allocInfo.descriptorSetCount = numberOfSets;
    //Specify the layouts of each the sets to create
    allocInfo.pSetLayouts = descriptorLayouts.data();

    //Track the original size of the destination vector
    int originalSize = descriptorSets.size();
    //Resize the vector to contain the new results
    descriptorSets.resize(descriptorSets.size() + numberOfSets);

    //Attempt to allocate the descriptor sets, populating the new space in the vector
    VkResult result = vkAllocateDescriptorSets(device, &allocInfo, &descriptorSets[originalSize - 1]);
    if (result != VK_SUCCESS){
        //Reset the vector back to its original state
        descriptorSets.resize(originalSize);
        throw std::runtime_error("Failed to allocate descriptor sets");
    }
}

void VulkanRenderer::createDescriptorPool(VkDescriptorPool& descriptorPool, int maxDescriptorSets, std::vector<VkDescriptorPoolSize> poolSizes){
    //Generate creation info
    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    //Specify the maximum number of sets that can be allocated
    poolInfo.maxSets = static_cast<uint32_t>(maxDescriptorSets);

    if(vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
        throw std::runtime_error("Failed to create descriptor pool");
}

void VulkanRenderer::createUniformBuffers(){
    VkDeviceSize bufferSize = sizeof(UniformBufferObject_Camera);

    uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
    uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

    for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++){
        createBuffer(bufferSize,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            uniformBuffers[i],
            uniformBuffersMemory[i]);

        vkMapMemory(device, uniformBuffersMemory[i], 0, bufferSize, 0, &uniformBuffersMapped[i]);
    }
}

void VulkanRenderer::createDescriptorSetLayout(){
    //Layout binding for the Uniform Buffer Object
    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    //Specify the binding used in the shader
    uboLayoutBinding.binding = 0;
    //Specify the type of the descriptor
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    //Specify the number of values in the case of an array of uniform buffer objects
    uboLayoutBinding.descriptorCount = 1;
    //Specify the stages the binding is going to be referenced in
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    //Relevant for image sampling descriptors
    uboLayoutBinding.pImmutableSamplers = nullptr;

    //Layout binding for the texture sampler
    VkDescriptorSetLayoutBinding samplerLayoutBinding{};
    samplerLayoutBinding.binding = 1;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.pImmutableSamplers = nullptr;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    std::array<VkDescriptorSetLayoutBinding, 2> bindings = {uboLayoutBinding, samplerLayoutBinding};
    //Generate the layout creation info
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = bindings.size();
    layoutInfo.pBindings = bindings.data();

    //Create the descriptor layout
    if(vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
        throw std::runtime_error("Failed to create descriptor set layout");
}

void VulkanRenderer::createSyncObjects(){
    //Resize the lists
    imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

    //Define semaphore creation info; no further parameters
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    //Define fence creation info
    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    //Create the desired number of semaphores and fences
    for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++){
        if(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS
            || vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS
            || vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS)
            throw std::runtime_error("Failed to create semaphores");
    }
}

void VulkanRenderer::recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex){
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    //Defines out the command buffer is to be used
    beginInfo.flags = 0;
    //Used by secondary command buffers to specify which state to inherit from the calling primary command buffer
    beginInfo.pInheritanceInfo = nullptr;

    if(vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
        throw std::runtime_error("Failed to begin recording command buffer");

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    //Specify the render pass and attachments to be used
    renderPassInfo.renderPass = renderPass;
    //Set the framebuffer to be used
    renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
    //Define the area shader loads/stores will occur.
    //Should match size of attachments for best performance. Pixels outside this region will be undefined
    renderPassInfo.renderArea.offset = {0,0};
    renderPassInfo.renderArea.extent = swapChainExtent;
    
    std::array<VkClearValue, 2> clearValues{};
    //Clear color defined as black with 100% opacity
    clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
    //Clear depth as 1.0 to set to far view plane
    clearValues[1].depthStencil = {1.0f, 0};
    //Define the clear values for "VK_ATTACHMENT_LOAD_OP_CLEAR"
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    //Begin the render pass
    //Third parameter controls how drawing commands within the render pass will be provided
    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    //Bind the graphics pipeline
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

    //Define the viewport to be drawn to
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(swapChainExtent.width);
    viewport.height = static_cast<float>(swapChainExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    //Set the viewport
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    //Define the scissor rectangle
    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = swapChainExtent;

    //Set the scissor
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    //Bind the vertex buffer to the shader bindings
    VkBuffer vertexBuffers[] = {vertexBuffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

    //Bind the index buffer to the shader bindings
    vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT16);

    //Bind the descriptor set for the frame
    vkCmdBindDescriptorSets(commandBuffer, 
        VK_PIPELINE_BIND_POINT_GRAPHICS, // _GRAPHICS or _COMPUTE
        pipelineLayout, //Layout the descriptors are based on
        0, //Index of first set
        1, //Number of sets to bind
        &descriptorSets[currentFrame], //Array of sets to bind
        0, //Array of offsets
        nullptr); //Pointer to array of offsets

    //Draw
    vkCmdDrawIndexed(commandBuffer, 
    static_cast<uint32_t>(hardcodedIndices.size()), //Index count
    1, //Instance count for instanced rendering
    0, //Vertex buffer offset; defines lowest value of gl_VertexIndex
    0, //Index buffer offset
    0);//Instance offset for instanced rendering; defines lowest value of gl_InstanceIndex

    //End the render pass
    vkCmdEndRenderPass(commandBuffer);

    if(vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
        throw std::runtime_error("Failed to record command buffer");
}

void VulkanRenderer::createCommandBuffers(VkCommandPool& commandPool, std::vector<VkCommandBuffer>& commandBuffers){
    //Resize the array to the target size
    commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    //Set the command pool that this buffer will be a part of
    allocInfo.commandPool = commandPool;
    //Specifies if command buffer is primary or secondary
    //Primary; can be submit to a queue for execution but cannot be called from other command buffers
    //Secondary; cannot be submit to a queue but can be called from a primary command buffer
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

    if(vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS)
        throw std::runtime_error("Failed to allocate command buffers");
}

void VulkanRenderer::createCommandPool(VkCommandPool& commandPool, QueueFamilyIndices familyIndices){
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    //Associate this command pool with the graphics queue family as this command pool will be used for drawing
    poolInfo.queueFamilyIndex = familyIndices.queueFamily.value();

    if(vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS)
        throw std::runtime_error("Failed to create command pool");
}

void VulkanRenderer::createFrameBuffers(){
    //Resize to hold all the framebuffers
    swapChainFramebuffers.resize(swapChainImageViews.size());

    //Iterate through the image views to create framebuffers from them
    for(size_t i = 0; i < swapChainImageViews.size(); i++){
        //Specify the image view objects that should be bound to the respective attachment descriptions defined in the render pass pAttachement array
        std::array<VkImageView, 2> attachments = {
            swapChainImageViews[i],
            depthImageView
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        //Specify which render pass the framebuffer needs to be compatible with
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = swapChainExtent.width;
        framebufferInfo.height = swapChainExtent.height;
        framebufferInfo.layers = 1;

        //Create the framebuffer
        if(vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS)
            throw std::runtime_error("Failed to create framebuffer");
    }
}

void VulkanRenderer::createRenderPass(){
    //Describe a color buffer attachment
    VkAttachmentDescription colorAttachment{};
    //Match the format of the images in the swap chain
    colorAttachment.format = swapChainImageFormat;
    //No multisampling so only one sample
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    //What to do with the attachment data before rendering
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    //What to do with the attachment data after rendering
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    //What to do with stencil data before rendering
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    //What to do with stencil data after rendering
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    //Which layout the image will have before render pass begins
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    //Which layout to transition to when render pass finishes
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef{};
    //Index of which attachment to reference in array of attachment descriptions
    colorAttachmentRef.attachment = 0;
    //Specify which layout the attachment should have during a subpass that uses this reference
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    //Depth attachment description
    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = findDepthFormat();
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    //Define a subpass for the render pass
    VkSubpassDescription subpass{};
    //Define the subpass as a graphics subpass. Could be defined as compute
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    //Attachments used for color data in a shader
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    //Attachments that are read from a shader
    subpass.inputAttachmentCount = 0;
    subpass.pInputAttachments = nullptr;
    //Attachments used for multisampling color attachments
    subpass.pResolveAttachments = nullptr;
    //Attachement used for depth and stencil data
    subpass.pDepthStencilAttachment = &depthAttachmentRef;
    //Attachments that must be preserved but are not used by the subpass
    subpass.preserveAttachmentCount = 0;
    subpass.pPreserveAttachments = nullptr;

    //Define a dependency for the subpass
    VkSubpassDependency dependency{};
    //Indicate that the subpass at index 0 is dependent on the implicit subpass before
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    //Specify the stages in which the specified operations will occur
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    //Specify the operations to wait on
    dependency.srcAccessMask = 0;
    //Specify the stage that should wait on the previously specified operation
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    //Specify the operation that should wait on the src operation
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    //Attachment data
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    //Subpass data
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    //Dependency data
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if(vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
        throw std::runtime_error("Failed to create render pass");
}

VkShaderModule VulkanRenderer::createShaderModule(const std::vector<char>& code){
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;

    createInfo.codeSize = code.size();
    //Cast to treat the code list as uint32 instead of char, default vector allocator allows for this cast to be done trivially
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shaderModule;
    if(vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
        throw std::runtime_error("Failed to create shader module");

    return shaderModule;
}

void VulkanRenderer::createGraphicsPipeline(){
    //Read the shader code files
    auto vertShaderCode = readFile("shaders/vert.spv");
    auto fragShaderCode = readFile("shaders/frag.spv");

    #ifdef DEBUG_SHADER_FILE_LENGTH_ON_READ
    std::cout << "Vertex shader loaded with length: " << vertShaderCode.capacity() << std::endl;
    std::cout << "Fragment shader loaded with length: " << fragShaderCode.capacity() << std::endl;
    #endif

    //Create shader modules from the loaded shader files
    VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
    VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    //Inform the pipeline that this shader is for the vertex stage
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    //Set the shader module created earlier
    vertShaderStageInfo.module = vertShaderModule;
    //Name of the method to call within the shader
    vertShaderStageInfo.pName = "main";
    //Currently unused but allows shader constants to be assigned in advance for optimization during compile instead of configuring at runtime
    vertShaderStageInfo.pSpecializationInfo = nullptr;

    //Same as the vertex shader creation but for fragment shader
    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";
    fragShaderStageInfo.pSpecializationInfo = nullptr;

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    //Create a vector of the desired dynamics. 
    //Without these a new graphics pipeline would have to be created any time one were to change
    std::vector<VkDynamicState> dynamicStates = {
        //Allows viewport to be resized
        VK_DYNAMIC_STATE_VIEWPORT,
        //Allows for a scissor box to be defined with each draw call
        VK_DYNAMIC_STATE_SCISSOR
    };

    //Populate the creation info structure with the desired dynamics
    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    auto bindingDescription = VkVertex::getBindingDescription();
    auto attributeDescriptions = VkVertex::getAttributeDescriptions();

    //Informs the graphics pipeline the format of the vertex data that is passed to the vertex shader
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    //Spacing between data and if it's per-vertex or per-instance
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    //Type of the attributes, which binding to load them from and at which offset
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    //Informs the graphics pipline of what kind of geometry will be drawn from the vertices
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    //Triangle from every 3 vertices without reuse
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    //If true it's possible to break up lines and triangles when using _STRIP topology using an index of 0xFFFF or 0xFFFFFFFF
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    //Defines the region of the framebuffer that the output will be rendered to
    //Using (0,0) to (width,height) will render to the entire framebuffer
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float) swapChainExtent.width;
    viewport.height = (float) swapChainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    //Defines the region that will actually store pixel data
    //Anything outside of the region will be discarded
    VkRect2D scissor{};
    scissor.offset = {0,0};
    scissor.extent = swapChainExtent;

    //Set up the viewport state. 
    //Because these are to be dynamic the previously defined data will not be used
    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    //Set up the rasterizer. Takes geometry from vertex shader and turns it into fragments
    //Rasterizer also performs depth testing, face culling and the scissor test. Can also be configured to output entire polygons or just edges (wireframe)
    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    //If true, fragments beyond the near and far planes are clamped instead of discarded. 
    //Requires a GPU feature to use
    rasterizer.depthClampEnable = VK_FALSE;
    //If true, geometry never passes through rasterizer stage. Essentially disables output to framebuffer
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    //Determines how fragments are generated. _FILL, _LINE, _POINT options available
    //Requires GPU feature for any mode other than _FILL
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    //Describes thickness of lines in terms of number of fragments. Max line width is hardware dependent
    //Requires GPU feature "wideLines" to use values greater than 1.0f
    rasterizer.lineWidth = 1.0f;
    //Determines cull mode. _FRONT, _BACK, _FRONT_AND_BACK, _NONE options available
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    //Determines vertex order for faces to be considered front-facing
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    //Can be used to alter depth values via constant or slope bias
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f;
    rasterizer.depthBiasClamp = 0.0f;
    rasterizer.depthBiasSlopeFactor = 0.0f;

    //Multisampling controls, disabled currently
    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f;
    multisampling.pSampleMask = nullptr;
    multisampling.alphaToCoverageEnable = VK_FALSE;
    multisampling.alphaToOneEnable = VK_FALSE;

    //Depth and Stencil testing, currently unused
    //VkPipelineDepthStencilStateCreateInfo depthStencil{};

    //Color blending for a given framebuffer; one for each
    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT 
        | VK_COLOR_COMPONENT_G_BIT
        | VK_COLOR_COMPONENT_B_BIT
        | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    //Global color blending creation
    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    //Depth and stencil testing
    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    //Should depth testing be enabled
    depthStencil.depthTestEnable = VK_TRUE;
    //Should anything that passes the depth test be written to the depth buffer
    depthStencil.depthWriteEnable = VK_TRUE;
    //Comparison operation used for depth testing
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    //Used for determening if fragments fall within a specified depth range
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.minDepthBounds = 0.0f;
    depthStencil.maxDepthBounds = 1.0f;
    //Should stenicl buffer tests be used
    depthStencil.stencilTestEnable = VK_FALSE;
    depthStencil.front = {};
    depthStencil.back = {};

    //Used to specify global uniform values in shaders
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    //Set the descriptor set count and address
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges = nullptr;

    if(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
        throw std::runtime_error("Failed to create pipeline layout");

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    //Set the shader stages
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    //Set the states that were generated earlier to describe the fixed function stage
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;        
    //Set the pipeline layout handle
    pipelineInfo.layout = pipelineLayout;
    //Set the render pass to be used
    pipelineInfo.renderPass = renderPass;
    //Set the index of the sub pass of the render pass where this graphics pipeline will be used
    pipelineInfo.subpass = 0;

    //Currently only creating one pipelie but can create multiple in one call
    //Second parameter references optional "VkPipelineCache" object to allow for pipeline creation data to be cached and reused
    if(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS)
        throw std::runtime_error("Failed to create graphics pipeline");

    //Clean up the shader modules
    vkDestroyShaderModule(device, vertShaderModule, nullptr);
    vkDestroyShaderModule(device, fragShaderModule, nullptr);
}

void VulkanRenderer::createImageViews(){
    swapChainImageViews.resize(swapChainImages.size());

    for(size_t idx = 0; idx < swapChainImages.size(); idx++)
        swapChainImageViews[idx] = createImageView(swapChainImages[idx], swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
}

void VulkanRenderer::cleanupSwapChain(){
    //Clean up the depth buffer
    vkDestroyImageView(device, depthImageView, nullptr);
    vkDestroyImage(device, depthImage, nullptr);
    vkFreeMemory(device, depthImageMemory, nullptr);

    //Clean up the frame buffers
    for(auto frameBuffer : swapChainFramebuffers)
        vkDestroyFramebuffer(device, frameBuffer, nullptr);

    //Clean up the image views
    for(auto imageView : swapChainImageViews)
        vkDestroyImageView(device, imageView, nullptr);

    vkDestroySwapchainKHR(device, swapChain, nullptr);
}

void VulkanRenderer::recreateSwapChain(){
    //Pauses the render system while the window is minimized
    int width = 0, height = 0;
    glfwGetFramebufferSize(window, &width, &height);
    while(width == 0 || height == 0){
        glfwGetFramebufferSize(window, &width, &height);
        glfwWaitEvents();
    }

    //Ensure any commands using the device and swap chain are completed before manipulating the swap chain
    vkDeviceWaitIdle(device);

    //Clean up the existing swap chain
    cleanupSwapChain();

    //Regenerate the swap chain
    createSwapChain();
    //Regenerate the views of the swap chain images
    createImageViews();
    //Regenerate depth buffer
    createDepthResources();
    //Regenerate the buffers for the swap chain images
    createFrameBuffers();
}

void VulkanRenderer::createSwapChain(){
    //Fetch the supported swap chain informatino from the physical device
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

    //Select the appropriate settings for the swap chain
    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

    //Recommended to request at least one image above the minimum to avoid having to wait for the driver to complete internal operations
    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

    //Ensure the max image count isn't exceeded. A value of 0 means there is no limit
    if(swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
        imageCount = swapChainSupport.capabilities.maxImageCount;

    #ifdef DEBUG_SWAP_CHAIN_IMAGE_COUNT
    std::cout << "Swap chain min image count: " << swapChainSupport.capabilities.minImageCount << std::endl;
    std::cout << "Swap chain max image count: " << swapChainSupport.capabilities.maxImageCount << std::endl;
    std::cout << "Swap chain assigned image count: " << imageCount << std::endl;
    #endif

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    //Select the surface to bind the swap chain to
    createInfo.surface = surface;
    //Assign the details that we queried
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    //Specify the amount of layers each image consists of. 1 unless developing something like stereoscopic 3D
    createInfo.imageArrayLayers = 1;
    //Currently the images will be rendered to directly. 
    //Post processing would use a bit such as VK_IMAGE_USAGE_TRANSFER_DST_BIT to render to a seperate image first before transfering it to a swap chain image
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    //Get the GRAPHICS queue family's indices
    QueueFamilyIndices indices = queueFamilies[0];
    uint32_t queueFamilyIndices[] = {indices.queueFamily.value(), indices.presentFamily.value()};

    //If the queue families are not the same then define the image sharing as concurrent. Exclusive is more performant but requires a transfer of ownership each time
    if(indices.queueFamily != indices.presentFamily){
        //Set image sharing to concurrent mode
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        //Set the number of queue families to be sharing the swap chain images
        createInfo.queueFamilyIndexCount = 2;
        //Set an array of the indices of the queue families to be sharing the swap chain images
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    //If the queue families are the same then exclusive is fine. Concurrent requires at least two distinct queue families
    else{
        //Set image sharing to exclusive mode
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        //These values are optional but explicitly setting them to a default makes it easy to follow
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
    }

    //Transformations to the image can be applied here. Currently not applying any
    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    //Use for blending with other windows in the window system. Setting to opaque prevents blending
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    //Assign the presentation mode that was queried earlier
    createInfo.presentMode = presentMode;
    //Ignores colors of pixels that are obscured, such as when another window is in front. Enabling this is more performant
    createInfo.clipped = VK_TRUE;
    //Old swap chain assigned to null. Currently resizing the window isn't supported
    //This would hold the reference to the old swap chain when creating the new one for the newly resized window
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if(vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS)
        throw std::runtime_error("Failed to create swap chain");

    //Reuse the image count that determined the minimum number of images in the swap chain and update it with the final count
    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
    //Resize the list of swap chain image handles
    swapChainImages.resize(imageCount);
    //Populate the list of swap chain image handles
    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

    //Store the format of the swap chain images
    swapChainImageFormat = surfaceFormat.format;
    //Store the extents of the swap chain images
    swapChainExtent = extent;
}

void VulkanRenderer::createSurface(){
    //Create and store a reference to the window surface, associating it with the stored window and Vulkan instance
    if(glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS){
        throw std::runtime_error("Failed to craete window surface");
    }
}

void VulkanRenderer::createLogicalDevice(){
    //Create a list of all desired queue family creations
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos{};
    //Using a set allows duplicates to be skipped in the case that a single family supports both desired types; this is a performance optimization
    std::set<uint32_t> uniqueQueueFamilies;

    //Reusable reference container for queue family indices
    QueueFamilyIndices indices;

    //Get the GRAPHICS queue family
    indices = queueFamilies[0];
    //Set the indices
    uniqueQueueFamilies.insert(indices.queueFamily.value());
    uniqueQueueFamilies.insert(indices.presentFamily.value());

    //Get the TRANSFER queue family
    indices = queueFamilies[1];
    uniqueQueueFamilies.insert(indices.queueFamily.value());

    //Set the queue priority to 1.0; float range is 0.0 to 1.0
    float queuePriority = 1.0f;

    //Iterate across the list of queue families and generate the device creation info for each
    for(uint32_t queueFamily : uniqueQueueFamilies){
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    //Generate the logical device creation info
    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    //Set the number of queues being created and the creation info
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();

    //Set the physical device features
    VkPhysicalDeviceFeatures deviceFeatures{};
    //Enable Anisotropy in samplers
    deviceFeatures.samplerAnisotropy = VK_TRUE;

    //Set the features data
    createInfo.pEnabledFeatures = &deviceFeatures;

    //Set extension information
    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    //Set validation layer information
    if(enableValidationLayers){
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    }
    else
        createInfo.enabledLayerCount = 0;

    //Create the logical device and associate it with the physical device
    if(vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS)
        throw std::runtime_error("Failed to create logical device");

    //Fetch and store the reference to the newly created graphics queues
    //We are only creating a single queue in these families so the indices can be hard coded to 0 for the time being
    vkGetDeviceQueue(device, queueFamilies[0].queueFamily.value(), 0, &graphicsQueue);
    vkGetDeviceQueue(device, queueFamilies[0].presentFamily.value(), 0, &presentQueue);
    vkGetDeviceQueue(device, queueFamilies[1].queueFamily.value(), 0, &transferQueue);
}

void VulkanRenderer::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory){
    //Store the array of queue familiy indices used by the vertex buffer
    std::array<uint32_t, 2> queueIndices;
    
    //Populate the array with the GRAPHICS and TRANSFER queue family indices
    queueIndices[0] = queueFamilies[0].queueFamily.value();
    queueIndices[1] = queueFamilies[1].queueFamily.value();

    //Generate the data buffer creation info
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    //Size of the buffer in bytes
    bufferInfo.size = size;
    //Specify flags that represent the purpose of the data buffer
    bufferInfo.usage = usage;
    //Specify if the buffer can be shared between queue families
    //Currently we are looking for two queue families, TRANSFER and GRAPHICS, these may or may not be the same
    bufferInfo.sharingMode = queueIndices[0] != queueIndices[1] ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE;

    bufferInfo.queueFamilyIndexCount = queueIndices.size();
    bufferInfo.pQueueFamilyIndices = std::data(queueIndices);

    //Create the data buffer handle
    if(vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
        throw std::runtime_error("Failed to create vertex buffer");

    //Fetch the memory requirements for the buffer
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

    //Generate the data buffer memory allocation info
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

    //Allocate the memory
    if(vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
        throw std::runtime_error("Failed to allocate vertex buffer memory");

    //Bind the buffer memory handle to the data buffer handle
    vkBindBufferMemory(device, buffer, bufferMemory, 0);
}

void VulkanRenderer::createVertexBuffer(const Mesh* meshData, MeshData* output){
    //Determine the size of the buffer
    VkDeviceSize bufferSize = sizeof(meshData->vertices[0]) * meshData->vertices.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    //Create a buffer for the vertex data to staged in
    createBuffer(bufferSize, 
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
        stagingBuffer,
        stagingBufferMemory);

    void* data;
    //Map the buffer memory into CPU accessible memory
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    //Copy the vertex data into the buffer
    memcpy(data, meshData->vertices.data(), (size_t) bufferSize);
    //Unmap the memory as we no longer need access
    vkUnmapMemory(device, stagingBufferMemory);

    //Create the buffer for the vertex data to exist locally on the device
    createBuffer(bufferSize, 
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        output->vertexBuffer,
        output->vertexMemory);

    //Copy the staging buffer into the vertex buffer
    copyBuffer(stagingBuffer, output->vertexBuffer, bufferSize);

    //Clean up the staging buffer and its memory
    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
}

void VulkanRenderer::createIndexBuffer(const Mesh* meshData, MeshData* output){
    //Determine the size of the buffer
    VkDeviceSize bufferSize = sizeof(meshData->indices[0]) * meshData->indices.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    //Create the staging buffer
    createBuffer(bufferSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer,
        stagingBufferMemory);

    void* data;
    //Map the memory of the staging buffer to the void pointer
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    //Copy the data into the buffer the pointer indicates
    memcpy(data, meshData->indices.data(), (size_t) bufferSize);
    //Unmap the staging buffer
    vkUnmapMemory(device, stagingBufferMemory);

    //Create the destination buffer on the device
    createBuffer(bufferSize, 
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        output->indexBuffer,
        output->indexMemory);
    
    //Copy the memory from source to destination
    copyBuffer(stagingBuffer, output->indexBuffer, bufferSize);

    //Clean up the staging buffer and its memory
    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
}

void VulkanRenderer::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size){
    //Create the command buffer
    VkCommandBuffer commandBuffer = beginSingleTimeCommands(transferCommandPool);

    //Populate the command buffer with a CopyBuffer command
    VkBufferCopy copyRegion{};
    copyRegion.srcOffset = 0;
    copyRegion.dstOffset = 0;
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    //End and submit the command
    endSingleTimeCommands(commandBuffer, transferQueue, transferCommandPool);
}

uint32_t VulkanRenderer::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties){
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    //Iterate through the list of memory types to find one that matches both the filter and the desired properties
    for(uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
        if(typeFilter & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
            return i;

    throw std::runtime_error("Failed to find suitable memory type");
}

VkExtent2D VulkanRenderer::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities){
    //If the current extents are already defined, leave them
    if(capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
        return capabilities.currentExtent;
    //Otherwise fetch the width and height of the frame buffer from GLFW and clamp it within the allowed min/max range
    else{
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        VkExtent2D actualExtent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };

        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width,capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height,capabilities.maxImageExtent.height);

        return actualExtent;
   }
}

VkPresentModeKHR VulkanRenderer::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes){
    //Check if Mailbox mode is available. Triple Buffering style
    for(const auto& availablePresentMode : availablePresentModes)
        if(availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
            return availablePresentMode;

    //This is the only mode guaranteed to be available so is the default return value
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkSurfaceFormatKHR VulkanRenderer::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats){
    //Search for the most desirable format
    //Format: B8G8R8A8_SRGB
    //Color Space: Non-Linear sRGB
    for(const auto& availableFormat : availableFormats)
        if(availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            return availableFormat;
        
    //Default to first available format
    return availableFormats[0];
}

SwapChainSupportDetails VulkanRenderer::querySwapChainSupport(VkPhysicalDevice device){
    SwapChainSupportDetails details;

    //Fetch the surface capabilities based on the device and window surface provided and populate the details struct with the results
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

   //Fetch the number of available formats based on device and window surface
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

    //Resize the structure's format list and populate it with the supported format details
    if(formatCount != 0){
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
    }

    //Fetch the number of avilable presentation modes based on device and window surface
    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

    //Reseze the structures' present mode list and populate it with the supported presentation mode details
    if(presentModeCount != 0){
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
    }

    return details;
}

bool VulkanRenderer::findQueueFamilies(VkPhysicalDevice device, VkQueueFlags queueFamilyFlags, bool presentationSupport, QueueFamilyIndices* pOutput, VkQueueFlags exclusionFlags){
    //Clear any existing data
    pOutput->reset(presentationSupport);

    //Fetch the queue family count
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    //Populate the vector of properties
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    //Search the queue families for at least one that supports the desired queue flags and store its index
    //The index stored will the that of the first family in the list that supports the target flags
    int i = 0;
    VkBool32 presentSupport = false;
    bool familyFound = false;
    for(const auto& queueFamily  : queueFamilies){
        //Graphics queue family check
        if((queueFamily.queueFlags & queueFamilyFlags) && !(queueFamily.queueFlags & exclusionFlags) ){
            pOutput->queueFamily = i;
            familyFound = true;
        }
        if(presentationSupport){
            //Presentation queue family check
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
            if(presentSupport)
                pOutput->presentFamily = i;
            else
                familyFound = false;
        }
        if(familyFound)
            break;
        i++;
    }

    return familyFound;
}

bool VulkanRenderer::checkDeviceExtensionSupport(VkPhysicalDevice device){
    //Fetch the number of extensions the device supports
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    //Populate the list with the extensions supported by the device
    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    //Create a unique set of the desired extensions 
    std::set<std::string> requiredExtensions(deviceExtensions.begin(),deviceExtensions.end());

    //Iterate across the list of available extensions and remove any that match a desired extension from the desired extension set
    for(const auto& extension : availableExtensions)
        requiredExtensions.erase(extension.extensionName);

    //Returns true if all desired extensions were available on the device and thus removed from the set
    return requiredExtensions.empty();
}

bool VulkanRenderer::isDeviceSuitable(VkPhysicalDevice device){
    //Fetch the device's property data
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);

    //Fetch the device's feature data
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

    //Check if the device has the desired queue families
    if(!requestQueueFamilies(device))
        return false;

    //Check if the device supports the desired extensions
    bool extensionsSupported = checkDeviceExtensionSupport(device);

    //Check if the device and surface window have any available formats and presentation modes
    bool swapChainAdequate = false;
    if(extensionsSupported){
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }

    //Only return a valid device if it has the desired queue family support and swap chain support
    return extensionsSupported && swapChainAdequate && deviceFeatures.samplerAnisotropy;

    //EXAMPLE QUERY: Return true if the device is a discreate GPU with support for geometry shaders
    //return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU
    //    && deviceFeatures.geometryShader;
}
    
void VulkanRenderer::pickPhysicalDevice(){
    uint32_t deviceCount = 0;
    //Populates deviceCount with the number of devices available
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    if(deviceCount == 0)
        throw std::runtime_error("Failed to find GPUS with Vulkan support");

    //Create a vector to contain the detected physical devices
    std::vector<VkPhysicalDevice> devices(deviceCount);
    //Populate the vector with the physical device data structures
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    //Iterate through the detected devices and attempt to find the first compatible device
    for(const auto& device : devices){
        if(isDeviceSuitable(device)){
            physicalDevice = device;
            break;
        }
    }

    //If no device is detected; throw
    if (physicalDevice == VK_NULL_HANDLE){
        throw std::runtime_error("Failed to find a suitable GPU");
    }
}

void VulkanRenderer::mainLoop(){
    while(!glfwWindowShouldClose(window)){
        glfwPollEvents();
        drawFrame();
    }

    //Wait for the logical device to finish operations before exiting main loop
    vkDeviceWaitIdle(device);
}

void VulkanRenderer::drawFrame(){
    //Wait for all fences provided, timeout effectively disabled via large value
    vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

    //TODO: This call should be after the generation of the submit info and the queue submissions are called. I'm not sure why the tutorial puts this call beforehand as it makes the semaphore redundant
    //TODO: This call could, and likely should, be moved to a thread and managed that way to prevent the blocking call from locking the main thread. Not an issue with simple triangles but complex models or scenes will cause problems
    //Fetch an image from the swap chain when it is done presentation
    //Blocking call; will wait until image is received or timeout is reached
    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

    //If the results of attempting to acquire the next swap chain image fails due to the swap chain being out of date, regenerate the chain
    if(result == VK_ERROR_OUT_OF_DATE_KHR){
        recreateSwapChain();
        return;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        throw std::runtime_error("Failed to acquire swap chain image");

    //Update the uniform buffer for this frame
    updateUniformBuffer(currentFrame);

    //Reset the fences
    vkResetFences(device, 1, &inFlightFences[currentFrame]);

    //Reset the command buffer
    //Second parameter is a "VkCommandBufferResetFlagBits" flag
    vkResetCommandBuffer(graphicsCommandBuffers[currentFrame], 0);

    //Record the command buffer targetting the image we received from the swap chain
    recordCommandBuffer(graphicsCommandBuffers[currentFrame], imageIndex);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    //Specify which sempahores to wait on before execution begins
    submitInfo.pWaitSemaphores = waitSemaphores;
    //Specify which stages of the pipeline to wait
    submitInfo.pWaitDstStageMask = waitStages;
    //Specify which command bufferst to submit for execution
    submitInfo.commandBufferCount =1;
    submitInfo.pCommandBuffers = &graphicsCommandBuffers[currentFrame];
    //Specify which semaphores to signal once command buffers have finished execution
    VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    //Submit the command buffer to the graphics queue and indicate a fence to be signaled when the buffers finish execution
    if(vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS)
        throw std::runtime_error("Failed to submit draw command buffer");

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    //Specify the semaphores to wait on before presentation can occur
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    //Specify the swap chains to present images to and the index of the image for each swap chain
    VkSwapchainKHR swapChains[] = {swapChain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;
    //Allows specification of an array of VkResult valuese to check for if presentation was successful on every individual swap chain
    presentInfo.pResults = nullptr;

    //Queue the presentation of the frame
    result = vkQueuePresentKHR(presentQueue, &presentInfo);

    if(result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized){
        framebufferResized = false;
        recreateSwapChain();
    }
    else if (result != VK_SUCCESS)
        throw std::runtime_error("Failed to present swap chain image");

    //Advance to the next frame in the loop
    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

bool VulkanRenderer::requestQueueFamilies(VkPhysicalDevice physicalDevice){
    //Clear the list of any existing values
    queueFamilies.clear();

    //Container for finding queue families
    QueueFamilyIndices indices;

    //Find a GRAPHICS queue family that supports presentation
    if(!findQueueFamilies(physicalDevice, VK_QUEUE_GRAPHICS_BIT, true, &indices))
        return false;
    queueFamilies.push_back(indices);

    //Find a TRANSFER queue family that does not include the GRAPHICS flag and does not support presentation
    if(!findQueueFamilies(physicalDevice, VK_QUEUE_TRANSFER_BIT, false, &indices, VK_QUEUE_GRAPHICS_BIT))
        return false;
    queueFamilies.push_back(indices);

    return true;
}

void VulkanRenderer::updateUniformBuffer(uint32_t currentImage){
    //Static tracker for start time; set when this method is initially called
    static auto startTime = std::chrono::high_resolution_clock::now();

    //Get the current time 
    auto currentTime = std::chrono::high_resolution_clock::now();
    //Find the difference between static start time and current time
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    UniformBufferObject_Camera ubo{};
    //Using an identity matrix, rotate 90 degrees per second around the z axis
    ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    //Eye position, center position, up axis
    ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    //FoV, aspect ratio, near clipping distance, far clipping distance
    ubo.proj = glm::perspective(glm::radians(45.0f), swapChainExtent.width / (float) swapChainExtent.height, 0.1f, 10.0f);
    //Flip Y coordinate of clip coords
    ubo.proj[1][1] *= -1;

    memcpy(uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
}

void VulkanRenderer::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo){
    //Create a default structure instance
    createInfo = {};
    //Debug utilities messenger type
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        
    //The message severities that the debug callback should be invoked for
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT 
    | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT 
    | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

    //The message types that the debug callback should be invoked for
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
    | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
    | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        
    //The callback to be invoked
    createInfo.pfnUserCallback = debugCallback;

    //Custom data pointer to be passed along with the callback invocations; eg class instance
    createInfo.pUserData = nullptr;
}

void VulkanRenderer::setupDebugMessenger(){
    //If validation layers are not enabled we don't want to create the debug messenger
    if(!enableValidationLayers) return;

    //Create and populate the debug instance creation info structure
    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    populateDebugMessengerCreateInfo(createInfo);

    if(CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS)
        throw std::runtime_error("Failed to set up debug messenger");
}

VKAPI_ATTR VkBool32 VKAPI_CALL VulkanRenderer::debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData){
        std::cerr << "Validation layer: " << pCallbackData->pMessage << std::endl;

        return VK_FALSE;
}

void VulkanRenderer::framebufferResizeCallback(GLFWwindow* window, int width, int height){
    //Cast the pointer to the VulkanRenderer class
    auto app = reinterpret_cast<VulkanRenderer*>(glfwGetWindowUserPointer(window));
    //Set the framebufferResized flag so the VulkanRenderer instance can recreate the swap chain and buffers
    app->framebufferResized = true;
}