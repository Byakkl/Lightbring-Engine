#include "object.h"
#include "structs.h"

bool Object::addComponent(Component* component){
    for(auto component : components)
        if(component->type == component->type)
            return false;

    components.push_back(component);
    return true;
}

Component* Object::getComponent(const ComponentType componentType){
    for(auto component : components)
        if(component->type == componentType)
            return component;
        
    return nullptr;
}

void Object::cleanup(){
    components.clear();
}

void Object::update(){

}