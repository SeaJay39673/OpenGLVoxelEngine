#pragma once
#ifndef CHUNK_TS_H
#define CHUNK_TS_H

#include "../Engine/VAO.h"
#include "Shader_TS.h"
#include "Mesh_TS.h"

#include <vector>

#include <glm/glm.hpp>

using std::vector, glm::vec3, glm::vec2, glm::mat4;

class Chunk_TS
{
private:
    const static int chunkSize = 16;
    int position[3];
    VoxelType voxels[chunkSize][chunkSize][chunkSize];
    unordered_map<VoxelType, Mesh_TS *> meshMap;
    mutex loadedMutex;
    bool loaded;

public:
    Chunk_TS(int pos[3]);
    ~Chunk_TS() {};
    void Render(Shader_TS &shader);
    static int ChunkSize() { return chunkSize; };
    int *Chunk_TS::GetPosition() { return position; }
    void LoadChunk();
    bool IsLoaded();
};

Chunk_TS::Chunk_TS(int pos[3]) : loaded(false)
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
                    meshMap[type] = new Mesh_TS(type, position);
                }
            }
}

bool Chunk_TS::IsLoaded()
{
    lock_guard<mutex> lock(loadedMutex);
    return loaded;
}

void Chunk_TS::LoadChunk()
{
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
        pair.second->Done();
    }

    lock_guard<mutex> lock(loadedMutex);
    loaded = true;
}

void Chunk_TS::Render(Shader_TS &shader)
{
    if (!IsLoaded())
        return;
    for (auto &pair : meshMap)
    {
        pair.second->Render(shader);
    }
}

#endif