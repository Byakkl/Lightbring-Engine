#define NDEBUG

#include <iostream>
#include <chrono>
#include <mutex>
#include "engine_p.h"
#include "fileio/import_image.h"
#include "fileio/import_obj.h"
#include "primitives.h"
#include "rendererData.h"
#include "input_internal.h"

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
        pImpl->initializeWindow(800,600);
        //Initialize the engine's renderer
        pImpl->renderer->initialize(pImpl->window, 800, 600, std::ref(pImpl->windowResizedEvent));
        //Initialize input system
        pImpl->initializeInput(pImpl->window);
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

        
        if(glfwWindowShouldClose(pImpl->window))
            return false;

        glfwPollEvents();


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
        delete texture;
    }

    //Clean up any model data
    for(auto mesh : pImpl->meshes){
        pImpl->renderer->unloadMesh(mesh);
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
    float ratio = (float)(pImpl->windowWidth) / (float)(pImpl->windowHeight);
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

    //Clean up the created window
    glfwDestroyWindow(window);

    //Clean up GLFW
    glfwTerminate();
}

void LightbringEngine::LightbringEngineImpl::initializeWindow(const int a_width, const int a_height){
            //Initialize GLFW
        glfwInit();
        //Disable creation of OpenGL context
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        //Create a new window with resolution 800x600 and name Vulkan
        //4th parameter can specify a monitor to open on
        //5th parameter is relevant to OpenGL
        window = glfwCreateWindow(a_width, a_height, "Vulkan", nullptr, nullptr);
        //Set a pointer to this instance of VulkanRenderer that can be used in the callback
        glfwSetWindowUserPointer(window, this);
        //Set a callback to be invoked by GLFW when the window is resized
        glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);

        windowWidth = a_width;
        windowHeight = a_height;
        //Invoke the window resize event now that the window is created. There may be no listeners at this time but it's consistent at least
        windowResizedEvent.Invoke(windowWidth, windowHeight);
}

void LightbringEngine::LightbringEngineImpl::initializeInput(GLFWwindow* a_window){
    if(input == nullptr)
        input = new Input_Internal();

    input->initialize(a_window);
}

void LightbringEngine::LightbringEngineImpl::framebufferResizeCallback(GLFWwindow* a_window, int a_width, int a_height){
    //Cast the pointer to the engine implementation class
    auto app = reinterpret_cast<LightbringEngineImpl*>(glfwGetWindowUserPointer(a_window));
        
    //Pauses while the window is minimized
    glfwGetFramebufferSize(a_window, &a_width, &a_height);
    while(a_width == 0 || a_height == 0){
        glfwGetFramebufferSize(a_window, &a_width, &a_height);
        glfwWaitEvents();
    }
    app->windowWidth = a_width;
    app->windowHeight = a_height;
    //Update the camera aspect ratios
    float newAspect = (float)a_width / (float)a_height;
    for(auto camera : app->cameras)
        camera->setAspectRatio(newAspect);

    //Invoke the window resize method to inform listening systems of the change
    app->windowResizedEvent.Invoke(app->windowWidth, app->windowHeight);
}