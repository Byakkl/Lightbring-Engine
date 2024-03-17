#pragma once

#include "../core/structs.h"

class Renderer {
public:
    //Pure virtual method used to initialize a renderer
    virtual void initialize() = 0;
    //Pure virtual method used to run the renderer
    virtual bool render() = 0;
    //Pure virtual method used to clean up the renderer
    virtual void cleanup() = 0;

    //Moves an image's data to the GPU
    virtual void uploadImage(const Image*) = 0;
    //Moves a mesh's data to the GPU
    virtual void uploadMesh(const Mesh*) = 0;
};