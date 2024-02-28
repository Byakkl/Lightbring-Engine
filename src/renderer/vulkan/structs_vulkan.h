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
    //Index of a given graphics queue family
    std::optional<uint32_t> graphicsFamily;
    //Index of a given presentation queue family
    std::optional<uint32_t> presentFamily;
    
    /// @brief Returns true if graphicsFamily has been assigned an index value
    /// @return 
    bool isComplete(){
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};