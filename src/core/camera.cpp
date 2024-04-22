#include <glm/gtc/matrix_transform.hpp>

#include "camera.h"

Camera::Camera(){
    isRendering = true;
    frameOfView = 45.0f;
    aspectRatio = 1.0f;
    nearClippingDist = 0.1f;
    farClippingDist = 100.0f;
    offset = glm::vec3(0.0f, 0.0f, 0.0f);
    flipProjectionY = true;
}

void Camera::setFoV(float fov){
    frameOfView = fov;
}

void Camera::setAspectRatio(float ratio){
    aspectRatio = ratio;
}

void Camera::setNearClippingDistance(float nearClip){
    nearClippingDist = nearClip;
}

void Camera::setFarClippingDistance(float farClip){
    farClippingDist = farClip;
}

void Camera::setCameraOffset(glm::vec3 cameraOffset){
    offset = cameraOffset;
}

void Camera::setYProjectionFlip(bool shouldFlip){
    flipProjectionY = shouldFlip;
}

void Camera::setIsRendering(bool rendering){
    isRendering = rendering;
}

bool Camera::getIsRendering(){
    return isRendering;
}

float Camera::getFrameOfView(){
    return frameOfView;
}

float Camera::getNearClippingDistance(){
    return nearClippingDist;
}

float Camera::getFarClippingDistance(){
    return farClippingDist;
}

glm::mat4 Camera::getLookAtMatrix(glm::vec3 cameraOrigin, glm::vec3 lookPosition){
    return glm::lookAt(cameraOrigin + offset, lookPosition, glm::vec3(0.0f, 0.0f, 1.0f));
}

glm::mat4 Camera::getViewMatrix(){
    glm::mat4 output = glm::mat4(1.0f);
    output = glm::translate(output, transform->position + offset);
    output *= transform->getRotationMatrix();

    output = glm::inverse(output);
    return output;
}

glm::mat4 Camera::getPerspectiveMatrix(){
    glm::mat4 output = glm::perspective(glm::radians(frameOfView), aspectRatio, nearClippingDist, farClippingDist);
    if(flipProjectionY)
        output[1][1] *= -1;
    return output;
}

void Camera::setRenderTexture(Texture* texture){
    renderTarget = texture;
}