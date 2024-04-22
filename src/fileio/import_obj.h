#pragma once
#define TINYOBJLOADER_IMPLEMENTATION

#include <stdexcept>
#include <tiny_obj_loader.h>
#include "structs.h"
#include <iostream>

static Mesh* importModelFile(const char* modelPath){
    Mesh* mesh = new Mesh();

    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn,err;

    if(!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, modelPath)){
        throw std::runtime_error(warn + err);
    }

    for(const auto& shape : shapes){
        for(const auto& index : shape.mesh.indices){
            Vertex vertex{};
            
            vertex.pos = {
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2]
            };

            vertex.uv = {
                attrib.texcoords[2 * index.texcoord_index + 0],
                //Flip Y tex coord to match Y flip of renderer
                1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
            };

            vertex.color = {1.0f, 1.0f, 1.0f};

            mesh->vertices.push_back(vertex);
            mesh->indices.push_back(mesh->indices.size());
        }
    }

    return mesh;
}