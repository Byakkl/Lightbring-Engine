#pragma once

#include "renderer.h"
#include "fileio/import_image.h"
#include "scene.h"
#include "structs.h"

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
    Mesh* importMesh(const char*, bool = true);

    /// @brief Imports an image
    /// @param filePath The file path of the image to be imported
    /// @param pushToGPU If true the image data will be pushed to the GPU immediately and the CPU memory will be released
    /// @return Returns a pointer to the imported data structure if successful. Nullptr if not
    Image* importImage(const char*, bool = true);

    bool uploadImage(Image*);

    bool uploadMesh(Mesh*);

    /// @brief Creates an instance of a mesh primitive
    /// @param primitive The enum value indicating the type of primitive to generate
    /// @return Returns a pointer to the primitive instance
    Mesh* createPrimitive(MeshPrimitive);

    /// @brief Creates an empty scene
    /// @return Returns a pointer to the scene instance
    Scene* createScene();

    /// @brief Creates an empty object
    /// @return Returns a pointer to the object instance
    Object* createObject();

    /// @brief Sets the active scene
    /// @param scene Pointer to the scene to be made active
    void setActiveScene(Scene*);

    /// @brief Creates a material component
    /// @param albedo Sets the albedo to the provided image
    /// @return Returns the created Material instance
    Material* createMaterial(Image* = nullptr);

private:
    //The active renderer instance
    Renderer* renderer;

    //List of all image handles
    std::vector<Image*> images;
    //List of all mesh handles
    std::vector<Mesh*> meshes;
    //List of all loaded scenes
    std::vector<Scene*> scenes;
    //Pointer to the active scene
    Scene* activeScene;
    //List of all created objects
    std::vector<Object*> objects;
    //List of all created materials
    std::vector<Material*> materials;
};