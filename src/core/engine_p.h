#pragma once

#include "engine.h"

#include "renderer.h"
#include "scene.h"
#include "camera_p.h"
#include "primitives.h"
#include "material.h"
#include "mesh.h"
#include "texture.h"

class LightbringEngine::LightbringEngineImpl{
public:
    bool isRunning;

    /// @brief Stores the time point of the previous frame. Initialized as current time when engine is started
    std::chrono::_V2::system_clock::time_point prevFrameTime;

    //The active renderer instance
    Renderer* renderer;

    //List of all image handles
    std::vector<Texture*> textures;
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
    //List of all created cameras
    std::vector<Camera*> cameras;

    LightbringEngineImpl();

    /// @brief Method used internally to respond to window resize event invocations
    /// @param width The new width of the window
    /// @param height The new height of the window
    void windowResizedCallback(const uint32_t, const uint32_t);
};