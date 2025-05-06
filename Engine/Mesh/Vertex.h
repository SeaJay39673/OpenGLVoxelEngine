#ifndef VERTEX_H
#define VERTEX_H

namespace Engine::MeshSpace
{
    /**
     * @brief A struct for handling vertex data.
     *
     */
    struct Vertex
    {
        int position[3];
        int normal[3];
        float texCoord[2];

        Vertex() = default; // Default constructor
        Vertex(int pos[3], int norm[3], float tex[2])
        {
            memcpy(position, pos, sizeof(position));
            memcpy(normal, norm, sizeof(normal));
            memcpy(texCoord, tex, sizeof(texCoord));
        }
    };
};

#endif