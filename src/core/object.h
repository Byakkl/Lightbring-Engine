#pragma once

#include <vector>

enum ComponentType;
class Component;

class Object {
public:
    bool addComponent(Component*);
    Component* getComponent(const ComponentType);
    void cleanup();
    void update();
private:
    std::vector<Component*> components;
};