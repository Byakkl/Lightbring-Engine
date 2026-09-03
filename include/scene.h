#pragma once

#include <vector>
#include "comp_camera.h"

class Scene{
public:
    //std::vector<Object*> sceneObjects;
    std::vector<Camera*> sceneCameras;

    //bool addSceneObject(Object*);
    bool addSceneCamera(Camera*);
    void update(float);
private:
};