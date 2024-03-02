#pragma once
#include <vector>
#include <array>
#include <glm/glm.hpp>
#include <vulkan/vulkan_core.h>

/// @brief Defines a vertex to be used with Vulkan
struct Vertex {
    //Vertex position
    glm::vec2 pos;
    //Vertex color
    glm::vec3 color;

    /// @brief Method to inform Vulkan the size of the data and at what rate to load it
    /// @return 
    static VkVertexInputBindingDescription getBindingDescription(){
        VkVertexInputBindingDescription bindingDescription{};
        //Index of the binding in the array of bindings
        bindingDescription.binding = 0;
        //Size of each per-vertex data entry
        bindingDescription.stride = sizeof(Vertex);
        //Rate to move to next data entry. _INSTANCE used with instanced rendering
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    /// @brief 
    /// @return 
    static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions(){
        //Array that contains the attribute descriptions of the Vertex structure
        std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};
        //Position data description
        //Index to binding that Vulkan will use
        attributeDescriptions[0].binding = 0;
        //Index to location directive used in vertex shader
        attributeDescriptions[0].location = 0;
        //Describes type of data for the attribute. Implicitly defines the byte size of the attribute data
        attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
        //Specifies the number of bytes since the start of the per-vertex data
        attributeDescriptions[0].offset = offsetof(Vertex, pos);

        //Color data description
        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, color);

        return attributeDescriptions;
    }
};

const std::vector<Vertex> triangleVerts = {
    {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
    {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
};