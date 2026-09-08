#pragma once

#include <vector>
#include "engineStructs.h"
#include "mesh.h"
#include "texture.h"

struct ResourceHandle
{
    size_t resourceID;
    size_t dataIdx;
    bool valid;
};

class ResourceManager
{
    Renderer* rendererRef;

    //List of all image data
    std::vector<Texture> textures;

    //List of all mesh data
    std::vector<Mesh> meshes;

    //Index counter used with generation of resource handles
    size_t resourceIdx{};
    //Indirection of handles allow dense packing and handle invalidation
    std::vector<ResourceHandle> resourceHandles;

    ResourceHandle CreateResourceHandle(const size_t);
public:
    void AssignRendererRef(Renderer*);

    template<typename T>
    T& GetResource(const Handle<T>&);

    std::optional<Handle<Mesh>> ImportMesh(const char*);
    std::optional<Handle<Texture>> ImportTexture(const char*);

    void Shutdown();
};