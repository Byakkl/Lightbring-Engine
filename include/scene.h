#pragma once

#include <vector>
#include "comp_camera.h"

class Scene{
public:
    //std::vector<Object*> sceneObjects;
    std::vector<Comp_Camera*> sceneCameras;

    //bool addSceneObject(Object*);
    bool addSceneCamera(Comp_Camera*);
    void update(float);
private:
};