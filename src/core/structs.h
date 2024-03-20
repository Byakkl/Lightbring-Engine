#pragma once

#include <vector>
#include <glm/glm.hpp>

enum ComponentType{
    COMP_MATERIAL,
    COMP_MODEL,
};

class Component{
public:
    ComponentType type;
};

/// @brief Material component. Holds references to images to be passed to shaders
struct Material: Component{
    Image* albedo;
};

struct Image{
    int width;
    int height;
    int channels;
    //Byte array of raw imported data
    unsigned char* rawData;
    //Void pointer used by the renderer to store its specific data container to reference when rendering
    void* rendererData;

    void releaseRawData(){
        if(rawData != nullptr)
            free(rawData);
        rawData = nullptr;
    }
};

struct Vertex {
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 uv;
};

struct Mesh : Component{
    std::vector<Vertex> vertices;
    std::vector<int> indices;
    //Byte array of raw imported data
    unsigned char* rawData;
    //Void pointer used by the renderer to store its specific data container to reference when rendering
    void* rendererData;

    Mesh();
    Mesh(std::vector<Vertex> _vertices, std::vector<int> _indices, unsigned char* _data){
        vertices = _vertices;
        indices = _indices;
        rawData = _data;
    }

    void releaseRawData() {
        if(rawData != nullptr)
            free(rawData);
        rawData = nullptr;
    }
};