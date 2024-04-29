#Specify the files to be included in this subdirectory library
set(RENDERER_VULKAN_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/structs_vulkan.h
    ${CMAKE_CURRENT_SOURCE_DIR}/sys_vulkan.h
    ${CMAKE_CURRENT_SOURCE_DIR}/sys_vulkan.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/util_vulkan.h
    ${CMAKE_CURRENT_SOURCE_DIR}/structs_model.h
)

target_sources(LightbringEngine PRIVATE
    ${RENDERER_VULKAN_SOURCES}
)