#Specify the files to be included in this subdirectory library
set(ECS_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/entity_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ecsManager_p.h
)

target_sources(LightbringEngine PRIVATE
    ${ECS_SOURCES}
)