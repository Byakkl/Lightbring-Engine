set(CORE_SOURCES
    structs.h
    primitives.h
    object.cpp
    scene.cpp
    renderer.h
    camera.cpp
    event.h)

add_library(Core_Lib ${CORE_SOURCES})

#Link the dependent libraries
target_link_libraries(Core_Lib PRIVATE
    glm
    )