// Gravity.h
#ifndef GRAVITY_H
#define GRAVITY_H

#include <glm/glm.hpp>
#include <chrono>

using glm::vec3;

using namespace std::chrono;

namespace Engine::Physics
{
    class Gravity
    {
    public:
        static constexpr vec3 Acceleration{0.0f, -9.81f, 0.0f};

        static vec3 Step(vec3 &velocity, duration<float> dt)
        {
            float delta = dt.count();
            vec3 displacement = velocity * delta + 0.5f * Acceleration * delta * delta;
            velocity += Acceleration * delta;
            return displacement;
        }
    };
}

#endif
