cmake_minimum_required(VERSION 3.10)
project(LightbringEngine VERSION 0.1.0 LANGUAGES CXX)

#Set properties for external library paths
set(GLFW_SOURCE_PATH "G:/Coding Libraries/glfw-3.4")
set(GLM_SOURCE_PATH "G:/Coding Libraries/glm-1.0.0")
set(STB_SOURCE_PATH "G:/Coding Libraries/stb")
set(TINYOBJ_SOURCE_PATH "G:/Coding Libraries/tinyOBJLoader")

#Set output directories for building
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/build/output)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/build/output)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/build/output)
set(HEADER_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output/include)

add_compile_definitions(VK_LOADER_DEBUG=all)

#Set option to use Vulkan
option(USE_VULKAN "Enable Vulkan Renderer" ON)

#Get a list of the public header files
file(GLOB ENGINE_HEADERS include/*.h)

#Add a library for the engine
add_library(LightbringEngine SHARED
    ${CMAKE_CURRENT_SOURCE_DIR}/src/engine.cpp
    ${ENGINE_HEADERS}
)

#Set output library name to LightbringEngine and remove the lib prefix
set_target_properties(LightbringEngine PROPERTIES
    OUTPUT_NAME "LightbringEngine"
    PREFIX ""
    IMPORT_PREFIX ""
    IMPORT_SUFFIX ".lib"
    ARCHIVE_OUTPUT_NAME "LightbringEngine"
)

target_include_directories(LightbringEngine PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if(USE_VULKAN)
    message("Vulkan Renderer is enabled")
    add_compile_definitions(RENDERER_VULKAN)
endif()


#Add the Core directory
add_subdirectory("src/core" core)
#Add the FileIO directory
add_subdirectory("src/fileio" fileio)
#Add the Vulkan renderer directory
add_subdirectory("src/renderer/vulkan" renderer_vulkan)
#Add the GLM Library directory
add_subdirectory(${GLM_SOURCE_PATH} glm)
#Add the GLFW Library directory
add_subdirectory(${GLFW_SOURCE_PATH} glfw)

target_include_directories(LightbringEngine PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core
    ${CMAKE_CURRENT_SOURCE_DIR}/src/fileio
    ${CMAKE_CURRENT_SOURCE_DIR}/src/renderer/Vulkan
    
    ${STB_SOURCE_PATH}
    ${TINYOBJ_SOURCE_PATH}
)

find_package(Vulkan REQUIRED)
target_link_libraries(LightbringEngine PUBLIC 
    Vulkan::Vulkan
)

target_link_libraries(LightbringEngine PRIVATE 
    glfw
    glm
)

add_custom_command(
    TARGET LightbringEngine POST_BUILD
    COMMAND 
        ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
        ${HEADER_OUTPUT_DIRECTORY}
    COMMENT "Copying public headers to output directory"
)