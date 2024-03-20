#pragma once

#include "structs.h"

class Renderer {
public:
    //Pure virtual method used to initialize a renderer
    virtual void initialize() = 0;
    //Pure virtual method used to run the renderer
    virtual bool render(std::vector<Object*>) = 0;
    //Pure virtual method used to clean up the renderer
    virtual void cleanup() = 0;

    //Moves an image's data to the GPU
    virtual void uploadImage(Image*) = 0;
    //Unloads an image from GPU memory
    virtual void unloadImage(Image*) = 0;

    //Moves a mesh's data to the GPU
    virtual void uploadMesh(Mesh*) = 0;
    //Unloads a mesh from GPU memory
    virtual void unloadMesh(Mesh*) = 0;

    //Registers a camera to the renderer, creating relevant data structures
    virtual void registerCamera(Camera*) = 0;
    //Unregisters a camera from the renderer, cleaning up any created data structures
    virtual void unregisterCamera(Camera*) = 0;
};