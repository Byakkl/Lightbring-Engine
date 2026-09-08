#pragma once

#include <cstddef>
#include "mesh.h"
#include "material.h"

//Templated Handle structure to allow clear distinction between handle targets
template<typename T>
struct Handle
{
    size_t id;
    //Something like an iteration/generation could be defined later if reusing IDs is desired later
};

struct RenderDescription
{
    Handle<Mesh> mesh;
    Handle<Material> material; 
};