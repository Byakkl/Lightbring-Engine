#pragma once

#include <vector>

enum ComponentType;
class Component;

class Object {
public:
    //Locally managed component present on all objects
    Component* transform;
    
    Object();
    ~Object();
    bool addComponent(Component*);
    Component* getComponent(const ComponentType);
    virtual void cleanup();
    virtual void update();
private:
    std::vector<Component*> components;
};