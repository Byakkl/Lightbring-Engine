#include "object.h"
#include "structs.h"

Object::Object(){
    transform = new Transform();
}

Object::~Object(){
    cleanup();
}

bool Object::addComponent(Component* component){
    for(auto component : components)
        if(component->type == component->type)
            return false;

    components.push_back(component);
    return true;
}

Component* Object::getComponent(const ComponentType componentType){
    if(componentType == ComponentType::COMP_TRANSFORM)
        return transform;

    for(auto component : components)
        if(component->type == componentType)
            return component;
        
    return nullptr;
}

void Object::cleanup(){
    delete transform;
    components.clear();
}

void Object::update(){

}