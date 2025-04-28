#ifndef PLAYER_H
#define PLAYER_H

#include <chrono>

#include "../Camera.h"
#include "../Physics/Gravity.h"

using namespace std::chrono;

namespace Engine::Entities
{
    class Player
    {
    private:
        Camera camera;
        vec3 velocity;

    public:
        Player(Camera &cam) : velocity(0), camera(cam) {}
        void Update(vec3 offset, duration<float> dt)
        {
            vec3 newPos = camera.GetCameraPos() + offset;
            vec3 gravityOffset = Physics::Gravity::Step(velocity, dt);
            newPos += gravityOffset;
            if (offset.y != 0)
                velocity.y = 0;
            if (newPos.y < 0)
                newPos.y = 0;

            camera.SetCameraPos(newPos);
        }
        Camera &GetCamera() { return camera; }
        vec3 GetPos() { return camera.GetCameraPos(); }
        void ProcessInput() { camera.ProcessInput(); }
        vec3 GetMin() { return camera.GetCameraPos() - vec3(.4f, 1.8, .4f); }
        vec3 GetMax() { return camera.GetCameraPos() + vec3(.4f, 0, .4f); }
    };
}

#endif