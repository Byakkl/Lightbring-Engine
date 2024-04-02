#pragma once

#include <vector>
#include "structs.h"

class Object {
public:
    //Locally managed component present on all objects
    Transform* transform;
    
    Object();
    ~Object();
    bool addComponent(Component*);
    Component* getComponent(const ComponentType);
    virtual void cleanup();
    virtual void update(float);
private:
    std::vector<Component*> components;
};