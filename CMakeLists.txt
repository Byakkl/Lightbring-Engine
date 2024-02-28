cmake_minimum_required(VERSION 3.10)
project(LightbringEngine VERSION 0.1.0 LANGUAGES CXX)

add_compile_definitions(VK_LOADER_DEBUG=all)

project(Lightbring)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLEES OFF CACHE BOOL "" FORCE)

add_subdirectory("G:/Coding Libraries/glfw-3.4" glfw)
add_subdirectory("G:/Coding Libraries/glm-1.0.0" glm)

add_executable(LightbringEngine src/engine.cpp)

target_include_directories(LightbringEngine PRIVATE 
    "C:/VulkanSDK/1.3.275.0/include"
    "G:/Coding Libraries/glm-1.0.0")
    
target_link_directories(LightbringEngine PRIVATE 
    "C:/VulkanSDK/1.3.275.0/Lib")

target_link_libraries(LightbringEngine PRIVATE
    "vulkan-1.lib"    
    glfw
    glm)
