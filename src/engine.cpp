#define NDEBUG

#include <iostream>
#include "engine.h"
#include "fileio/import_image.h"
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
        renderer->initialize();
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
        
        //Update the active scene
        activeScene->update();

        //Update the renderer and render the next frame
        isRunning = renderer->render();
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
    for(auto image : images){
        renderer->unloadImage(image);
        image->releaseRawData();
        delete image;
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

Image* LightbringEngine::importImage(const char* filePath, bool pushToGPU){
    Image* importedData;
    try{
        //Import the image data from the file
        importedData = importImageFile(filePath);

        //If the data is to be uploaded immediately; do so and clear the CPU data
        if(pushToGPU){
            renderer->uploadImage(importedData);
            importedData->releaseRawData();
        }

        //Add the data structure to the engine's tracker
        images.push_back(importedData);
    }
    catch(const std::exception& e){
        std::cerr << e.what() << std::endl;
        return nullptr;
    }
    return importedData;
}

Mesh* LightbringEngine::importMesh(const char* filePath, bool pushToGPU){
    std::cerr << "Import model not yet implemented" << std::endl;
    return nullptr;
}

bool LightbringEngine::uploadImage(Image* imageData){
    //If this image's data has already been registered with the renderer don't upload it again
    if(imageData->rendererData != nullptr)
        return true;

    try{
        renderer->uploadImage(imageData);
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
    Mesh output;

    //Determine the type of primitive based on input value
    switch (primitive)
    {
    case MeshPrimitive::PRIM_QUAD:
        output = quad;
    
    default:
        return nullptr;
    }

    //Add the pointer to the engine's tracker
    meshes.push_back(&output);
    //Return a pointer to the instance
    return &output;
}

Scene* LightbringEngine::createScene(){
    //Create the instance
    Scene* emptyScene = new Scene;

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

    //Track the active scene
    activeScene = scene;
}

Material* LightbringEngine::createMaterial(Image* albedo){
    //Create the instance
    Material* material = new Material;

    //Assign the values
    material->albedo = albedo;

    //Add it to the list of materials
    materials.push_back(material);

    //Return the pointer
    return material;
}

Camera* LightbringEngine::createCamera(){
    //Create the camera instance
    Camera* camera = new Camera;

    //Add it to the list of cameras
    cameras.push_back(camera);

    //Return the pointer
    return camera;
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
    Image* testImage = engine->importImage("test images/UVChecker_512.png");
    if(testImage == nullptr){
        //Currently not handling reattempting as this a pseudo program
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