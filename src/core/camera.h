#pragma once

#include <glm/glm.hpp>
#include "object.h"

class Camera : public Object{
    bool isRendering;

    float frameOfView;
    float aspectRatio;
    float nearClippingDist;
    float farClippingDist;
    glm::vec3 offset;
    bool flipProjectionY;

    void* rendererData;

public:
    Camera(){}

    void setFoV(float);
    void setAspectRatio(float);
    void setNearClippingDistance(float);
    void setFarClippingDistance(float);
    void setCameraOffset(glm::vec3);
    void setYProjectionFlip(bool);
    void setIsRendering(bool);

    glm::mat4 getLookAtMatrix(glm::vec3, glm::vec3);

    glm::mat4 getPerspectiveMatrix();
};