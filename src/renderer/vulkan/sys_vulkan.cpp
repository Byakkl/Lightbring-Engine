#include <stdexcept>
#include <iostream>
#include <cstdlib>
#include <vector>
#include <cstring>
#include <optional>
#include <set>
#include <limits>
#include <algorithm>

#include "debug_vulkan.h"
#include "sys_vulkan.h"
#include "util_vulkan.h"
#include "../../fileio/util_io.h"


void VulkanRenderer::run(){
        initWindow();
        initVulkan();
        mainLoop();
        cleanup();
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
        //Disable resizing of created window
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        //Create a new window with resolution 800x600 and name Vulkan
        //4th parameter can specify a monitor to open on
        //5th parameter is relevant to OpenGL
        window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
    }

void VulkanRenderer::createInstance(){
    //If validation layers are enabled but any are unsupported then throw an error
    if(enableValidationLayers && !checkValidationLayerSupport()){
        throw std::runtime_error("Validation layers requested, but not available");
    }

    //Create an info structure about the application; technically optional
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Hello Triangle";
    appInfo.applicationVersion = VK_MAKE_VERSION(1,0,0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1,0,0);
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
    createGraphicsPipeline();
    createFrameBuffers();
    createCommandPool();
    createCommandBuffer();
    createSyncObjects();
}

void VulkanRenderer::createSyncObjects(){
    //Define semaphore creation info; no further parameters
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    //Define fence creation info
    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    if(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphore) != VK_SUCCESS
        || vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphore) != VK_SUCCESS
        || vkCreateFence(device, &fenceInfo, nullptr, &inFlightFence) != VK_SUCCESS)
        throw std::runtime_error("Failed to create semaphores");
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
    //Clear color defined as black with 100% opacity
    VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    //Define the clear values for "VK_ATTACHMENT_LOAD_OP_CLEAR"
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

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

    //Draw
    //Second parameter is the vertex count
    //Third parameter is instance count used for instanced rendering
    //Fourth parameter is vertex buffer offset; defines lowest value of gl_VertexIndex
    //Fight parameter is instance offset for instanced rendering; defines lowest value of gl_InstanceIndex
    vkCmdDraw(commandBuffer, 3, 1 ,0 ,0);

    //End the render pass
    vkCmdEndRenderPass(commandBuffer);

    if(vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
        throw std::runtime_error("Failed to record command buffer");
}

void VulkanRenderer::createCommandBuffer(){
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    //Set the command pool that this buffer will be a part of
    allocInfo.commandPool = commandPool;
    //Specifies if command buffer is primary or secondary
    //Primary; can be submit to a queue for execution but cannot be called from other command buffers
    //Secondary; cannot be submit to a queue but can be called from a primary command buffer
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    if(vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer) != VK_SUCCESS)
        throw std::runtime_error("Failed to allocate command buffers");
}

void VulkanRenderer::createCommandPool(){
    QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    //Associate this command pool with the graphics queue family as this command pool will be used for drawing
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    if(vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS)
        throw std::runtime_error("Failed to create command pool");
}

void VulkanRenderer::createFrameBuffers(){
    //Resize to hold all the framebuffers
    swapChainFramebuffers.resize(swapChainImageViews.size());

    //Iterate through the image views to create framebuffers from them
    for(size_t i = 0; i < swapChainImageViews.size(); i++){
        //Specify the image view objects that should be bound to the respective attachment descriptions defined in the render pass pAttachement array
        VkImageView attachments[] = {
            swapChainImageViews[i]
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        //Specify which render pass the framebuffer needs to be compatible with
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
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
    subpass.pDepthStencilAttachment = nullptr;
    //Attachments that must be preserved but are not used by the subpass
    subpass.preserveAttachmentCount = 0;
    subpass.pPreserveAttachments = nullptr;

    //Define a dependency for the subpass
    VkSubpassDependency dependency{};
    //Indicate that the subpass at index 0 is dependent on the implicit subpass before
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    //Specify the stages in which the specified operations will occur
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    //Specify the operations to wait on
    dependency.srcAccessMask = 0;
    //Specify the stage that should wait on the previously specified operation
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    //Specify the operation that should wait on the src operation
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    //Attachment data
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
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

    //Informs the graphics pipeline the format of the vertex data that is passed to the vertex shader
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    //Spacing between data and if it's per-vertex or per-instance
    vertexInputInfo.vertexBindingDescriptionCount = 0;
    vertexInputInfo.pVertexBindingDescriptions = nullptr;
    //Type of the attributes, which binding to load them from and at which offset
    vertexInputInfo.vertexAttributeDescriptionCount = 0;
    vertexInputInfo.pVertexAttributeDescriptions = nullptr;

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
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
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

    //Used to specify global uniform values in shaders
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pSetLayouts = nullptr;
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
    pipelineInfo.pDepthStencilState = nullptr;
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

    for(size_t idx = 0; idx < swapChainImages.size(); idx++){
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = swapChainImages[idx];
        //Determines what it treats the image as, in this case 2D. Can be used to treat it as 1D, 2D, 3D and cube maps
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = swapChainImageFormat;
        //Allows for swizzling of color channels
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        //Subresource Range field informs of what the image's purpose is.
        //Used as a color target
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        //No mip mapping
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        //No layers
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        //Create the image view and store it in the list
        if(vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews[idx]) != VK_SUCCESS)
            throw std::runtime_error("Failed to create image views");
    }
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

    //Fetch the queue family indices
    QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    //If the queue families are not the same then define the image sharing as concurrent. Exclusive is more performant but requires a transfer of ownership each time
    if(indices.graphicsFamily != indices.presentFamily){
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
    //Find a queue family on the physical device
    QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

    //Create a list of all desired queue family creations
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos{};
    //Using a set allows duplicates to be skipped in the case that a single family supports both desired types; this is a performance optimization
    std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};
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
    vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
    vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
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

QueueFamilyIndices VulkanRenderer::findQueueFamilies(VkPhysicalDevice device){
    QueueFamilyIndices indices;

    //Fetch the queue family count
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    //Populate the vector of properties
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    //Search the queue families for at least one that supports the VK_QUEUE_GRAPHHICS_BIT and store its index
    //The index stored will the that of the last family in the list that supports the target flag
    int i = 0;
    VkBool32 presentSupport = false;
    for(const auto& queueFamily  : queueFamilies){
        //Graphics queue family check
        if(queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT){
            indices.graphicsFamily = i;
        }
        //Presentation queue family check
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
        if(presentSupport)
            indices.presentFamily = i;

        i++;
    }

    return indices;
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
    QueueFamilyIndices indices = findQueueFamilies(device);

    //Check if the device supports the desired extensions
    bool extensionsSupported = checkDeviceExtensionSupport(device);

    //Check if the device and surface window have any available formats and presentation modes
    bool swapChainAdequate = false;
    if(extensionsSupported){
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }

    //Only return a valid device if it has the desired queue familiy support and swap chain support
    return indices.isComplete() && extensionsSupported && swapChainAdequate;

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
    vkWaitForFences(device, 1, &inFlightFence, VK_TRUE, UINT64_MAX);

    //Reset the fences
    vkResetFences(device, 1, &inFlightFence);

    //Fetch an image from the swap chain when it is done presentation
    uint32_t imageIndex;
    vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

    //Reset the command buffer
    //Second parameter is a "VkCommandBufferResetFlagBits" flag
    vkResetCommandBuffer(commandBuffer, 0);

    //Record the command buffer targetting the image we received from the swap chain
    recordCommandBuffer(commandBuffer, imageIndex);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {imageAvailableSemaphore};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    //Specify which sempahores to wait on before execution begins
    submitInfo.pWaitSemaphores = waitSemaphores;
    //Specify which stages of the pipeline to wait
    submitInfo.pWaitDstStageMask = waitStages;
    //Specify which command bufferst to submit for execution
    submitInfo.commandBufferCount =1;
    submitInfo.pCommandBuffers = &commandBuffer;
    //Specify which semaphores to signal once command buffers have finished execution
    VkSemaphore signalSemaphores[] = {renderFinishedSemaphore};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    //Submit the command buffer to the graphics queue and indicate a fence to be signaled when the buffers finish execution
    if(vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFence) != VK_SUCCESS)
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

    vkQueuePresentKHR(presentQueue, &presentInfo);
}

void VulkanRenderer::cleanup(){
    //Clean up sync objects
    vkDestroySemaphore(device, imageAvailableSemaphore, nullptr);
    vkDestroySemaphore(device, renderFinishedSemaphore, nullptr);
    vkDestroyFence(device, inFlightFence, nullptr);

    //Clean up the command pool
    vkDestroyCommandPool(device, commandPool, nullptr);

    //Clean up the framebuffers
    for(auto framebuffer : swapChainFramebuffers)
        vkDestroyFramebuffer(device, framebuffer, nullptr);

    //Clean up the graphics pipeline
    vkDestroyPipeline(device, graphicsPipeline, nullptr);

    //Clean up the graphics pipeline layout
    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
        
    //Clean up the render pass
    vkDestroyRenderPass(device, renderPass, nullptr);

    //Clean up the image views
    for(auto imageView : swapChainImageViews)
        vkDestroyImageView(device, imageView, nullptr);

    //Clean up the swap chain
    vkDestroySwapchainKHR(device, swapChain, nullptr);

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

