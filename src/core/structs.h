#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

enum ComponentType{
    COMP_TRANSFORM,
    COMP_MATERIAL,
    COMP_MESH
};

class Component{
public:
    //Stores the type of component it is
    ComponentType type;
    //TODO: Determine if this will be used with a pre-defined render data or ignored in the case of something like Vulkan render batching with a recycled pool of descriptor sets
    //Tracks if any changes have been made to the component that need to be reflected in other systems
    bool isDirty;
};

struct Texture{
    int width;
    int height;
    int channels;
    //Byte array of raw imported data
    unsigned char* rawData;
    //Void pointer used by the renderer to store its specific data container to reference when rendering
    void* rendererData;

    Texture(){
        width = 0;
        height = 0;
        channels = 0;

        rawData = nullptr;
        rendererData = nullptr;
    }

    Texture(int _width, int _height, int _channels = 4){
        width = _width;
        height = _height;
        channels = _channels;

        rawData = nullptr;
        rendererData = nullptr;
    }

    void releaseRawData(){
        if(rawData != nullptr)
            free(rawData);
        rawData = nullptr;
    }
};

/// @brief Material component. Holds references to images to be passed to shaders
struct Material: Component{
    Texture* albedo;

    Material(){
        type = ComponentType::COMP_MATERIAL;
    }
};

struct Vertex {
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 uv;
};

struct Mesh : Component{
    std::vector<Vertex> vertices;
    std::vector<uint16_t> indices;
    //Byte array of raw imported data
    unsigned char* rawData;
    //Void pointer used by the renderer to store its specific data container to reference when rendering
    void* rendererData;

    Mesh(){
        type = ComponentType::COMP_MESH;
        rawData = nullptr;
        rendererData = nullptr;
    }
    Mesh(std::vector<Vertex> _vertices, std::vector<uint16_t> _indices, unsigned char* _data){
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

    //Void pointer used by the renderer to store its specific data container to reference when rendering
    void* rendererData;

    Transform(){
        type = ComponentType::COMP_TRANSFORM;

        position = glm::vec3(0.0f);
        rotation = glm::vec3(0.0f);
        scale = glm::vec3(1.0f);

        rendererData = nullptr;

        //Set initial flag to true to ensure it gets passed in initially
        isDirty = true;
    }

    void setPosition(glm::vec3 newPosition){
        position = newPosition;
        isDirty = true;
    }

    void setRotation(glm::vec3 newRotation){
        rotation = newRotation;
        isDirty = true;
    }

    void setScale(glm::vec3 newScale){
        scale = newScale;
        isDirty = true;
    }

    glm::mat4 getRotationMatrix(){
        //ZXY rotation order
        glm::mat4 rotationMatrix = glm::mat4(1.0f);

        rotationMatrix = glm::rotate(rotationMatrix, rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
        rotationMatrix = glm::rotate(rotationMatrix, rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
        rotationMatrix = glm::rotate(rotationMatrix, rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));

        return rotationMatrix;
    }

    glm::mat4 getTransformMatrix(){
        glm::mat4 transformMatrix = glm::translate(glm::mat4(1.0f), position);
        transformMatrix *= getRotationMatrix();
        transformMatrix = glm::scale(transformMatrix, scale);

        return transformMatrix; 
    }
};