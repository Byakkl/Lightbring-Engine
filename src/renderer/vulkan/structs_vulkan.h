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

struct BufferSet{
    //Stores the buffer handle
    VkBuffer buffer;
    //Stores the device memory handle
    VkDeviceMemory memory;

    void cleanup(VkDevice device){
        vkDeviceWaitIdle(device);

        vkDestroyBuffer(device, buffer, nullptr);
        vkFreeMemory(device, memory, nullptr);
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
        vkDeviceWaitIdle(device);
        for(auto view : imageViews)
            vkDestroyImageView(device, view, nullptr);
        vkDestroyImage(device, image, nullptr);
        vkFreeMemory(device, memory, nullptr);
    }
};

//Container for a set of Mesh data. Currently Vertex and Index data is split into two device memory allocations
struct MeshData{
    //Buffer set for mesh vertices
    BufferSet vertexBufferSet;

    //Buffer set for mesh indices
    BufferSet indexBufferSet;

    /// @brief Cleans up the vertex and index memory allocations and their associated access buffers
    /// @param device 
    void cleanup(VkDevice device){
        vertexBufferSet.cleanup(device);
        indexBufferSet.cleanup(device);
    }
};

struct TransformData{
    //Buffer set for transform matrix
    BufferSet transformBufferSet;

    void cleanup(VkDevice device){
        transformBufferSet.cleanup(device);
    }
};

struct CameraData{
    //Buffer set for camera matrices
    BufferSet cameraBufferSet;

    void cleanup(VkDevice device){
        cameraBufferSet.cleanup(device);
    }
};

struct PushConstants{
    alignas(16) glm::mat4 mvp;
};