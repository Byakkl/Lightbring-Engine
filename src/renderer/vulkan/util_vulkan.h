#pragma once

/// @brief Proxy method to load the Vulkan API extension function "vkCreateDebugUtilsMessengerEXT"
///     The method is not loaded automatically so it must be looked up and invoked manually
/// @param instance 
/// @param pCreateInfo 
/// @param pAllocator 
/// @param pDebugMessenger 
/// @return 
VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger){
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
        if (func != nullptr)
            return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
        else
            return VK_ERROR_EXTENSION_NOT_PRESENT;
    }

/// @brief Proxy method to load the Vulkan API extension function "vkDestrotyDebugUtilsMessengerEXT"
///     The method is not loaded automatically so it must be looked up and invoked manually
/// @param instance 
/// @param debugMessenger 
/// @param pAllocator 
void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
    const VkAllocationCallbacks* pAllocator){
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
        if(func != nullptr)
            func(instance, debugMessenger, pAllocator);
}
