#pragma once

#include <vector>
#include "object.h"
#include "camera.h"

class Scene{
public:
    std::vector<Object*> sceneObjects;
    std::vector<Camera*> sceneCameras;

    bool addSceneObject(Object*);
    bool addSceneCamera(Camera*);
    void update();
private:
};