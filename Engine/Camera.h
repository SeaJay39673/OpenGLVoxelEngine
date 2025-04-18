#pragma once
#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include "Shader.h"

using glm::vec3, glm::mat4;

class Camera
{
private:
    mat4 *view;
    vec3 cameraPos, cameraUp, cameraFront, cameraRight, cameraDirection, up;
    float pitch, yaw, roll;
    Shader *shader;

    void updateCamera();

public:
    Camera(mat4 *view, vec3 up = vec3(0, 1, 0)) : view(view), up(up)
    {
        // Only used in setup
        glm::vec3 cameraTarget = glm::vec3(0, 0, 3);

        // Will vary with movement
        cameraPos = glm::vec3(0, 0, 0);
        cameraDirection = glm::normalize(cameraPos - cameraTarget);
        cameraFront = glm::normalize(cameraPos - cameraDirection);
        cameraRight = glm::normalize(glm::cross(up, cameraDirection));
        cameraUp = glm::normalize(glm::cross(cameraDirection, cameraRight));

        updateCamera();
    }

    void SetShader(Shader *shader) { this->shader = shader; }
    void SlideFront(float speed)
    {
        cameraPos += speed * cameraFront;
        updateCamera();
    }
    void SlideSide(float speed)
    {
        cameraPos += speed * cameraRight;
        updateCamera();
    }
    void SlideUp(float speed)
    {
        cameraPos += speed * up;
        updateCamera();
    }
    void Yaw(float angle)
    {
        yaw += angle;
        updateCamera();
    }
    void Pitch(float angle)
    {
        pitch += angle;
        if (pitch > 89.0f)
            pitch = 89.0f;
        if (pitch < -89.0f)
            pitch = -89.0f;
        updateCamera();
    }
    void ProcessInput();
};

void Camera::updateCamera()
{
    // Calculate new camera direction
    cameraDirection.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraDirection.y = sin(glm::radians(pitch));
    cameraDirection.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

    // Recalculate camera variables
    cameraRight = glm::normalize(glm::cross(up, cameraDirection));
    cameraUp = glm::normalize(glm::cross(cameraDirection, cameraRight));
    cameraFront = glm::normalize(cameraDirection);

    *view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

    // Update shader information
    // if (shader != nullptr)
    // {
    //     shader->setVec3("viewPos", cameraPos);
    // }
}

void Camera::ProcessInput()
{
    if (Keyboard::keys[GLFW_KEY_W])
        SlideFront(0.01f);
    if (Keyboard::keys[GLFW_KEY_S])
        SlideFront(-0.01f);
    if (Keyboard::keys[GLFW_KEY_A])
        SlideSide(0.01f);
    if (Keyboard::keys[GLFW_KEY_D])
        SlideSide(-0.01f);
    if (Keyboard::keys[GLFW_KEY_SPACE])
        SlideUp(0.01f);
    if (Keyboard::keys[GLFW_KEY_LEFT_SHIFT])
        SlideUp(-0.01f);
    if (Mouse::delx)
        Yaw(-Mouse::delx * 0.08);
    if (Mouse::dely)
        Pitch(-Mouse::dely * 0.08);
}

#endif