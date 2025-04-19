#ifndef FRUSTUM_H
#define FRUSTUM_H

#include <glm/glm.hpp>
#include <array>

using glm::mat4;
using glm::vec3;
using glm::vec4;

class Frustum
{
private:
    std::array<vec4, 6> planes; // Six planes of the frustum: left, right, bottom, top, near, far

public:
    void Update(const mat4 &projection, const mat4 &view)
    {
        mat4 clip = projection * view;

        // Extract the planes from the combined projection-view matrix
        planes[0] = vec4(clip[0][3] + clip[0][0], clip[1][3] + clip[1][0], clip[2][3] + clip[2][0], clip[3][3] + clip[3][0]); // Left
        planes[1] = vec4(clip[0][3] - clip[0][0], clip[1][3] - clip[1][0], clip[2][3] - clip[2][0], clip[3][3] - clip[3][0]); // Right
        planes[2] = vec4(clip[0][3] + clip[0][1], clip[1][3] + clip[1][1], clip[2][3] + clip[2][1], clip[3][3] + clip[3][1]); // Bottom
        planes[3] = vec4(clip[0][3] - clip[0][1], clip[1][3] - clip[1][1], clip[2][3] - clip[2][1], clip[3][3] - clip[3][1]); // Top
        planes[4] = vec4(clip[0][3] + clip[0][2], clip[1][3] + clip[1][2], clip[2][3] + clip[2][2], clip[3][3] + clip[3][2]); // Near
        planes[5] = vec4(clip[0][3] - clip[0][2], clip[1][3] - clip[1][2], clip[2][3] - clip[2][2], clip[3][3] - clip[3][2]); // Far

        // Normalize the planes
        for (auto &plane : planes)
        {
            float length = glm::length(vec3(plane));
            plane /= length;
        }
    }

    bool IsBoxInFrustum(const vec3 &min, const vec3 &max) const
    {
        for (const auto &plane : planes)
        {
            // Check if all corners of the box are outside the plane
            if (glm::dot(vec3(plane), vec3(min.x, min.y, min.z)) + plane.w > 0.0f)
                continue;
            if (glm::dot(vec3(plane), vec3(max.x, min.y, min.z)) + plane.w > 0.0f)
                continue;
            if (glm::dot(vec3(plane), vec3(min.x, max.y, min.z)) + plane.w > 0.0f)
                continue;
            if (glm::dot(vec3(plane), vec3(max.x, max.y, min.z)) + plane.w > 0.0f)
                continue;
            if (glm::dot(vec3(plane), vec3(min.x, min.y, max.z)) + plane.w > 0.0f)
                continue;
            if (glm::dot(vec3(plane), vec3(max.x, min.y, max.z)) + plane.w > 0.0f)
                continue;
            if (glm::dot(vec3(plane), vec3(min.x, max.y, max.z)) + plane.w > 0.0f)
                continue;
            if (glm::dot(vec3(plane), vec3(max.x, max.y, max.z)) + plane.w > 0.0f)
                continue;

            // If all corners are outside, the box is not in the frustum
            return false;
        }
        return true;
    }
};

#endif