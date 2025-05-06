#ifndef PLAYER_H
#define PLAYER_H

#include <chrono>

#include "../Camera.h"
#include "../Physics/Gravity.h"
#include "../Chunk/ChunkManager.h"

using Engine::ChunkSpace::ChunkManager;

using namespace std::chrono;

namespace Engine::Entities
{
    /**
     * @brief A class for handling player input and movement.
     *
     */
    class Player
    {
    public:
        Player(Camera &cam) : velocity(0), camera(cam) {}
        void Update(float dt)
        {
            vec3 currentPos = camera.GetCameraPos();
            vec3 offset = ChunkManager::HandleCollisions(currentPos, GetMin(), GetMax());
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
        vec3 &GetPos() { return camera.GetCameraPos(); }
        void ProcessInput() { camera.ProcessInput(); }
        vec3 GetMin() { return camera.GetCameraPos() - vec3(.4f, 1.8, .4f); }
        vec3 GetMax() { return camera.GetCameraPos() + vec3(.4f, 0, .4f); }

    private:
        Camera camera;
        vec3 velocity;
    };

}

#endif