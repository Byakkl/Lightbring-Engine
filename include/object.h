#pragma once

#include <memory>
#include <vector>
#include "transform.h"

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
    class ObjectImpl;
    std::unique_ptr<ObjectImpl> pImpl;
};