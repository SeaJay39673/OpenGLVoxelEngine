// Gravity.h
#ifndef GRAVITY_H
#define GRAVITY_H

#include <glm/glm.hpp>
#include <chrono>

using glm::vec3;

using namespace std::chrono;

namespace Engine::Physics
{
    /**
     * @brief A class for handling gravity and motion in a 3D space.
     *
     */
    class Gravity
    {
    public:
        /**
         * @brief Calculate the displacement of an object under gravity for a given time step.
         *
         * @param velocity
         * @param delta
         * @return vec3
         */
        static vec3 Step(vec3 &velocity, float delta)
        {
            vec3 displacement = velocity * delta + 0.5f * Acceleration * delta * delta;
            velocity += Acceleration * delta;
            return displacement;
        }

    private:
        static constexpr vec3 Acceleration{0.0f, -9.81f, 0.0f};
    };
}

#endif
