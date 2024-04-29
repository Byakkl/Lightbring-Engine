#pragma once

#include <string>
#include "event.h"
#include "mesh.h"
#include "camera.h"
#include "object.h"

class Renderer {
public:
    /// @brief Event invoked when the window is resized
    Event<uint32_t, uint32_t> windowResizedEvent;

    /// @brief Stores the width of the window
    uint32_t windowWidth;

    /// @brief Stores the height of the window
    uint32_t windowHeight;

    /// @brief Path to the vertex shader file
    std::string vertexShaderPath;

    /// @brief Path to the fragment shader file
    std::string fragmentShaderPath;

    /// @brief Pure virtual method used to initialize a renderer
    virtual void initialize(uint32_t, uint32_t) = 0;

    /// @brief Pure virtual method used to run the renderer
    /// @param camera The camera that is being rendered
    /// @param objects The objects to be rendered
    virtual bool render(Camera*, std::vector<Object*>) = 0;

    /// @brief Pure virtual method used to clean up the renderer
    virtual void cleanup() = 0;

    /// @brief Creates a texture with the given parameters. If data is present in Texture.rawData it will be copied into the texture created by the renderer
    /// @param texture The data container that contains the initial texture data
    virtual void createTexture(Texture*) = 0;
    /// @brief Unloads a texture from GPU memory. This will release any structures created by the renderer and contained in Texture.rendererData
    /// @param texture The data container that holds the texture data
    virtual void unloadTexture(Texture*) = 0;

    /// @brief Moves a mesh's data to the GPU
    /// @param mesh The 
    virtual void uploadMesh(Mesh*) = 0;
    /// @brief Unloads a mesh from GPU memory
    virtual void unloadMesh(Mesh*) = 0;

    /// @brief Registers a camera to the renderer, creating relevant data structures
    virtual void registerCamera(Camera*) = 0;
    /// @brief Unregisters a camera from the renderer, cleaning up any created data structures
    virtual void unregisterCamera(Camera*) = 0;
};