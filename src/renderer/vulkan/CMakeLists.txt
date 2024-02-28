#Specify the files to be included in this subdirectory library
set(RENDERER_VULKAN_SOURCES
    structs_vulkan.h
    sys_vulkan.h
    sys_vulkan.cpp
    util_vulkan.h)

#Add the dependecies 
add_subdirectory("G:/Coding Libraries/glfw-3.4" glfw)
add_subdirectory("G:/Coding Libraries/glm-1.0.0" glm)

#Create the library
add_library(Renderer_Vulkan_Lib ${RENDERER_VULKAN_SOURCES})

#Include the dependencies to the library
target_include_directories(Renderer_Vulkan_Lib PRIVATE 
    "C:/VulkanSDK/1.3.275.0/include"
    "G:/Coding Libraries/glm-1.0.0")

#Link the dependent directory
target_link_directories(Renderer_Vulkan_Lib PRIVATE 
    "C:/VulkanSDK/1.3.275.0/Lib")

#Link the dependent libraries
target_link_libraries(Renderer_Vulkan_Lib PRIVATE
    "vulkan-1.lib"    
    glfw
    glm)