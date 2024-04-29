#include "object_p.h"

Object::Object() : pImpl(std::make_unique<ObjectImpl>()) {
    transform = new Transform();
}

Object::~Object(){}

Object::ObjectImpl::ObjectImpl(){
}

Object::ObjectImpl::~ObjectImpl(){
    cleanup();
}

bool Object::addComponent(Component* _component){
    return pImpl->addComponent(_component);
}
bool Object::ObjectImpl::addComponent(Component* _component){
    for(auto component : components)
        if(component->type == _component->type)
            return false;

    components.push_back(_component);
    return true;
}

Component* Object::getComponent(const ComponentType componentType){
    if(componentType == ComponentType::COMP_TRANSFORM)
        return transform;

    return pImpl->getComponent(componentType);
}
Component* Object::ObjectImpl::getComponent(const ComponentType componentType){
    for(auto component : components)
        if(component->type == componentType)
            return component;
        
    return nullptr;
}

void Object::cleanup(){
    delete transform;
    pImpl->cleanup();
}
void Object::ObjectImpl::cleanup(){
    components.clear();
}

void Object::update(float deltaTime){
}