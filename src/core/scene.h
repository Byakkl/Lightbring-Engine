#pragma once

#include <vector>
#include "object.h"

class Scene{
public:
    bool addSceneObject(Object*);
    void update();
private:
    std::vector<Object*> sceneObjects;
};