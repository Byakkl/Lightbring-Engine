#pragma once

#include <memory>
#include <vector>
#include "object.h"
#include "transform.h"

class Object::ObjectImpl {
public:
    ObjectImpl();
    ~ObjectImpl();
    bool addComponent(Component*);
    Component* getComponent(const ComponentType);
    virtual void cleanup();
private:
    std::vector<Component*> components;
};