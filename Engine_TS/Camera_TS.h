#pragma once
#ifndef CAMERA_TS_H
#define CAMERA_TS_H

#include <glm/glm.hpp>
#include "Shader_TS.h"
#include "Lockable.h"
#include "../IO_TS/Mouse_TS.h"
#include "../IO_TS/Keyboard_TS.h"

#include <iostream>

using glm::vec3, glm::mat4, std::cout, std::endl;

class Camera_TS
{
private:
    Lockable<mat4> view;
    vec3 cameraPos, cameraUp, cameraFront, cameraRight, cameraDirection, up;
    float pitch, yaw, roll;
    float speed = 1.0f;
    Shader_TS *shader;

    void updateCamera();

public:
    Camera_TS(Shader_TS *shader, vec3 up = vec3(0, 1, 0)) : shader(shader), up(up), yaw(-90), pitch(0), roll(0)
    {
        view.ptr = new mat4(1.0f);
        // Only used in setup
        vec3 cameraTarget = vec3(0, 0, -1); // Where the camera is looking at

        // Will vary with movement
        cameraPos = vec3(0, 0, 0);
        cameraDirection = glm::normalize(cameraPos - cameraTarget);
        cameraFront = glm::normalize(cameraPos - cameraDirection);
        cameraRight = glm::normalize(glm::cross(up, cameraDirection));
        cameraUp = glm::normalize(glm::cross(cameraDirection, cameraRight));

        updateCamera();
    }

    void SetView()
    {
        lock_guard<mutex> lock(view.mut);
        shader->Use();
        shader->SetMat4f("view", *view.ptr);
    }

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

    vec3 GetCameraPos() const { return cameraPos; }
    vec3 GetCameraDirection() const { return cameraDirection; }
};

void Camera_TS::updateCamera()
{
    // Calculate new camera direction
    cameraDirection.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraDirection.y = sin(glm::radians(pitch));
    cameraDirection.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

    // Recalculate camera variables
    cameraRight = glm::normalize(glm::cross(up, cameraDirection));
    cameraUp = glm::normalize(glm::cross(cameraDirection, cameraRight));
    cameraFront = glm::normalize(cameraDirection);

    lock_guard<mutex> lock(view.mut);
    *view.ptr = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

    // Update shader information
    // if (shader != nullptr)
    // {
    //     shader->setVec3("viewPos", cameraPos);
    // }
}

void Camera_TS::ProcessInput()
{
    if (Keyboard_TS::HasKey(GLFW_KEY_LEFT_CONTROL))
        speed = 20.0f;
    else
        speed = 10.0f;

    if (Keyboard_TS::HasKey(GLFW_KEY_W))
        SlideFront(0.01f * speed);
    if (Keyboard_TS::HasKey(GLFW_KEY_S))
        SlideFront(-0.01f * speed);
    if (Keyboard_TS::HasKey(GLFW_KEY_A))
        SlideSide(0.01f * speed);
    if (Keyboard_TS::HasKey(GLFW_KEY_D))
        SlideSide(-0.01f * speed);
    if (Keyboard_TS::HasKey(GLFW_KEY_SPACE))
        SlideUp(0.01f * speed);
    if (Keyboard_TS::HasKey(GLFW_KEY_LEFT_SHIFT))
        SlideUp(-0.01f * speed);
    const double *del = Mouse_TS::GetMouseDeltas();
    if (del[0])
        Yaw(del[0] * 0.08);
    if (del[1])
        Pitch(-del[1] * 0.08);
    double temp[2] = {0, 0};
}

#endif