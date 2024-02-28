#Specify the files to be included
set(FILE_IO_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/util_io.h)

#Add the library; indicate it is an Interface as it is header only
add_library(File_IO_Lib INTERFACE)

#Specify the source files for the library and indicate it is an Interface
target_sources(File_IO_Lib INTERFACE ${FILE_IO_SOURCES})