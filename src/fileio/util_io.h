#pragma once

#include <vector>
#include <fstream>

/// @brief Helper function to open and read a file
/// @param filename 
/// @return 
static std::vector<char> readFile(const std::string& filename){
    //std::ios::ate -> Open the file and immediately seek to the end file
    //std::ios::binary -> Read the file as a binary file
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    //If the file failed to open; throw
    if(!file.is_open())
        throw std::runtime_error("Failed to open file");

    //Get the size of the file
    size_t fileSize = (size_t) file.tellg();
    //Create a vector that can contain the entire file
    std::vector<char> buffer(fileSize);

    //Seek to the start of the file
    file.seekg(0);
    //Read the contents of the file into the buffer
    file.read(buffer.data(), fileSize);
    //Close the file
    file.close();

    return buffer;
}