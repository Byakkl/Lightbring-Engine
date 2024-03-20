#include "camera.h"

Camera::Camera(){
    isRendering = true;
    frameOfView = 45.0f;
    nearClippingDist = 0.1f;
    farClippingDist = 10.0f;
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

void Camera::setYProjectionFlip(bool shouldFlip){
    flipProjectionY = shouldFlip;
}

void Camera::setIsRendering(bool rendering){
    isRendering = rendering;
}

glm::mat4 Camera::getLookAtMatrix(glm::vec3 cameraOrigin, glm::vec3 lookPosition){
    return glm::mat4(1.0f);
}

glm::mat4 Camera::getPerspectiveMatrix(){
    glm::mat4 output = glm::mat4(1.0f);
    if(flipProjectionY)
        output[1][1] *= -1;
    return output;
}