#define NDEBUG

#include <iostream>
#include <chrono>
#include <mutex>
#include "engine_p.h"
#include "fileio/import_image.h"
#include "fileio/import_obj.h"
#include "primitives.h"
#include "rendererData.h"

#ifdef RENDERER_VULKAN
#include "renderer/vulkan/sys_vulkan.h"
#endif

//Definition of static implementation pointer
std::unique_ptr<LightbringEngine::LightbringEngineImpl> LightbringEngine::pImpl = nullptr;
//Flag to ensure initialization only occurs once
std::once_flag pImplInitFlag;

LightbringEngine::LightbringEngine() {
    //Create the static instance of the engine implementation
    std::call_once(pImplInitFlag, []() {
        pImpl = std::make_unique<LightbringEngineImpl>();
    });
}

LightbringEngine::~LightbringEngine(){}

bool LightbringEngine::start(){
    //Flag the engine as running
    pImpl->isRunning = true;    

    //Initialize the previous frame tracker used for delta time calc
    pImpl->prevFrameTime = std::chrono::high_resolution_clock::now();

    //Initialize any engine data
    try{
        //Initialize the engine's renderer
        pImpl->renderer->initialize(800, 600);
        pImpl->renderer->windowResizedEvent.Register([this](uint32_t width, uint32_t height) {pImpl->windowResizedCallback(width, height);});
    } catch (const std::exception& e){
        std::cerr << e.what() << std::endl;
        shutdown();
        return false;
    }
    return true;
}

bool LightbringEngine::update(){
    //Run per-frame engine code
    try{
        //If there is no active scene skip updating
        if(pImpl->activeScene == nullptr)
            return true;
        
        //Get the current time 
        auto currentTime = std::chrono::high_resolution_clock::now();
        //Find the difference between last frame's time and current time
        deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - pImpl->prevFrameTime).count();

        //Update the timestamp for the start of the previous previous frame time to the current frame start time
        pImpl->prevFrameTime = currentTime;

        //Update the active scene
        pImpl->activeScene->update(deltaTime);

        //Render any active cameras
        for(auto camera : pImpl->activeScene->sceneCameras){
            if(!camera->getIsRendering())
                continue;

            //TODO: Renderer preprocessing; eg CPU side object culling

            //Update the renderer and render the next frame
            pImpl->isRunning = pImpl->renderer->render(camera, pImpl->activeScene->sceneObjects);
        }
    } catch (const std::exception& e){
        std::cerr << e.what() << std::endl;
        shutdown();
        return false;
    }
    return true;
}

void LightbringEngine::shutdown(){
    //Turn off the running flag
    pImpl->isRunning = false;

    //Clean up any image data
    for(auto texture : pImpl->textures){
        pImpl->renderer->unloadTexture(texture);
        texture->pRendererData->releaseRawData();
        delete texture;
    }

    //Clean up any model data
    for(auto mesh : pImpl->meshes){
        pImpl->renderer->unloadMesh(mesh);
        mesh->pRendererData->releaseRawData();
        delete mesh;
    }

    //Clean up any cameras
    for(auto camera : pImpl->cameras){
        pImpl->renderer->unregisterCamera(camera);
        delete camera;
    }

    //Clean up the renderer
    if(pImpl->renderer != nullptr){
        try{
            pImpl->renderer->cleanup();
        } catch(const std::exception& e){
            std::cerr << e.what() << std::endl;
        }
        delete pImpl->renderer;
    }
}

Texture* LightbringEngine::importImage(const char* filePath, bool pushToGPU){
    Texture* importedData;
    try{
        //Import the image data from the file
        importedData = importImageFile(filePath);

        //If the data is to be uploaded immediately; do so and clear the CPU data
        if(pushToGPU){
            pImpl->renderer->createTexture(importedData);
            importedData->pRendererData->releaseRawData();
        }

        //Add the data structure to the engine's tracker
        pImpl->textures.push_back(importedData);
    }
    catch(const std::exception& e){
        std::cerr << e.what() << std::endl;
        return nullptr;
    }
    return importedData;
}

Mesh* LightbringEngine::importMesh(const char* filePath, bool pushToGPU){
    Mesh* importedData;
    try{
        //Import the model data from the file
        importedData = importModelFile(filePath);

        //If the data is to be uploaded immediately; do so and clear the CPU data
        if(pushToGPU){
            pImpl->renderer->uploadMesh(importedData);
            importedData->pRendererData->releaseRawData();
        }

        //Add the data structure to the engine's tracker
        pImpl->meshes.push_back(importedData);
    } catch(const std::exception& e){
        std::cerr << e.what() << std::endl;
        return nullptr;
    }
    return importedData;
}

bool LightbringEngine::uploadImage(Texture* imageData){
    //If this image's data has already been registered with the renderer don't upload it again
    if(imageData->pRendererData->rendererData != nullptr)
        return true;

    try{
        pImpl->renderer->createTexture(imageData);
    } catch(const std::exception& e){
        std::cerr << e.what() << std::endl;
        return false;
    }
    return true;
}

bool LightbringEngine::uploadMesh(Mesh* meshData){
    //If this mesh's data has already been registered with the renderer don't upload it again
    if(meshData->pRendererData != nullptr)
        return true;

    try{
        pImpl->renderer->uploadMesh(meshData);
    } 
    catch(const std::exception& e){
        std::cerr << e.what() << std::endl;
        return false;
    }
    return true;
}

Mesh* LightbringEngine::createPrimitive(MeshPrimitive primitive){
    //Container for the copy-constructed instance
    Mesh* output;

    //Determine the type of primitive based on input value
    switch (primitive)
    {
    case MeshPrimitive::PRIM_QUAD:
        output = new Mesh(quad);
        break;
    default:
        return nullptr;
    }

    //Add the pointer to the engine's tracker
    pImpl->meshes.push_back(output);
    //Return a pointer to the instance
    return output;
}

Scene* LightbringEngine::createScene(){
    //Create the instance
    Scene* emptyScene = new Scene();

    //Add it to the list of scenes
    pImpl->scenes.push_back(emptyScene);

    //Return the pointer
    return emptyScene;
}

Object* LightbringEngine::createObject(){
    //Create the instance
    Object* emptyObject = new Object();

    //Add it to the list of objects
    pImpl->objects.push_back(emptyObject);

    //Return the pointer
    return emptyObject;
}

void LightbringEngine::setActiveScene(Scene* scene){
    //If there is an active scene, unload the data
    if(pImpl->activeScene != nullptr){
        //TODO: Unload active scene data
    }

    //TODO: Load new scene data

    //TODO: Register cameras with renderer
    for(auto camera : pImpl->cameras)
        pImpl->renderer->registerCamera(camera);

    //Track the active scene
    pImpl->activeScene = scene;
}

Material* LightbringEngine::createMaterial(Texture* albedo){
    //Create the instance
    Material* material = new Material();

    //Assign the values
    material->albedo = albedo;

    //Add it to the list of materials
    pImpl->materials.push_back(material);

    //Return the pointer
    return material;
}

Camera* LightbringEngine::createCamera(){
    //Create the camera instance
    Camera* camera = new Camera();

    //Initialize the aspect ratio
    float ratio = (float)(pImpl->renderer->windowWidth) / (float)(pImpl->renderer->windowHeight);
    camera->setAspectRatio(ratio);

    //Add it to the list of cameras
    pImpl->cameras.push_back(camera);

    //Return the pointer
    return camera;
}

void LightbringEngine::setCameraActive(Camera* camera, bool active){
    //Return if call is redundant
    if(camera->getIsRendering() == active)
        return;
    
    //Un/register the camera as needed
    if(active)
        pImpl->renderer->registerCamera(camera);
    else
        pImpl->renderer->unregisterCamera(camera);

    //Update the camera's render flag
    camera->setIsRendering(active);
}

void LightbringEngine::setVertexShaderPath(std::string path){
    pImpl->renderer->vertexShaderPath = path;
}

void LightbringEngine::setFragmentShaderPath(std::string path){
    pImpl->renderer->fragmentShaderPath = path;
}

LightbringEngine::LightbringEngineImpl::LightbringEngineImpl(){
    //Select the correct renderer based on preprocessor defines
    #ifdef RENDERER_VULKAN
    renderer = new VulkanRenderer();
    #endif
    
    activeScene = nullptr;
}

LightbringEngine::LightbringEngineImpl::~LightbringEngineImpl(){
    //Clean up the renderer
    if(renderer)
        delete renderer;
}

void LightbringEngine::LightbringEngineImpl::windowResizedCallback(const uint32_t width, const uint32_t height){
    float newAspect = (float)width / (float)height;
    for(auto camera : cameras)
        camera->setAspectRatio(newAspect);
}