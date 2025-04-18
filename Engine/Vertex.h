#ifndef VERTEX_H
#define VERTEX_H

struct Vertex
{
    int position[3];
    int normal[3];
    int texCoord[2];

    Vertex() = default; // Default constructor
    Vertex(int pos[3], int norm[3], int tex[2])
    {
        memcpy(position, pos, sizeof(position));
        memcpy(normal, norm, sizeof(normal));
        memcpy(texCoord, tex, sizeof(texCoord));
    }
};

#endif