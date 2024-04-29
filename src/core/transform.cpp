#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/quaternion.hpp"
#include "transform.h"

Transform::Transform(){
    type = ComponentType::COMP_TRANSFORM;

    position = glm::vec3(0.0f);
    rotation = glm::vec3(0.0f);
    scale = glm::vec3(1.0f);

    quatRot = glm::quat(glm::vec3(0,0,0));

    //Set initial flag to true to ensure it gets passed in initially
    isDirty = true;
}

    void Transform::setPosition(glm::vec3 newPosition){
        position = newPosition;
        //position.y *= -1;
        isDirty = true;
    }

    void Transform::setRotation(glm::vec3 newRotation){
        //Deg to Rad
        rotation = glm::radians(newRotation);
        quatRot = glm::quat(rotation);
        isDirty = true;
    }

    void Transform::setScale(glm::vec3 newScale){
        scale = newScale;
        isDirty = true;
    }

    glm::mat4 Transform::getRotationMatrix() const{
        return glm::toMat4(quatRot);
    }

    glm::mat4 Transform::getTransformMatrix() const{
        glm::mat4 transformMatrix = glm::translate(glm::mat4(1.0f), position);
        transformMatrix *= getRotationMatrix();
        transformMatrix = glm::scale(transformMatrix, scale);

        return transformMatrix; 
    }