#ifndef PLAYER_H
#define PLAYER_H

#include "../Camera.h"
#include "../Physics/Gravity.h"
#include "../Chunk/ChunkManager.h"

using Engine::ChunkSpace::ChunkManager;

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
            vec3 newPos = camera.GetCameraPos();
            vec3 offset(0);
            vec3 gravityOffset(0);

            if (collisionsEnabled)
            {
                offset = ChunkManager::HandleCollisions(newPos, GetMin(), GetMax());
                newPos += offset;
            }

            if (gravityEnabled)
            {
                vec3 gravityOffset = Physics::Gravity::Step(velocity, dt);
                newPos += gravityOffset;
            }

            if (offset.y != 0)
                velocity.y = 0;
            if (newPos.y < 0)
                newPos.y = 0;
            camera.SetCameraPos(newPos);
        }
        Camera &GetCamera() { return camera; }
        vec3 &GetPos() { return camera.GetCameraPos(); }
        void ProcessInput()
        {
            if (Keyboard::keys[GLFW_KEY_G])
            {
                if (!gravityKeyPressed)
                {
                    gravityKeyPressed = true;
                    toggleGravity();
                }
            }
            else
                gravityKeyPressed = false;
            if (Keyboard::keys[GLFW_KEY_C])
            {
                if (!collisionKeyPressed)
                {
                    collisionKeyPressed = true;
                    toggleCollisions();
                }
            }
            else
                collisionKeyPressed = false;
            camera.ProcessInput();
        }
        vec3 GetMin() { return camera.GetCameraPos() - vec3(.4f, 1.8, .4f); }
        vec3 GetMax() { return camera.GetCameraPos() + vec3(.4f, 0, .4f); }

    private:
        void toggleGravity() { gravityEnabled = !gravityEnabled; }
        void toggleCollisions() { collisionsEnabled = !collisionsEnabled; }

        bool collisionKeyPressed = false;
        bool collisionsEnabled = true;

        bool gravityKeyPressed = false;
        bool gravityEnabled = true;
        Camera camera;
        vec3 velocity;
    };

}

#endif