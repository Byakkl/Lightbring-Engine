#pragma once

#include <memory>
#include "renderer.h"
#include "scene.h"
#include "camera.h"
#include "material.h"
#include "mesh.h"
#include "texture.h"
#include "meshPrimitive.h"

class LightbringEngine{
public:
    /// @brief Delta time in seconds since last frame
    float deltaTime;

    LightbringEngine();
    ~LightbringEngine();

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
    Texture* importImage(const char*, bool = true);

    /// @brief Pushes the provided image's data to the GPU through the renderer
    /// @param imageData Pointer to the image whose data is to be uploaded
    /// @return True if successful or data is already uploaded
    bool uploadImage(Texture*);

    /// @brief Pushes the provided mesh's data to the GPU through the renderer
    /// @param meshData Pointer to the mesh whose data is to be uploaded
    /// @return True if successful or data is already uploaded
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
    /// @return Returns a pointer to the created Material instance
    Material* createMaterial(Texture* = nullptr);

    /// @brief Creates a camera object
    /// @return Returns a pointer to the camera instance 
    Camera* createCamera();

    /// @brief Sets a camera to actively render or not
    /// @param camera Pointer to the camera to change the render state of
    /// @param active When True a camera will render every frame
    void setCameraActive(Camera*, bool);

    void setVertexShaderPath(std::string);

    void setFragmentShaderPath(std::string);

private:
    class LightbringEngineImpl;
    static std::unique_ptr<LightbringEngineImpl> pImpl;
};