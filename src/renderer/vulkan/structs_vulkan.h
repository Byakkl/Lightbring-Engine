#pragma once
#include <optional>

//Container for supported swap chain features
struct SwapChainSupportDetails{
    //Surface capabilities; eg min/max number of images in swap chain, min/max width/height of images
    VkSurfaceCapabilitiesKHR capabilities;
    //Surface formats; eg pixel format, color space
    std::vector<VkSurfaceFormatKHR> formats;
    //Available presentation modes
    std::vector<VkPresentModeKHR> presentModes;
};

//Container for a set of queue family indices on a device
struct QueueFamilyIndices{
    //Index of a given queue family
    std::optional<uint32_t> queueFamily;
    //Index of a given presentation queue family
    std::optional<uint32_t> presentFamily;
    //Determines if the presentation queue family is required for completion to be considered
    bool requiresPresentFamily = false;
    
    /// @brief Returns true if queueFamily has been assigned an index value
    /// @return 
    bool isComplete(){
        return queueFamily.has_value() && (requiresPresentFamily ? presentFamily.has_value() : true);
    }

    /// @brief Reset method to allow a structure instance to be reused
    /// @param presentationRequired Sets the initial state of the presentation requirement. Defaults to false if no state provided
    void reset(bool presentationRequired = false){
        queueFamily.reset();
        presentFamily.reset();
        requiresPresentFamily = presentationRequired;
    }
};

//Container for a set of Image data
struct ImageData{
    //Stores the handle to an image buffer
    VkImage image;
    //Stores the handle to the image buffer's device memory
    VkDeviceMemory memory;
    //Stores an image view for the texture
    std::vector<VkImageView> imageViews;

    /// @brief Cleans up the view, memory and image buffers
    /// @param device The logical device the image exists on
    void cleanup(VkDevice device){
        for(auto view : imageViews)
            vkDestroyImageView(device, view, nullptr);
        vkDestroyImage(device, image, nullptr);
        vkFreeMemory(device, memory, nullptr);
    }
};

//Container for a set of Mesh data. Currently Vertex and Index data is split into two device memory allocations
struct MeshData{
    //Stores the handle to the device memory allocated for vertices
    VkDeviceMemory vertexMemory;
    //Stores a buffer region for the vertexBufferMemory
    VkBuffer vertexBuffer;

    //Stores the handle to the device memory allocated for indices
    VkDeviceMemory indexMemory;
    //Stores a buffer region for the indexBufferMemory
    VkBuffer indexBuffer;

    /// @brief Cleans up the vertex and index memory allocations and their associated access buffers
    /// @param device 
    void cleanup(VkDevice device){
        vkDestroyBuffer(device, vertexBuffer, nullptr);
        vkFreeMemory(device, vertexMemory, nullptr);

        vkDestroyBuffer(device, indexBuffer, nullptr);
        vkFreeMemory(device, indexMemory, nullptr);
    }
};