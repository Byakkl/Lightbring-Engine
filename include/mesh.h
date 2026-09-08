#pragma once

#include <memory>
#include <vector>
#include "vertex.h"
#include "component.h"

class RendererData;
class Mesh {
public:
    //Container of Renderer specific data
    std::unique_ptr<RendererData> pRendererData;

    std::vector<Vertex> vertices;
    std::vector<uint16_t> indices;
    
    Mesh();
    Mesh(std::vector<Vertex>, std::vector<uint16_t>, unsigned char*);

    size_t resourceID;
};