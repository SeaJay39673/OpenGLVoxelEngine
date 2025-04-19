#pragma once
#ifndef CHUNK_H
#define CHUNK_H

#include "VAO.h"
#include "Shader.h"
#include "Mesh.h"

#include <vector>

#include <glm/glm.hpp>

using std::vector, glm::vec3, glm::vec2, glm::mat4;

class Chunk
{
private:
    const static int chunkSize = 16;
    int position[3];
    VoxelType voxels[chunkSize][chunkSize][chunkSize];
    unordered_map<VoxelType, Mesh *> meshMap;
    void loadChunk();

public:
    Chunk(int pos[3]);
    ~Chunk() {};
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
            {

                VoxelType type = GetRandomVoxel();
                voxels[i][j][k] = type;
                if (type != VoxelType::AIR && meshMap.find(type) == meshMap.end())
                {
                    meshMap[type] = new Mesh(type, position);
                }
            }
    loadChunk();
}

void Chunk::loadChunk()
{
    vector<Vertex> vertices;

    unsigned int offset = 0;
    for (int i = 0; i < chunkSize; i++)
    {
        for (int j = 0; j < chunkSize; j++)
        {
            for (int k = 0; k < chunkSize; k++)
            {
                if (voxels[i][j][k] != VoxelType::AIR)
                {
                    if (!i || !j || !k || i == chunkSize - 1 || j == chunkSize - 1 || k == chunkSize - 1)
                    {
                        meshMap[voxels[i][j][k]]->GenerateVoxel(i, j, k);
                        continue;
                    }
                    if (voxels[i - 1][j][k] == VoxelType::AIR ||
                        i + 1 < chunkSize && voxels[i + 1][j][k] == VoxelType::AIR ||
                        voxels[i][j - 1][k] == VoxelType::AIR ||
                        j + 1 < chunkSize && voxels[i][j + 1][k] == VoxelType::AIR ||
                        voxels[i][j][k - 1] == VoxelType::AIR ||
                        k + 1 < chunkSize && voxels[i][j][k + 1] == VoxelType::AIR)
                    {
                        meshMap[voxels[i][j][k]]->GenerateVoxel(i, j, k);
                    }
                }
            }
        }
    }

    for (auto &pair : meshMap)
    {
        pair.second->CreateMesh();
    }
}

void Chunk::Render(Shader &shader)
{

    for (auto &pair : meshMap)
    {
        pair.second->Render(shader);
    }
}

#endif