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