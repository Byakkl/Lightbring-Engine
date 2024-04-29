#include "camera_p.h"

#include <glm/gtc/matrix_transform.hpp>

Camera::Camera() : pImpl(std::make_unique<CameraImpl>())
{}

Camera::CameraImpl::CameraImpl():
    isRendering {true},
    frameOfView {45.0f},
    aspectRatio {1.0f},
    nearClippingDist {0.1f},
    farClippingDist { 100.0f},
    offset {glm::vec3(0.0f, 0.0f, 0.0f)},
    flipProjectionY {true}
{}


float Camera::getFoV(){
    return pImpl->getFoV();
}
float Camera::CameraImpl::getFoV(){
    return frameOfView;
}

void Camera::setFoV(float fov){
    pImpl->setFoV(fov);
}
void Camera::CameraImpl::setFoV(float fov){
    frameOfView = fov;
}

float Camera::getAspectRatio(){
    return pImpl->getAspectRatio();
}
float Camera::CameraImpl::getAspectRatio(){
    return aspectRatio;
}

void Camera::setAspectRatio(float ratio){
    pImpl->setAspectRatio(ratio);
}
void Camera::CameraImpl::setAspectRatio(float ratio){
    aspectRatio = ratio;
}

float Camera::getNearClippingDistance(){
    return pImpl->getNearClippingDistance();
}
float Camera::CameraImpl::getNearClippingDistance(){
    return nearClippingDist;
}

void Camera::setNearClippingDistance(float nearClip){
    pImpl->setNearClippingDistance(nearClip);
}
void Camera::CameraImpl::setNearClippingDistance(float nearClip){
    nearClippingDist = nearClip;
}

float Camera::getFarClippingDistance(){
    return pImpl->getFarClippingDistance();
}
float Camera::CameraImpl::getFarClippingDistance(){
    return farClippingDist;
}

void Camera::setFarClippingDistance(float farClip){
    pImpl->setFarClippingDistance(farClip);
}
void Camera::CameraImpl::setFarClippingDistance(float farClip){
    farClippingDist = farClip;
}

glm::vec3 Camera::getCameraOffset(){
    return pImpl->getCameraOffset();
}
glm::vec3 Camera::CameraImpl::getCameraOffset(){
    return offset;
}

void Camera::setCameraOffset(glm::vec3 cameraOffset){
    pImpl->setCameraOffset(cameraOffset);
}
void Camera::CameraImpl::setCameraOffset(glm::vec3 cameraOffset){
    offset = cameraOffset;
}

void Camera::setYProjectionFlip(bool shouldFlip){
    pImpl->setYProjectionFlip(shouldFlip);
}
void Camera::CameraImpl::setYProjectionFlip(bool shouldFlip){
    flipProjectionY = shouldFlip;
}

bool Camera::getIsRendering(){
    return pImpl->getIsRendering();
}
bool Camera::CameraImpl::getIsRendering(){
    return isRendering;
}

void Camera::setIsRendering(bool rendering){
    pImpl->setIsRendering(rendering);
}
void Camera::CameraImpl::setIsRendering(bool rendering){
    isRendering = rendering;
}

glm::mat4 Camera::getLookAtMatrix(glm::vec3 cameraOrigin, glm::vec3 lookPosition){
    return pImpl->getLookAtMatrix(cameraOrigin, lookPosition);
}
glm::mat4 Camera::CameraImpl::getLookAtMatrix(glm::vec3 cameraOrigin, glm::vec3 lookPosition){
    return glm::lookAt(cameraOrigin + offset, lookPosition, glm::vec3(0.0f, 0.0f, 1.0f));
}

glm::mat4 Camera::getViewMatrix(){
    return pImpl->getViewMatrix(transform);
}
glm::mat4 Camera::CameraImpl::getViewMatrix(const Transform* transform){
    glm::mat4 output = glm::mat4(1.0f);
    output = glm::translate(output, transform->position + offset);
    output *= transform->getRotationMatrix();

    output = glm::inverse(output);
    return output;
}

glm::mat4 Camera::getPerspectiveMatrix(){
    return pImpl->getPerspectiveMatrix();
}
glm::mat4 Camera::CameraImpl::getPerspectiveMatrix(){
    glm::mat4 output = glm::perspective(glm::radians(frameOfView), aspectRatio, nearClippingDist, farClippingDist);
    if(flipProjectionY)
        output[1][1] *= -1;
    return output;
}

void Camera::setRenderTexture(Texture* texture){
    pImpl->setRenderTexture(texture);
}
void Camera::CameraImpl::setRenderTexture(Texture* texture){
    renderTarget = texture;
}