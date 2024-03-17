#pragma once

#include <vector>
#include <glm/glm.hpp>

struct Image{
    int width;
    int height;
    int channels;
    unsigned char* data;

    //Releases the CPU memory and clears the pointer
    void clearData(){
        if(data != nullptr)
            free(data);
        data = nullptr;
    }
};

struct Vertex {
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 uv;
};

struct Mesh{
    std::vector<Vertex> vertices;
    std::vector<int> indices;
    unsigned char* data;

    void clearData(){
        if(data != nullptr)
            free(data);
        data = nullptr;
    }
};