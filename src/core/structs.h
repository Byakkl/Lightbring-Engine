#pragma once

#include <vector>
#include <glm/glm.hpp>

enum ComponentType{
    COMP_TRANSFORM,
    COMP_MATERIAL,
    COMP_MESH
};

class Component{
public:
    ComponentType type;
};

/// @brief Material component. Holds references to images to be passed to shaders
struct Material: Component{
    Image* albedo;

    Material(){
        type = ComponentType::COMP_MATERIAL;
    }
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

    Mesh(){
        type = ComponentType::COMP_MESH;
    }
    Mesh(std::vector<Vertex> _vertices, std::vector<int> _indices, unsigned char* _data){
        type = ComponentType::COMP_MESH;
        
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

struct Transform : Component{
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;

    Transform(){
        type = ComponentType::COMP_TRANSFORM;

        position = glm::vec3(0.0f);
        rotation = glm::vec3(0.0f);
        scale = glm::vec3(1.0f);
    }

    void setPosition(glm::vec3 newPosition){
        position = newPosition;
    }

    void setRotation(glm::vec3 newRotation){
        rotation = newRotation;
    }

    void setScale(glm::vec3 newScale){
        scale = newScale;
    }
};