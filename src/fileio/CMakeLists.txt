#Specify the files to be included
set(FILE_IO_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/util_io.h
    ${CMAKE_CURRENT_SOURCE_DIR}/import_image.h
    ${CMAKE_CURRENT_SOURCE_DIR}/import_obj.h
)

target_sources(LightbringEngine PRIVATE
    ${FILE_IO_SOURCES}
)