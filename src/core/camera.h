#pragma once

#include <glm/glm.hpp>
#include "object.h"
#include "structs.h"

class Camera : public Object{
    //Defines if camera is actively rendering each frame
    bool isRendering;

    float frameOfView;
    float aspectRatio;
    float nearClippingDist;
    float farClippingDist;
    //Positional offset of the camera from the object transform
    glm::vec3 offset;
    //Toggle to flip the Y value of the projection matrix
    bool flipProjectionY;

    //Texture to be rendered to
    Texture* renderTarget;
    
public:
    //Renderer specific data pointer
    void* rendererData;

    Camera();

    ///@brief Sets the frame of view of the camera
    ///@param fov The new frame of view
    void setFoV(float);

    /// @brief Sets the aspect ratio of the camera
    /// @param ratio The new aspect ratio
    void setAspectRatio(float);

    /// @brief Sets the near clipping plane distance
    /// @param nearClip The distance from the camera where the near clipping plane exists
    void setNearClippingDistance(float);

    /// @brief Sets the far clipping plane distance
    /// @param farClip The distance from the camera where the far clipping plane exists
    void setFarClippingDistance(float);

    /// @brief Sets a positional offset for the camera when rendering
    /// @param cameraOffset The offset of the camera
    void setCameraOffset(glm::vec3);

    /// @brief Sets a flag to determine if the Y value of the projection matrix should be inverted
    /// @param shouldFlip True; multiplies the projection matrix Y value by -1
    void setYProjectionFlip(bool);

    /// @brief Sets the camera's rendering state
    /// @param rendering True; the camera will render every frame
    void setIsRendering(bool);

    /// @brief Returns true if camera is rendering
    /// @return 
    bool getIsRendering();

    /// @brief Returns a look at matrix for the camera
    /// @param cameraOrigin The position of the camera. The offset is applied automatically
    /// @param lookPosition The position that the camera is looking at
    /// @return 
    glm::mat4 getLookAtMatrix(glm::vec3, glm::vec3);

    /// @brief Returns a view matrix for the camera
    /// @return 
    glm::mat4 getViewMatrix();

    /// @brief Returns a perspective matrix for the camera
    /// @return 
    glm::mat4 getPerspectiveMatrix();

    /// @brief Sets the texture the camera will render to
    /// @param texture The target texture
    void setRenderTexture(Texture*);
};