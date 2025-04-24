#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include "Shader.h"

using glm::vec3, glm::mat4;

namespace Engine
{
    class Camera
    {
    private:
        mat4 view;
        vec3 cameraPos, cameraUp, cameraFront, cameraRight, cameraDirection, up;
        float pitch, yaw, roll;
        float speed = 10.0f;
        Shader *shader;

        void updateCamera();

    public:
        Camera(Shader *shader, vec3 up = vec3(0, 1, 0)) : shader(shader), view(1.f), up(up), yaw(-90), pitch(0), roll(0)
        {
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

        void SetCameraPos(vec3 pos)
        {
            cameraPos = pos;
            updateCamera();
        }

        vec3 const GetCameraPos() const { return cameraPos; }
        vec3 const GetCameraDirection() const { return cameraDirection; }
        mat4 const GetCameraView() const { return view; }
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

        view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

        shader->SetMat4f("view", view);

        // Update shader information
        // if (shader != nullptr)
        // {
        //     shader->setVec3("viewPos", cameraPos);
        // }
    }

    void Camera::ProcessInput()
    {
        if (Keyboard::keys[GLFW_KEY_LEFT_CONTROL])
            speed = 20.0f;
        else
            speed = 10.0f;

        if (Keyboard::keys[GLFW_KEY_W])
            SlideFront(0.01f * speed);
        if (Keyboard::keys[GLFW_KEY_S])
            SlideFront(-0.01f * speed);
        if (Keyboard::keys[GLFW_KEY_A])
            SlideSide(0.01f * speed);
        if (Keyboard::keys[GLFW_KEY_D])
            SlideSide(-0.01f * speed);
        if (Keyboard::keys[GLFW_KEY_SPACE])
            SlideUp(0.01f * speed);
        if (Keyboard::keys[GLFW_KEY_LEFT_SHIFT])
            SlideUp(-0.01f * speed);
        if (Mouse::delx)
            Yaw((float)(Mouse::delx * 0.08));
        if (Mouse::dely)
            Pitch((float)(-Mouse::dely * 0.08));
    }
}

#endif