#pragma once
#ifndef CHUNK_H
#define CHUNK_H

#include "VAO.h"
#include "Shader.h"
#include "Mesh.h"
#include "Voxel.h"
#include "Generator.h"

#include <vector>
#include <atomic>

#include <glm/glm.hpp>

using std::vector, glm::vec3, glm::vec2, glm::mat4, std::atomic;

using namespace Voxel;
using namespace Generator;

class Chunk
{
private:
    atomic<bool> loaded, generated;
    const static int chunkSize = 16;
    int position[3];
    VoxelType voxels[chunkSize][chunkSize][chunkSize];
    unordered_map<VoxelType, Mesh *> meshMap;
    void loadChunk();
    void generate();

public:
    Chunk(int pos[3]);
    ~Chunk() {};
    void Initialize();
    void Render(Shader &shader);
    static const int ChunkSize() { return chunkSize; };
    int *GetPosition() { return position; }
};

Chunk::Chunk(int pos[3])
{
    loaded.store(false);
    generated.store(false);
    memcpy(position, pos, sizeof(position));
    for (int i = 0; i < chunkSize; i++)
        for (int j = 0; j < chunkSize; j++)
            for (int k = 0; k < chunkSize; k++)
                voxels[i][j][k] = VoxelType::AIR;

    for (int i = 0; i < chunkSize; i++)     // X
        for (int j = 0; j < chunkSize; j++) // Z
        {
            double noiseValue = Noise2D_01((float)(position[0] + i), (float)(position[2] + j));
            int height = (int)(noiseValue * MaxHeight * chunkSize);
            for (int k = 0; k < chunkSize; k++) // Y
            {
                int y = (k + position[1]);
                if (y < height)
                {
                    VoxelType type = VoxelType::AIR;
                    noiseValue = Noise3D_01((float)position[0] + i, (float)position[1] + k, (float)position[2] + j);
                    if (noiseValue > .7)
                        type = VoxelType::BRICK;
                    else if (noiseValue > .6)
                        type = VoxelType::BRICK_RED;
                    else if (noiseValue > .58)
                        type = VoxelType::EYE;
                    else
                        type = VoxelType::WOOD;

                    voxels[i][k][j] = type;
                    if (meshMap.find(type) == meshMap.end())
                    {
                        meshMap[type] = new Mesh(type, position);
                    }
                }
            }
        }
    for (int i = 0; i < chunkSize; i++)
        for (int j = 0; j < chunkSize; j++)
            for (int k = 0; k < chunkSize; k++)
            {
                int y = (j + position[1]);
                if (y < 30 && voxels[i][j][k] == VoxelType::AIR)
                {
                    voxels[i][j][k] = VoxelType::WATER;
                    if (meshMap.find(VoxelType::WATER) == meshMap.end())
                        meshMap[VoxelType::WATER] = new Mesh(VoxelType::WATER, position);
                }
            }
}

void Chunk::Initialize()
{
    loadChunk();
    loaded.store(true);
}

void Chunk::loadChunk()
{
    for (int i = 0; i < chunkSize; i++) // x
    {
        for (int j = 0; j < chunkSize; j++) // z
        {
            for (int k = 0; k < chunkSize; k++)
            {
                if (voxels[i][j][k] != VoxelType::AIR)
                {
                    // Check each face of the voxel and only generate it if it's visible
                    if (i == 0 || voxels[i - 1][j][k] == VoxelType::AIR || voxels[i - 1][j][k] == VoxelType::WATER) // Left face
                    {
                        meshMap[voxels[i][j][k]]->GenerateFace(i, j, k, Face::LEFT);
                    }
                    if (i == chunkSize - 1 || voxels[i + 1][j][k] == VoxelType::AIR || voxels[i + 1][j][k] == VoxelType::WATER) // Right face
                    {
                        meshMap[voxels[i][j][k]]->GenerateFace(i, j, k, Face::RIGHT);
                    }
                    if (j == 0 || voxels[i][j - 1][k] == VoxelType::AIR || voxels[i][j - 1][k] == VoxelType::WATER) // Bottom face
                    {
                        meshMap[voxels[i][j][k]]->GenerateFace(i, j, k, Face::BOTTOM);
                    }
                    if (j == chunkSize - 1 || voxels[i][j + 1][k] == VoxelType::AIR || voxels[i][j + 1][k] == VoxelType::WATER) // Top face
                    {
                        meshMap[voxels[i][j][k]]->GenerateFace(i, j, k, Face::TOP);
                    }
                    if (k == 0 || voxels[i][j][k - 1] == VoxelType::AIR || voxels[i][j][k - 1] == VoxelType::WATER) // Back face
                    {
                        meshMap[voxels[i][j][k]]->GenerateFace(i, j, k, Face::BACK);
                    }
                    if (k == chunkSize - 1 || voxels[i][j][k + 1] == VoxelType::AIR || voxels[i][j][k + 1] == VoxelType::WATER) // Front face
                    {
                        meshMap[voxels[i][j][k]]->GenerateFace(i, j, k, Face::FRONT);
                    }
                }
            }
        }
    }
}

void Chunk::generate()
{
    for (auto pair : meshMap)
    {
        pair.second->CreateMesh();
    }
    generated.store(true);
}

void Chunk::Render(Shader &shader)
{
    if (!loaded.load())
        return;
    if (!generated.load())
        generate();
    for (auto &pair : meshMap)
    {
        pair.second->Render(shader);
    }
}

#endif