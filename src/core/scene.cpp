#include "scene.h"

bool Scene::addSceneObject(Object* object){
    for(auto sceneObject : sceneObjects)
        if(sceneObject == object)
            return false;

    sceneObjects.push_back(object);
    return true;
}

bool Scene::addSceneCamera(Camera* camera){
    for(auto sceneCamera : sceneCameras)
        if(sceneCamera == camera)
            return false;

    sceneCameras.push_back(camera);
    return true;
}

void Scene::update(float deltaTime){
    //Update the objects in the scene
    for(auto sceneObject : sceneObjects)
        sceneObject->update(deltaTime);

    //Update the cameras in the scene
    for(auto sceneCamera : sceneCameras)
        sceneCamera->update(deltaTime);
}