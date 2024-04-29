#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include "component.h"

class Transform : public Component{
public:
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;

    glm::quat quatRot;

    Transform();

    void setPosition(glm::vec3);
    void setRotation(glm::vec3);
    void setScale(glm::vec3);
    glm::mat4 getRotationMatrix() const;
    glm::mat4 getTransformMatrix() const;
};