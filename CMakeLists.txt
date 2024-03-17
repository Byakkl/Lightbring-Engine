cmake_minimum_required(VERSION 3.10)
project(LightbringEngine VERSION 0.1.0 LANGUAGES CXX)

option(USE_VULKAN "Enable Vulkan Renderer" ON)

if(USE_VULKAN)
    message("Vulkan Renderer is enabled")
    add_compile_definitions(RENDERER_VULKAN)
endif()

add_compile_definitions(VK_LOADER_DEBUG=all)
 
project(Lightbring)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

set(BASE_SOURCES
    src/engine.cpp
    src/renderer/renderer.h)

#Add the Core directory
add_subdirectory("src/core" core)
#Add the FileIO directory
add_subdirectory("src/fileio" fileio)
#Add the Vulkan renderer directory
add_subdirectory("src/renderer/vulkan" renderer_vulkan)

#Add an exe for the engine
add_executable(LightbringEngine ${BASE_SOURCES})

#Include the Vulkan SDK
target_include_directories(LightbringEngine PRIVATE 
    "C:/VulkanSDK/1.3.275.0/include"
    )

#Link the library from the Vulkan SDK
target_link_directories(LightbringEngine PRIVATE 
    "C:/VulkanSDK/1.3.275.0/Lib")

#Link the various libraries
target_link_libraries(LightbringEngine PRIVATE
    Core_Lib
    Renderer_Vulkan_Lib
    File_IO_Lib
    "vulkan-1.lib"  
    glfw
)
