#pragma once

#include "renderer/renderer.h"
#include "fileio/import_image.h"

class LightbringEngine{
public:
    bool isRunning;

    bool start();
    bool update();
    void shutdown();

    /// @brief Imports a model
    /// @param filePath The file path of the model to be imported
    /// @param pushToGPU If true the mesh data will be pushed to the GPU immediately and the CPU memory will be released
    /// @return Returns a pointer to the imported data structure if successful. Nullptr if not
    Mesh* importModel(const char*, bool = true);

    /// @brief Imports an image
    /// @param filePath The file path of the image to be imported
    /// @param pushToGPU If true the image data will be pushed to the GPU immediately and the CPU memory will be released
    /// @return Returns a pointer to the imported data structure if successful. Nullptr if not
    Image* importImage(const char*, bool = true);

    bool uploadImage(const Image*);

    bool uploadMesh(const Mesh*);

    /// @brief Creates an instance of a mesh primitive
    /// @param primitive The enum value indicating the type of primitive to generate
    /// @return Returns a pointer to the primitive instance
    Mesh* createPrimitive(const MeshPrimitive);

private:
    Renderer* renderer;
    std::vector<Image*> images;
    std::vector<Mesh*> meshes;
};