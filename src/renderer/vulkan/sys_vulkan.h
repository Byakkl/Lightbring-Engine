#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>
#include "structs_vulkan.h"

class VulkanRenderer{
public:
    /// @brief Called to run the engine
    void run();

private:
    //Constant to define concurrent frame processing
    const int MAX_FRAMES_IN_FLIGHT = 2;
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
    std::vector<VkImage> swapChainImages;
    //Stores the format of the swap chain images
    VkFormat swapChainImageFormat;
    //Stores the extents of the swap chain images
    VkExtent2D swapChainExtent;
    //Stores an image view for each of the images within the swap chain
    std::vector<VkImageView> swapChainImageViews;
    //Stores the render pass used by the graphics pipeline
    VkRenderPass renderPass;
    //Stores the descriptor set layout for shader bindings
    VkDescriptorSetLayout descriptorSetLayout;
    //Stores the descriptor pool
    VkDescriptorPool descriptorPool;
    //Stores the descriptor set handles from the descriptor pool; automatically freed when the descriptor pool is destroyed
    std::vector<VkDescriptorSet> descriptorSets;
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
    //Stores the current frame index; used as an index into semaphores
    uint32_t currentFrame = 0;
    //Stores flag to track if a resize has occurred
    bool framebufferResized = false;
    //Stores a data buffer for vertex data
    VkBuffer vertexBuffer;
    //Stores the handle to the vertex buffer's device memory 
    VkDeviceMemory vertexBufferMemory;
    //Stores a data buffer for index data
    VkBuffer indexBuffer;
    //Stores the handle to the index buffer's device memory
    VkDeviceMemory indexBufferMemory;
    //List of handles to unifrom buffers
    std::vector<VkBuffer> uniformBuffers;
    //List of handles to uniform buffer memory
    std::vector<VkDeviceMemory> uniformBuffersMemory;
    //List of handles to mapped uniform buffers
    std::vector<void*> uniformBuffersMapped;
    //Stores the handle to an image buffer
    VkImage textureImage;
    //Stores the handle to the image buffer's device memory
    VkDeviceMemory textureImageMemory;
    //Stores an image view for the texture
    VkImageView textureImageView;
    //Stores the texture sampler handle
    VkSampler textureSampler;

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
    /// @param commandBuffer 
    /// @param imageIndex 
    void recordCommandBuffer(VkCommandBuffer, uint32_t);
    
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
    void createImageViews();
    
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
    void createVertexBuffer();

    /// @brief Creats an index buffer for use in shaders
    void createIndexBuffer();

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
    
    /// @brief Main engine loop
    void mainLoop();
    
    /// @brief Draws a frame
    void drawFrame();
    
    /// @brief Top level clean up method called when the engine is shut down
    void cleanup();

    /// @brief Clean up method for swap chain and related structures
    void cleanupSwapChain();

    /// @brief Creates the shader binding layouts
    void createDescriptorSetLayout();
    
    /// @brief Creates the uniform buffers used in shaders
    void createUniformBuffers();

    /// @brief Creates a pool of uniform buffer descriptors
    void createDescriptorPool();

    /// @brief Creates descriptor sets from a layout
    void createDescriptorSets();

    /// @brief Creates a texture from an image source
    void createTextureImage();

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
        VkImageUsageFlags, VkMemoryPropertyFlags, VkImage&, VkDeviceMemory&);

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
    void transitionImageLayout(VkImage, VkFormat, VkImageLayout, VkImageLayout);

    /// @brief Copies a buffer's data to an image
    /// @param buffer The buffer to copy data from
    /// @param image The image to populate
    /// @param width The image width
    /// @param height The image height
    void copyBufferToImage(VkBuffer, VkImage, uint32_t, uint32_t);

    /// @brief Creates an image view for the texture image
    void createTextureImageView();

    /// @brief Creates an image view for the provided image
    /// @param image The image to create a view for
    /// @param format The format of the provided image
    /// @return 
    VkImageView createImageView(VkImage, VkFormat);

    /// @brief Creates a texture sampler
    void createTextureSampler();

    /// @brief Updates a uniform buffer
    /// @param currentImage The current frame index, used to identify which buffer needs updating
    void updateUniformBuffer(uint32_t);

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