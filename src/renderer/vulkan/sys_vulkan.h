#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>
#include "../../core/renderer.h"
#include "../../core/structs.h"
#include "structs_vulkan.h"
#include "../../core/camera.h"

class VulkanRenderer : public Renderer{
public:
    /// @brief Implementation of Renderer pure virtual method
    void initialize() override;

    bool render(Camera*, std::vector<Object*>) override;

    void cleanup() override;

    void createTexture(Texture*) override;

    void unloadTexture(Texture*) override;

    void uploadMesh(Mesh*) override;

    void unloadMesh(Mesh*) override;

    void registerCamera(Camera*) override;

    void unregisterCamera(Camera*) override;

private:
    //Constant to define concurrent frame processing
    const int MAX_FRAMES_IN_FLIGHT = 2;
    //Constant to define the maximum number of sets in the object pool
    const int MAX_OBJECT_DESCRIPTOR_SETS = 10;
    //Constant to define the maximum number of sets in the camera pool
    const int MAX_CAMERA_DESCRIPTOR_SETS = 5;
    //Constants for window width and height
    const uint32_t WIDTH = 800;
    const uint32_t HEIGHT = 600;
    //Control if validation layers will be used through the define of NDEBUG
    #ifndef NDEBUG
        const bool enableValidationLayers = false;
    #else
        const bool enableValidationLayers = true;
    #endif

    //Identify which validation layers to be used
    const std::vector<const char*> validationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };

    //Identify which device extensions to be used
    const std::vector<const char*> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };


    //Stores a reference to a created window
    GLFWwindow* window;
    //Stores the Vulkan instance
    VkInstance instance;
    //Stores the Vulkan debug messenger instance
    VkDebugUtilsMessengerEXT debugMessenger;
    //Stores the physical device to be targeted by Vulkan; this is implicitly destroyed alongside the instance
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    //Stores a list of all of the queue family sets in use
    std::vector<QueueFamilyIndices> queueFamilies;
    //Stores the logical device to be used by Vulkan
    VkDevice device;
    //Stores a handle to the Vulkan graphics queue; this is implicitly cleaned up algonside the device it's associated with
    VkQueue graphicsQueue;
    //Stores a reference to the target window surface that will be rendered to
    VkSurfaceKHR surface;
    //Stores a handle to the Vulkan presentation queue; this is implicitly cleaned up alongside the device it's associated with
    VkQueue presentQueue;
    //Stores a handle the transfer queue; this is implicitly cleaned up alongside the device it's associated with
    VkQueue transferQueue;
    //Stores the Vulkan swap chain object
    VkSwapchainKHR swapChain;

    //Stores a list of handles for each of the swap chain images
    //std::vector<VkImage> swapChainImages;

    //Stores the format of the swap chain images
    VkFormat swapChainImageFormat;
    //Stores the extents of the swap chain images
    VkExtent2D swapChainExtent;

    std::vector<ImageData> swapChainImageData;
    //Stores an image view for each of the images within the swap chain
    //std::vector<VkImageView> swapChainImageViews;

    //Stores the render pass used by the graphics pipeline
    VkRenderPass renderPass;

    //Stores the descriptor pool for object data
    VkDescriptorPool objectDescriptorPool;
    //Stores the descriptor set layout for shader bindings related to object data; eg mesh and materials
    VkDescriptorSetLayout objectDescriptorSetLayout;
    //Stores the descriptor set handles from the object descriptor pool; automatically freed when objectDescriptorPool is destroyed
    std::vector<VkDescriptorSet> objectDescriptorSets;
    
    //Stores the graphics pipeline layout object
    VkPipelineLayout pipelineLayout;
    //Stores the graphics pipeline object
    VkPipeline graphicsPipeline;
    //Stores the swap chain frame buffers
    std::vector<VkFramebuffer> swapChainFramebuffers;
    //Stores the command pool that contains the command buffers for the family supporting the GRAPHICS type
    VkCommandPool graphicsCommandPool;
    //Stores the command pool that contains the command buffers for the family supporting the TRANSFER type
    VkCommandPool transferCommandPool;
    //Stores the command buffers for the GRAPHICS command pool; automatically freed when its command pool is destroyed
    std::vector<VkCommandBuffer> graphicsCommandBuffers;
    //Stores the command buffers for the TRANSFER command pool; automatically freed when its command pool is destroyed
    std::vector<VkCommandBuffer> transferCommandBuffers;
    //Stores the semaphore objects to when when an image in the swap chain is available for rendering
    std::vector<VkSemaphore> imageAvailableSemaphores;
    //Stores the semaphore objects to signal when rendering is complete and presentation can happen
    std::vector<VkSemaphore> renderFinishedSemaphores;
    //Stores the fence objects to signal when presentation is complete
    std::vector<VkFence> inFlightFences;
    //Stores the fence object used to signal when an object render batch is complete
    VkFence renderFence;
    //Stores the current frame index; used as an index into semaphores
    uint32_t currentFrame = 0;
    //Stores flag to track if a resize has occurred
    bool framebufferResized = false;

    //Stores the texture sampler handle
    VkSampler textureSampler;

    //Stores depth image handles
    ImageData depthImage;

    PushConstants pushConstants;

    
    std::vector<VkImage> images;

    /// @brief Initializes the window used to render to
    void initWindow();

    /// @brief Creates the Vulkan instance
    void createInstance();

    /// @brief Initializes Vulkan
    void initVulkan();
    
    /// @brief Gets the extensions required by the GLFW instance
    /// @return 
    std::vector<const char*> getRequiredExtensions();

    /// @brief Checks if all desired validation layers in "validationLayers" are supported
    /// @return 
    bool checkValidationLayerSupport();
    
    void createSyncObjects();
    
    /// @brief Records the command buffer that will render a frame to a swap chain image
    /// @param commandBuffer Command buffer to write commands into
    /// @param imageIndex Index of the framebuffer that will be rendered to
    /// @param viewProjMatrix Precomputed camera matrices to be used with object transformation matrix to generate MVP push constant
    /// @param objects Pointer to entry in object array used to fetch model data
    /// @param objectCount Number of objects to be drawn within this render command buffer. Used to increment the pointer to walk along the array
    void recordObjectRenderCommandBuffer(VkCommandBuffer, uint32_t, glm::mat4, Object*, int);
    
    /// @brief Creates the command buffers
    /// @param commandPool Reference to the command pool the buffer will be created on
    /// @param commandBuffers Reference to the list of command buffers to populate
    void createCommandBuffers(VkCommandPool&, std::vector<VkCommandBuffer>&);
    
    /// @brief Creates the command pool that the command buffers will be created out of
    /// @param commandPool Reference to the command pool variable to be populated
    /// @param familyIndices Container for the indices of the queue family
    void createCommandPool(VkCommandPool&, QueueFamilyIndices);
    
    /// @brief Create the framebuffers for the swap chain images
    void createFrameBuffers();
    
    /// @brief Creates the render pass to be used in the graphics pipeline
    void createRenderPass();
    
    /// @brief Creates a Vulkan shader module from the passed in vector of shader binary code
    /// @param code 
    /// @return 
    VkShaderModule createShaderModule(const std::vector<char>&);
    
    /// @brief Creates the graphics/shader pipeline that Vulkan will use to render to render targets
    void createGraphicsPipeline();
    
    /// @brief Creates an image view for each of the images within the swap chain
    void createSwapChainImageViews();
    
    /// @brief Regenerates the swap chain
    void recreateSwapChain();

    /// @brief Creates the Vulkan swap chain 
    void createSwapChain();
    
    /// @brief Creates the window surface that will be the render target
    void createSurface();
    
    /// @brief Creates the logical device and queues to be used by Vulkan
    void createLogicalDevice();

    /// @brief Creates a memory buffer
    /// @param size The size of the memory buffer in bytes
    /// @param usage Informs Vulkan of the purpose of the buffer
    /// @param properties Specifies the properties of the type of memory that should be assigned to the buffer
    /// @param buffer Reference to output the buffer handle to
    /// @param bufferMemory Reference to output the buffer memory handle to
    void createBuffer(VkDeviceSize, VkBufferUsageFlags, VkMemoryPropertyFlags, VkBuffer&, VkDeviceMemory&);

    /// @brief Creates a vertex buffer for use in shaders
    void createVertexBuffer(const Mesh*, MeshData*);

    /// @brief Creats an index buffer for use in shaders
    void createIndexBuffer(const Mesh*, MeshData*);

    /// @brief Copies data from one buffer to another
    /// @param srcBuffer Source data buffer
    /// @param dstBuffer Destination data buffer
    /// @param size Size of the data to be transfered in bytes
    void copyBuffer(VkBuffer, VkBuffer, VkDeviceSize);

    /// @brief Requests all desired queue families from the physical device
    /// @param physicalDevice The physical device to request the queue families from
    /// @return Returns true if all requested queue families are found
    bool requestQueueFamilies(VkPhysicalDevice);

    /// @brief Finds the correct type of memory on the GPU for the given parameters
    /// @param  typeFilter specifies the bit field of suitable memory types
    /// @param  properties 
    /// @return 
    uint32_t findMemoryType(uint32_t, VkMemoryPropertyFlags);
    
    /// @brief 
    /// @param capabilities 
    /// @return 
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR&);
    
    /// @brief Checks for the most desirable presentation mode from the provided list. Defaults to FIFO mode if specific mode is not found
    /// @param availablePresentModes 
    /// @return 
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>&);
    
    /// @brief Checks for the most desirable swap surface format from the provided list. Defaults to the first entry if specific type is not found
    /// @param availableFormats 
    /// @return 
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>&);
    
    /// @brief 
    /// @param  
    /// @return 
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice);
    
    /// @brief Attempts to find a queue family matching desired hard coded search criteria
    /// @param device The physical device to be searched on
    /// @param queueFamilyFlags The flags that are required of a family for a successful search
    /// @param exclusionFlags The flags that are required to not be present in the queue
    /// @param presentationSupport Should the queue family support presentation
    /// @param pOutput Pointer to the structure to contain the family indices
    /// @return Returns true if a family matching the requirements was found
    bool findQueueFamilies(VkPhysicalDevice, VkQueueFlags, bool, QueueFamilyIndices*, VkQueueFlags = 0);
    
    /// @brief 
    /// @param  
    /// @return 
    bool checkDeviceExtensionSupport(VkPhysicalDevice);
    
    /// @brief Checks if a given physical device is suitable to be used for the Vulkan instance
    /// @param device 
    /// @return 
    bool isDeviceSuitable(VkPhysicalDevice);
    
    /// @brief Determines which physical device should be used by Vulkan
    void pickPhysicalDevice();

    /// @brief Clean up method for swap chain and related structures
    void cleanupSwapChain();

    /// @brief Creates the shader binding layouts
    void createObjectDescriptorSetLayout();
    
    /// @brief Creates the uniform buffers used in shaders
    template <typename T>
    BufferSet createUniformBuffer();

    /// @brief Creates a pool of uniform buffer descriptors
    /// @param descriptorPool The descriptor pool variable to populate
    /// @param maxDescriptorSets The maximum number of descriptor sets that can be created at one time in this pool
    /// @param poolSizes Array of allowed descriptory types and their max quanitites to be allocated from the pool at a given time
    void createDescriptorPool(VkDescriptorPool&, int, std::vector<VkDescriptorPoolSize>);

    /// @brief Creates descriptor sets from a layout. Fails if numberOfSets does not match the size of descriptorLayouts
    /// @param descriptorPool The pool that the sets will be allocated from
    /// @param numberOfSets The number of sets to be allocated
    /// @param descriptorLayouts The layouts of each of the sets to be allocated
    /// @param descriptorSets The vector to resize and populate the new set handles into
    void createDescriptorSets(VkDescriptorPool, uint32_t, std::vector<VkDescriptorSetLayout>, std::vector<VkDescriptorSet>&);

    void updateDescriptorSet(std::vector<VkWriteDescriptorSet>&, VkDescriptorSet&, Object*);

    VkWriteDescriptorSet createDescriptorWrite(VkDescriptorSet&, int, int, VkDescriptorType, int, VkDescriptorBufferInfo* = nullptr, VkDescriptorImageInfo* = nullptr, VkBufferView* = nullptr);

    /// @brief Creates a texture from an image source
    void createTextureImage(const Texture*, ImageData*);

    /// @brief Creates a Vulkan image
    /// @param width Width in pixels
    /// @param height Height in pixels
    /// @param format Format of the image data
    /// @param tiling Texel layout format
    /// @param usage Purpose of the image buffer
    /// @param properties Properties of the image memory
    /// @param image Reference to the image handle to be populated
    /// @param imageMemory Reference to the memory handle to be populated
    void createImage(uint32_t, uint32_t, VkFormat, VkImageTiling, 
        VkImageUsageFlags, VkMemoryPropertyFlags, ImageData*);

    /// @brief Creates a command buffer and executes a Begin command
    /// @param commandPool The command pool to create the buffer in
    /// @return Returns the created command buffer
    VkCommandBuffer beginSingleTimeCommands(VkCommandPool&);

    /// @brief Ends a command buffer and submits it to the provided queue. Waits until the queue is complete before freeing the buffer
    /// @param commandBuffer The command buffer to End and submit to the queue
    /// @param queue The queue to submit the command buffer to
    /// @param commandPool The command pool the buffer belongs to. Used to free the command buffer after completion
    void endSingleTimeCommands(VkCommandBuffer, VkQueue&, VkCommandPool&);

    /// @brief Converts an image to another layout
    /// @param image The image to be converted
    /// @param format The format of the image
    /// @param oldLayout The current layout of the image
    /// @param newLayut The layout to conver the image to
    void transitionImageLayout(VkImage&, VkFormat, VkImageLayout, VkImageLayout);

    /// @brief Copies a buffer's data to an image
    /// @param buffer The buffer to copy data from
    /// @param image The image to populate
    /// @param width The image width
    /// @param height The image height
    void copyBufferToImage(VkBuffer, VkImage, uint32_t, uint32_t);

    /// @brief Creates an image view for the provided image
    /// @param image The image to create a view for
    /// @param format The format of the provided image
    /// @param aspectFlags The aspect flag for the image, eg color or depth
    void createImageView(ImageData*, VkFormat, VkImageAspectFlags);

    /// @brief Creates a texture sampler
    void createTextureSampler();

    /// @brief Updates a buffer
    /// @param bufferSet Reference to the buffer and device memory to be updated
    /// @param offset The offset into the memory that the data is to be copied at
    /// @param dataObject The data to be copied into the memory
    template<typename T>
    void updateBuffer(BufferSet&, uint32_t, T);

    /// @brief Creates texture resource for a depth buffer
    void createDepthResources();

    /// @brief Finds a format that supports the desired features
    /// @param candidates The list of formats to be looked through
    /// @param tiling Image tiling
    /// @param features The features to be supported
    /// @return 
    VkFormat findSupportedFormat(const std::vector<VkFormat>&, VkImageTiling, VkFormatFeatureFlags);

    /// @brief Finds the depth format
    /// @return 
    VkFormat findDepthFormat();

    /// @brief Checks if a given format has a stencil component
    /// @param format The format to check
    /// @return True if the format has a stencil component 
    bool hasStencilComponent(VkFormat);

    /// @brief Populates a debug messenger instance creation info structure
    /// @param createInfo 
    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT&);
    
    /// @brief Creates the Vulkan debug messenger instance
    void setupDebugMessenger();
    
    /// @brief Callback invoked by the Vulkan API to provide information
    /// @param messageSeverity 
    /// @param messageType 
    /// @param pCallbackData 
    /// @param pUserData 
    /// @return 
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT,
        VkDebugUtilsMessageTypeFlagsEXT,
        const VkDebugUtilsMessengerCallbackDataEXT*,
        void*);

    /// @brief Callback used by GLFW when a resize occurs
    /// @param  window
    /// @param  width
    /// @param  height
    static void framebufferResizeCallback(GLFWwindow*, int, int);
};