#ifndef COLLISIONS_H
#define COLLISIONS_H

#include <glm/glm.hpp>
#include <algorithm>

using glm::vec3;

class Collisions
{
public:
    // Collisions.h
    static vec3 Collide(vec3 min1, vec3 max1,
                        vec3 min2, vec3 max2)
    {
        // compute overlap on each axis
        float ox = std::min(max1.x, max2.x) - std::max(min1.x, min2.x);
        float oy = std::min(max1.y, max2.y) - std::max(min1.y, min2.y);
        float oz = std::min(max1.z, max2.z) - std::max(min1.z, min2.z);
        // if any axis has no overlap, no collision
        if (ox <= 0 || oy <= 0 || oz <= 0)
            return vec3(0.0f);

        // find smallest overlap axis
        if (ox < oy && ox < oz)
        {
            // push out along X
            float sx = ((min1.x + max1.x) * 0.5f < (min2.x + max2.x) * 0.5f) ? -ox : +ox;
            return vec3(sx, 0, 0);
        }
        else if (oy < ox && oy < oz)
        {
            float sy = ((min1.y + max1.y) * 0.5f < (min2.y + max2.y) * 0.5f) ? -oy : +oy;
            return vec3(0, sy, 0);
        }
        else
        {
            float sz = ((min1.z + max1.z) * 0.5f < (min2.z + max2.z) * 0.5f) ? -oz : +oz;
            return vec3(0, 0, sz);
        }
    }
};

#endif