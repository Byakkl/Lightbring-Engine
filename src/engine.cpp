#define NDEBUG

#include <iostream>
#include <chrono>
#include "engine.h"
#include "fileio/import_image.h"
#include "fileio/import_obj.h"
#include "primitives.h"

#ifdef RENDERER_VULKAN
#include "renderer/vulkan/sys_vulkan.h"
#endif

bool LightbringEngine::start(){
    //Flag the engine as running
    isRunning = true;    

    //Select the correct renderer based on preprocessor defines
    #ifdef RENDERER_VULKAN
    renderer = new VulkanRenderer();
    #endif

    //Initialize any engine data
    try{
        //Initialize the engine's renderer
        renderer->initialize(800, 600);
        renderer->windowResizedEvent.Register([this](uint32_t width, uint32_t height) {windowResizedCallback(width, height);});
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
        if(activeScene == nullptr)
            return true;
        
        //Static tracker for start time; set when this method is initially called
        static auto startTime = std::chrono::high_resolution_clock::now();
        
        //Get the current time 
        auto currentTime = std::chrono::high_resolution_clock::now();
        //Find the difference between static start time and current time
        float deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();


        //Update the active scene
        activeScene->update(deltaTime);

        //Render any active cameras
        for(auto camera : activeScene->sceneCameras){
            if(!camera->getIsRendering())
                continue;

            //TODO: Renderer preprocessing; eg CPU side object culling

            //Update the renderer and render the next frame
            isRunning = renderer->render(camera, activeScene->sceneObjects);
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
    isRunning = false;

    //Clean up any image data
    for(auto texture : textures){
        renderer->unloadTexture(texture);
        texture->releaseRawData();
        delete texture;
    }

    //Clean up any model data
    for(auto mesh : meshes){
        renderer->unloadMesh(mesh);
        mesh->releaseRawData();
        delete mesh;
    }

    //Clean up any cameras
    for(auto camera : cameras){
        renderer->unregisterCamera(camera);
        delete camera;
    }

    //Clean up the renderer
    if(renderer != nullptr){
        try{
            renderer->cleanup();
        } catch(const std::exception& e){
            std::cerr << e.what() << std::endl;
        }
        delete renderer;
    }
}

Texture* LightbringEngine::importImage(const char* filePath, bool pushToGPU){
    Texture* importedData;
    try{
        //Import the image data from the file
        importedData = importImageFile(filePath);

        //If the data is to be uploaded immediately; do so and clear the CPU data
        if(pushToGPU){
            renderer->createTexture(importedData);
            importedData->releaseRawData();
        }

        //Add the data structure to the engine's tracker
        textures.push_back(importedData);
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
            renderer->uploadMesh(importedData);
            importedData->releaseRawData();
        }

        //Add the data structure to the engine's tracker
        meshes.push_back(importedData);
    } catch(const std::exception& e){
        std::cerr << e.what() << std::endl;
    return nullptr;
    }
    return importedData;
}

bool LightbringEngine::uploadImage(Texture* imageData){
    //If this image's data has already been registered with the renderer don't upload it again
    if(imageData->rendererData != nullptr)
        return true;

    try{
        renderer->createTexture(imageData);
    } catch(const std::exception& e){
        std::cerr << e.what() << std::endl;
        return false;
    }
    return true;
}

bool LightbringEngine::uploadMesh(Mesh* meshData){
    //If this mesh's data has already been registered with the renderer don't upload it again
    if(meshData->rendererData != nullptr)
        return true;

    try{
        renderer->uploadMesh(meshData);
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
    meshes.push_back(output);
    //Return a pointer to the instance
    return output;
}

Scene* LightbringEngine::createScene(){
    //Create the instance
    Scene* emptyScene = new Scene();

    //Add it to the list of scenes
    scenes.push_back(emptyScene);

    //Return the pointer
    return emptyScene;
}

Object* LightbringEngine::createObject(){
    //Create the instance
    Object* emptyObject = new Object();

    //Add it to the list of objects
    objects.push_back(emptyObject);

    //Return the pointer
    return emptyObject;
}

void LightbringEngine::setActiveScene(Scene* scene){
    //If there is an active scene, unload the data
    if(activeScene != nullptr){
        //TODO: Unload active scene data
    }

    //TODO: Load new scene data

    //TODO: Register cameras with renderer
    for(auto camera : cameras)
        renderer->registerCamera(camera);

    //Track the active scene
    activeScene = scene;
}

Material* LightbringEngine::createMaterial(Texture* albedo){
    //Create the instance
    Material* material = new Material();

    //Assign the values
    material->albedo = albedo;

    //Add it to the list of materials
    materials.push_back(material);

    //Return the pointer
    return material;
}

Camera* LightbringEngine::createCamera(){
    //Create the camera instance
    Camera* camera = new Camera();

    //Initialize the aspect ratio
    float ratio = (float)(renderer->windowWidth) / (float)(renderer->windowHeight);
    camera->setAspectRatio(ratio);

    //Add it to the list of cameras
    cameras.push_back(camera);

    //Return the pointer
    return camera;
}

void LightbringEngine::setCameraActive(Camera* camera, bool active){
    //Return if call is redundant
    if(camera->getIsRendering() == active)
        return;
    
    //Un/register the camera as needed
    if(active)
        renderer->registerCamera(camera);
    else
        renderer->unregisterCamera(camera);

    //Update the camera's render flag
    camera->setIsRendering(active);
}

void LightbringEngine::windowResizedCallback(const uint32_t width, const uint32_t height){
    float newAspect = (float)width / (float)height;
    for(auto camera : cameras)
        camera->setAspectRatio(newAspect);
}

/// @brief This will be removed but is currently used as a quick and dirty way to test the engine as an exe
/// @return 
int main() {
    //Create an instance of the engine
    LightbringEngine* engine = new LightbringEngine();

    //Attempt to start up the engine. Should it fail then exit
    if(!engine->start()){
        delete engine;
        return EXIT_FAILURE;
    }

    //Game code to interact with the engine can be run now; eg loading assets
    //Import the test checker image
    Texture* testImage = engine->importImage("images/UVChecker_512.png");
    if(testImage == nullptr){
        //Currently not handling reattempting as this is a pseudo program
        throw std::runtime_error("Engine failed image import"); 
        engine->shutdown();
        return EXIT_FAILURE;
    }

    Material* material = engine->createMaterial(testImage);

    //Create an instance of a quad primitive
    Mesh* quadMesh = engine->createPrimitive(MeshPrimitive::PRIM_QUAD);
    //Upload the mesh data to the GPU
    engine->uploadMesh(quadMesh);
    //Release the raw data from memory
    quadMesh->releaseRawData();

    //Create an object
    Object* sceneObj = engine->createObject();
    //Add components to the object
    sceneObj->addComponent(quadMesh);
    sceneObj->addComponent(material);

    //Create a camera object
    Camera* cameraObj = engine->createCamera();
    cameraObj->transform->setPosition(glm::vec3(0.0f, 0.0f, -2.0f));

    //Create a scene
    Scene* scene = engine->createScene();
    //Include the object in the scene
    scene->addSceneObject(sceneObj);
    //Include the camera in the scene
    scene->addSceneCamera(cameraObj);
    //Set the scene as active
    engine->setActiveScene(scene);


    //Run the update loop. The condition would likely be different in a game that's using the engine
    while(engine->isRunning){
        //Attempt to update the engine. Should it fail then exit
        if(!engine->update()){
            delete engine;
            return EXIT_FAILURE;
        }
    }

    //Shut down the engine
    engine->shutdown();
    //Clean up the engine pointer
    delete engine;

    system("pause");
    return EXIT_SUCCESS;
}