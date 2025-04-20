#pragma once
#ifndef CHUNK_H
#define CHUNK_H

#include "VAO.h"
#include "Shader.h"
#include "Mesh.h"
#include "Voxel.h"

#include <vector>

#include <glm/glm.hpp>

using std::vector, glm::vec3, glm::vec2, glm::mat4;

using namespace Voxel;

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
                // VoxelType type = VoxelType::BRICK;
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
                    // Check each face of the voxel and only generate it if it's visible
                    if (i == 0 || voxels[i - 1][j][k] == VoxelType::AIR) // Left face
                    {
                        meshMap[voxels[i][j][k]]->GenerateFace(i, j, k, Face::LEFT);
                    }
                    if (i == chunkSize - 1 || voxels[i + 1][j][k] == VoxelType::AIR) // Right face
                    {
                        meshMap[voxels[i][j][k]]->GenerateFace(i, j, k, Face::RIGHT);
                    }
                    if (j == 0 || voxels[i][j - 1][k] == VoxelType::AIR) // Bottom face
                    {
                        meshMap[voxels[i][j][k]]->GenerateFace(i, j, k, Face::BOTTOM);
                    }
                    if (j == chunkSize - 1 || voxels[i][j + 1][k] == VoxelType::AIR) // Top face
                    {
                        meshMap[voxels[i][j][k]]->GenerateFace(i, j, k, Face::TOP);
                    }
                    if (k == 0 || voxels[i][j][k - 1] == VoxelType::AIR) // Back face
                    {
                        meshMap[voxels[i][j][k]]->GenerateFace(i, j, k, Face::BACK);
                    }
                    if (k == chunkSize - 1 || voxels[i][j][k + 1] == VoxelType::AIR) // Front face
                    {
                        meshMap[voxels[i][j][k]]->GenerateFace(i, j, k, Face::FRONT);
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