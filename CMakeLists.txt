cmake_minimum_required(VERSION 3.10)
project(LightbringEngine VERSION 0.1.0 LANGUAGES CXX)

add_compile_definitions(VK_LOADER_DEBUG=all)
 
project(Lightbring)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

#Add the FileIO directory
add_subdirectory("src/fileio" fileio)
#Add the Vulkan renderer directory
add_subdirectory("src/renderer/vulkan" renderer_vulkan)

#Add an exe for the engine
add_executable(LightbringEngine src/engine.cpp)

#Include the Vulkan SDK
target_include_directories(LightbringEngine PRIVATE 
    "C:/VulkanSDK/1.3.275.0/include"
    )

#Link the library from the Vulkan SDK
target_link_directories(LightbringEngine PRIVATE 
    "C:/VulkanSDK/1.3.275.0/Lib")

#Link the various libraries
target_link_libraries(LightbringEngine PRIVATE
    Renderer_Vulkan_Lib
    File_IO_Lib
    "vulkan-1.lib"  
    glfw
)
