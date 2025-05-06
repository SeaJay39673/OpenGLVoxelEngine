#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include "Shader.h"
#include "../IO/IO.h"

using glm::vec3, glm::mat4;
using namespace IO;

namespace Engine
{
    /**
     * @brief Camera class for handling camera movement and view matrix.
     * @details This class handles camera movement and view matrix updates. It uses the GLM library for matrix and vector operations.
     *      Since this camera updates the OpenGL shader uniforms, the camera methods must be called on the main thread.
     */
    class Camera
    {
    public:
        Camera(Shader *shader, vec3 up = vec3(0, 1, 0));
        void SlideFront(float speed)
        {
            cameraPos += speed * vec3(cameraFront.x, 0, cameraFront.z);
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
        void SetCameraPos(vec3 pos)
        {
            cameraPos = pos;
            updateCamera();
        }
        vec3 const GetCameraPos() const { return cameraPos; }
        vec3 &GetCameraPos() { return cameraPos; }
        vec3 const GetCameraDirection() const { return cameraDirection; }
        mat4 const GetCameraView() const { return view; }
        void ProcessInput();

    private:
        mat4 view;
        vec3 cameraPos, cameraUp, cameraFront, cameraRight, cameraDirection, up;
        float pitch, yaw, roll;
        float speed = 10.0f;
        Shader *shader;

        void updateCamera();
    };

    //====| Constructors/Destructors |====//

    /**
     * @brief Construct a new Camera object
     *
     * @param shader The shader to use for the camera.
     * @param up Which direction is up. Default is (0, 1, 0).
     */
    Camera::Camera(Shader *shader, vec3 up = vec3(0, 1, 0)) : shader(shader), view(1.f), up(up), yaw(-90), pitch(0), roll(0)
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

    //====| Public Methods |====//

    /**
     * @brief Process input from the keyboard and mouse.
     *
     */
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

//====| Private Methods |====//

/**
 * @brief Update the camera view matrix and shader uniform.
 *
 */
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

#endif