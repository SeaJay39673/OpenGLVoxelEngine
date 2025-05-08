#ifndef RAY_H
#define RAY_H

#include <glm/glm.hpp>
#include <optional>

using glm::vec3;

namespace Engine::Utility
{
    struct RaycastHit
    {
        vec3 hitPosition; // Position of the hit voxel
        vec3 hitNormal;   // Normal of the hit face
    };

    class Ray
    {
    public:
        Ray(const vec3 &origin, const vec3 &direction, float length)
            : origin(origin), direction(glm::normalize(direction)), length(length) {}

        const vec3 &GetOrigin() const { return origin; }
        const vec3 &GetDirection() const { return direction; }
        float GetLength() const { return length; }

        // Check if the ray intersects a voxel's bounding box
        std::optional<RaycastHit> Intersects(const vec3 &min, const vec3 &max) const
        {
            vec3 invDir = 1.0f / direction;
            vec3 t0 = (min - origin) * invDir;
            vec3 t1 = (max - origin) * invDir;
            vec3 tMin = glm::min(t0, t1);
            vec3 tMax = glm::max(t0, t1);
            float tNear = glm::max(glm::max(tMin.x, tMin.y), tMin.z);
            float tFar = glm::min(glm::min(tMax.x, tMax.y), tMax.z);

            if (tNear <= tFar && tFar >= 0.0f && tNear <= length)
            {
                vec3 hitPosition = origin + direction * tNear;
                vec3 hitNormal(0.0f);
                if (tNear == tMin.x)
                    hitNormal.x = direction.x > 0.f ? -1.f : 1.f;
                else if (tNear == tMin.y)
                    hitNormal.y = direction.y > 0.f ? -1.f : 1.f;
                else if (tNear == tMin.z)
                    hitNormal.z = direction.z > 0.f ? -1.f : 1.f;

                return RaycastHit{hitPosition, hitNormal};
            }

            return std::nullopt;
        }

    private:
        vec3 origin;
        vec3 direction;
        float length;
    };
}

#endif