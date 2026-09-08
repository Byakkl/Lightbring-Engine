
#include <optional>
#include "renderer.h"
#include "resources.h"
#include "import_obj.h"
#include "import_image.h"

void ResourceManager::AssignRendererRef(Renderer* renderer)
{
    rendererRef = renderer;
}


template<typename T>
T& ResourceManager::GetResource(const Handle<T>& handle)
{
    ResourceHandle rHandle = resourceHandles[handle.id];
    if(!rHandle.valid)
        return NULL;

    if constexpr (std::is_same<T, Mesh>::value)
        return meshes[rHandle.dataIdx];
    else if constexpr (std::is_same<T, Texture>::value)
        return textures[rHandle.dataIdx];
    else
        static_assert("Unsupported data type");
}

std::optional<Handle<Mesh>> ResourceManager::ImportMesh(const char* filePath){
    Mesh importedData;
    Handle<Mesh> handle;
    try{
        //Import the model data from the file
        importedData = importModelFile(filePath);
        
        //Add the data structure to the engine's tracker
        meshes.push_back(importedData);

        //Create the resource handle
        ResourceHandle rHandle = CreateResourceHandle(meshes.size() - 1);
        rHandle.dataIdx = meshes.size() - 1;
        rHandle.resourceID = resourceIdx++;
        rHandle.valid = true;

        //Add the resource handle to the engine's tracker
        resourceHandles.push_back(rHandle);

        //Set the typed handle to point to the resource
        handle.id = rHandle.resourceID;
        //Link the mesh to the resource; this is used for reorganizing memory
        importedData.resourceID = rHandle.resourceID;
    } catch(const std::exception& e){
        std::cerr << e.what() << std::endl;
        return std::nullopt;
    }
    return std::make_optional(handle);
}

std::optional<Handle<Texture>> ResourceManager::ImportTexture(const char* filePath){
    Texture importedData;
    Handle<Texture> handle;
    try{
        //Import the image data from the file
        importedData = importImageFile(filePath);

        //Add the data structure to the engine's tracker
        textures.push_back(importedData);

        //Create the resource handle
        ResourceHandle rHandle = CreateResourceHandle(textures.size() - 1);
        rHandle.dataIdx = textures.size() - 1;
        rHandle.resourceID = resourceIdx++;
        rHandle.valid = true;

        //Add the resource handle to the engine's tracker
        resourceHandles.push_back(rHandle);

        //Set the typed handle to point to the resource
        handle.id = rHandle.resourceID;
        //Link the mesh to the resource; this is used for reorganizing memory
        importedData.resourceID = rHandle.resourceID;
    }
    catch(const std::exception& e){
        std::cerr << e.what() << std::endl;
        return std::nullopt;
    }
    return std::make_optional(handle);
}

void ResourceManager::Shutdown()
{
    //Clean up any image data
    for(auto& texture : textures){
        rendererRef->unloadTexture(texture);
    }

    //Clean up any model data
    for(auto& mesh : meshes){
        rendererRef->unloadMesh(mesh);
    }
}