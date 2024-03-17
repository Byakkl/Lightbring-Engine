set(CORE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/structs.h
    ${CMAKE_CURRENT_SOURCE_DIR}/primitives.h)

add_library(Core_Lib INTERFACE)

target_sources(Core_Lib INTERFACE ${CORE_SOURCES})

target_include_directories(Core_Lib INTERFACE
    "G:/Coding Libraries/glm-1.0.0")