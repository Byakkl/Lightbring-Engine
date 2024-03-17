#define NDEBUG

#include <iostream>
#include "engine.h"
#include "fileio/import_image.h"
#include "core/primitives.h"

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

    //Clean up the renderer
    if(renderer != nullptr){
        try{
            renderer->cleanup();
        } catch(const std::exception& e){
            std::cerr << e.what() << std::endl;
        }
        delete renderer;
    }

    //Clean up any image data
    for(auto image : images){
        image->clearData();
        delete image;
    }

    //Clean up any model data
}

Image* LightbringEngine::importImage(const char* filePath, bool pushToGPU){
    Image* importedData;
    try{
        //Import the image data from the file
        importedData = importImageFile(filePath);

        //If the data is to be uploaded immediately; do so and clear the CPU data
        if(pushToGPU){
            renderer->uploadImage(importedData);
            importedData->clearData();
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

Mesh* LightbringEngine::importModel(const char* filePath, bool pushToGPU){
    std::cerr << "Import model not yet implemented" << std::endl;
    return nullptr;
}

bool LightbringEngine::uploadImage(const Image* imageData){
    renderer->uploadImage(imageData);
}

bool LightbringEngine::uploadMesh(const Mesh* meshData){
    renderer->uploadMesh(meshData);
}

Mesh* LightbringEngine::createPrimitive(const MeshPrimitive primitive){
    //Container for the copy-constructed instance
    Mesh output;

    //Determine the type of primitive based on input value
    switch (primitive)
    {
    case MeshPrimitive::Quad:
        output = quad;
    
    default:
        return nullptr;
    }

    //Add the pointer to the engine's tracker
    meshes.push_back(&output);
    //Return a pointer to the instance
    return &output;
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

    //Create an instance of a quad primitive
    Mesh* quadMesh = engine->createPrimitive(MeshPrimitive::Quad);
    //Upload the mesh data to the GPU
    engine->uploadMesh(quadMesh);
    //Release the CPU memory
    quadMesh->clearData();


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