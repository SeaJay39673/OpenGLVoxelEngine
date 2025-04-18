#pragma once
#ifndef CHUNK_H
#define CHUNK_H

#include "VAO.h"
#include "Shader.h"

#include <vector>

#include <glm/glm.hpp>

using std::vector, glm::vec3, glm::vec2, glm::mat4;

enum class VoxelType
{
    AIR,
    DIRT,
};

class Chunk
{
private:
    const static int chunkSize = 16;
    VAO vao;
    BO *vbo, *ebo;

    int position[3];
    VoxelType voxels[chunkSize][chunkSize][chunkSize];
    void loadChunk();
    void generateVoxel(vector<int> &vertices, vector<unsigned int> &indices, int i, int j, int k, unsigned int &offset);

public:
    Chunk(int pos[3]);
    ~Chunk();
    void Render(Shader &shader);
    static int ChunkSize() { return chunkSize; };
    int *Chunk::GetPosition() { return position; }
};

Chunk::Chunk(int pos[3])
{
    memcpy(position, pos, sizeof(position));
    for (int i = 0; i < chunkSize; i++)
        for (int j = 0; j < chunkSize; j++)
            for (int k = 0; k < chunkSize; k++)
                voxels[i][j][k] = rand() % 100 < 98 ? VoxelType::DIRT : VoxelType::AIR;
    loadChunk();
}

Chunk::~Chunk()
{
    delete vbo;
    delete ebo;
}

void Chunk::generateVoxel(vector<int> &vertices, vector<unsigned int> &indices, int i, int j, int k, unsigned int &offset)
{
    vertices.insert(
        vertices.end(),
        {
            i + position[0], j + position[1], k + position[2],             // bottom left
            i + position[0], j + 1 + position[1], k + position[2],         // top left
            i + 1 + position[0], j + 1 + position[1], k + position[2],     // top right
            i + 1 + position[0], j + position[1], k + position[2],         // bottom right
            i + position[0], j + position[1], k + 1 + position[2],         // Near Bottom Left
            i + position[0], j + 1 + position[1], k + 1 + position[2],     // Near Top Left
            i + 1 + position[0], j + 1 + position[1], k + 1 + position[2], // Near Top Right
            i + 1 + position[0], j + position[1], k + 1 + position[2],     // Near Bottom Right
        });

    // Front face (CCW)
    indices.insert(indices.end(), {offset + 0, offset + 1, offset + 3,
                                   offset + 1, offset + 2, offset + 3});

    // Back face (CCW)
    indices.insert(indices.end(), {offset + 4, offset + 7, offset + 5,
                                   offset + 5, offset + 7, offset + 6});

    // Left face (CCW)
    indices.insert(indices.end(), {offset + 0, offset + 4, offset + 1,
                                   offset + 1, offset + 4, offset + 5});

    // Right face (CCW)
    indices.insert(indices.end(), {offset + 3, offset + 2, offset + 7,
                                   offset + 2, offset + 6, offset + 7});

    // Top face (CCW)
    indices.insert(indices.end(), {offset + 1, offset + 5, offset + 2,
                                   offset + 2, offset + 5, offset + 6});

    // Bottom face (CCW)
    indices.insert(indices.end(), {offset + 0, offset + 3, offset + 4,
                                   offset + 3, offset + 7, offset + 4});

    offset += 8; // Each cube adds 8 vertices
}

void Chunk::loadChunk()
{
    vector<int> vertices;
    vector<unsigned int> indices;

    unsigned int offset = 0;
    for (int i = 0; i < chunkSize; i++)
    {
        for (int j = 0; j < chunkSize; j++)
        {
            for (int k = 0; k < chunkSize; k++)
            {
                if (voxels[i][j][k] == VoxelType::DIRT)
                {
                    if (!i || !j || !k || i == chunkSize - 1 || j == chunkSize - 1 || k == chunkSize - 1)
                    {
                        generateVoxel(vertices, indices, i, j, k, offset);
                        continue;
                    }
                    if (voxels[i - 1][j][k] == VoxelType::AIR ||
                        i + 1 < chunkSize && voxels[i + 1][j][k] == VoxelType::AIR ||
                        voxels[i][j - 1][k] == VoxelType::AIR ||
                        j + 1 < chunkSize && voxels[i][j + 1][k] == VoxelType::AIR ||
                        voxels[i][j][k - 1] == VoxelType::AIR ||
                        k + 1 < chunkSize && voxels[i][j][k + 1] == VoxelType::AIR)
                    {
                        generateVoxel(vertices, indices, i, j, k, offset);
                    }
                }
            }
        }
    }

    vao.Bind();
    vbo = new BO(vertices);
    ebo = new BO(indices);
    vao.LinkBO(*vbo);
    vbo->Unbind();
    vao.Unbind();
    ebo->Unbind();
}

void Chunk::Render(Shader &shader)
{
    shader.Use();
    vao.Bind();
    glDrawElements(GL_TRIANGLES, ebo->GetCount(), GL_UNSIGNED_INT, 0);
    vao.Unbind();
}

#endif