#include "mesh.h"
#include "rendererData.h"

Mesh::Mesh() 
    : pRendererData(std::make_unique<RendererData>()){
    type = ComponentType::COMP_MESH;

    pRendererData->rawData = nullptr;
    pRendererData->rendererData = nullptr;
}

Mesh::Mesh(const Mesh& mesh)
    : pRendererData(std::make_unique<RendererData>()){
    type = ComponentType::COMP_MESH;

    vertices = mesh.vertices;
    indices = mesh.indices;

    pRendererData->rawData = nullptr;
    pRendererData->rendererData = nullptr;
}

Mesh::Mesh(std::vector<Vertex> _vertices, std::vector<uint16_t> _indices, unsigned char* _data)
    : pRendererData(std::make_unique<RendererData>()){
    type = ComponentType::COMP_MESH;
        
    vertices = _vertices;
    indices = _indices;

    pRendererData->rawData = _data;
    pRendererData->rendererData = nullptr;
}