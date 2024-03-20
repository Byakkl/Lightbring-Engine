#pragma once

#include <vector>
#include "object.h"
#include "camera.h"

class Scene{
public:
    bool addSceneObject(Object*);
    bool addSceneCamera(Camera*);
    void update();
private:
    std::vector<Object*> sceneObjects;
    std::vector<Camera*> sceneCameras;
};