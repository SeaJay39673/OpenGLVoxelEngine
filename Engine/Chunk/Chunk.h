#ifndef CHUNK_H
#define CHUNK_H

#include "../Mesh/Mesh.h"
#include "Voxel.h"
#include "../Utility/Generator.h"
#include "Config.h"

#include <vector>
#include <unordered_set>

#include <glm/glm.hpp>

using std::vector, glm::vec3, glm::vec2, glm::mat4, std::unordered_set;
using namespace Engine::Utility;
using namespace Engine::MeshSpace;
using namespace Engine::Utility;
using namespace Engine::ChunkSpace::Voxel;

namespace Engine::ChunkSpace
{

    class Chunk
    {
    private:
        int position[3];
        const int chunkSize;
        vector<VoxelType> voxels;
        unordered_set<VoxelType> voxelsHash;
        unordered_map<VoxelType, Mesh *> meshMap;
        bool checkNeighbor(int x, int y, int z)
        {
            VoxelType neighbor = GetVoxel(x, y, z);
            return neighbor == VoxelType::AIR || neighbor == VoxelType::WATER;
        }

    public:
        Chunk(int pos[3]);
        ~Chunk()
        {
            for (const auto &pair : meshMap)
                delete pair.second;
            meshMap.clear();
        };
        bool HasVoxels();
        void CreateMeshes();
        void Initialize();
        void GenerateBuffers();
        void Render(Shader &shader);
        int *GetPosition() { return position; }
        vec3 GetHighestMiddleVoxel()
        {
            int half = chunkSize / 2;
            for (int y = chunkSize - 1; y >= 0; y--)
                if (GetVoxel(half, y, half) != VoxelType::AIR)
                {
                    return vec3(half + position[0], y + position[1], half + position[2]);
                }
            return vec3(0, 0, 0);
        }
        VoxelType &GetVoxel(int x, int y, int z)
        {
            return voxels[x * chunkSize * chunkSize + y * chunkSize + z];
        }
    };

    Chunk::Chunk(int pos[3]) : chunkSize(Config::GetChunkSize())
    {
        voxels.resize(chunkSize * chunkSize * chunkSize, VoxelType::AIR);
        memcpy(position, pos, sizeof(position));

        for (int i = 0; i < chunkSize; i++)     // X
            for (int j = 0; j < chunkSize; j++) // Z
            {
                double noiseValue = Generator::Noise2D_01((float)(position[0] + i), (float)(position[2] + j));
                int height = (int)(noiseValue * Config::GetMaxHeight() * chunkSize);
                for (int k = 0; k < chunkSize; k++) // Y
                {
                    int y = (k + position[1]);
                    if (y < height)
                    {
                        VoxelType type = VoxelType::AIR;
                        noiseValue = Generator::Noise3D_01((float)position[0] + i, (float)position[1] + k, (float)position[2] + j);
                        if (noiseValue > .7)
                            type = VoxelType::BRICK;
                        else if (noiseValue > .6)
                            type = VoxelType::BRICK_RED;
                        else if (noiseValue > .58)
                            type = VoxelType::EYE;
                        else
                            type = VoxelType::WOOD;

                        GetVoxel(i, k, j) = type;
                        if (type != VoxelType::AIR && voxelsHash.find(type) == voxelsHash.end())
                        {
                            voxelsHash.insert(type);
                        }
                    }
                }
            }
        for (int i = 0; i < chunkSize; i++)
            for (int j = 0; j < chunkSize; j++)
                for (int k = 0; k < chunkSize; k++)
                {
                    int y = (j + position[1]);
                    if (y < 30 && GetVoxel(i, j, k) == VoxelType::AIR)
                    {
                        GetVoxel(i, j, k) = VoxelType::WATER;
                        if (voxelsHash.find(VoxelType::WATER) == voxelsHash.end())
                            voxelsHash.insert(VoxelType::WATER);
                    }
                    if (y < 2)
                    {
                        GetVoxel(i, j, k) = VoxelType::BLOCK;
                        if (voxelsHash.find(VoxelType::BLOCK) == voxelsHash.end())
                            voxelsHash.insert(VoxelType::BLOCK);
                    }
                }
    }

    bool Chunk::HasVoxels()
    {
        for (const VoxelType &type : voxelsHash)
            return true;
        return false;
    }

    void Chunk::CreateMeshes()
    {
        for (const VoxelType &type : voxelsHash)
        {
            meshMap[type] = new Mesh(type, position);
        }
    }

    void Chunk::Initialize()
    {
        for (int x = 0; x < chunkSize; x++)
        {
            for (int z = 0; z < chunkSize; z++)
            {
                // Find the highest solid voxel in this (x, z) column
                for (int y = chunkSize - 1; y >= 0; y--)
                {
                    VoxelType voxelType = GetVoxel(x, y, z);
                    if (voxelType != VoxelType::AIR)
                    {
                        if (x != 0 && checkNeighbor(x - 1, y, z))
                            meshMap[voxelType]->GenerateFace(x, y, z, Face::LEFT);

                        if (x != chunkSize - 1 && checkNeighbor(x + 1, y, z))
                            meshMap[voxelType]->GenerateFace(x, y, z, Face::RIGHT);

                        if (y != 0 && checkNeighbor(x, y - 1, z))
                            meshMap[voxelType]->GenerateFace(x, y, z, Face::BOTTOM);

                        if (y != chunkSize - 1 && checkNeighbor(x, y + 1, z))
                            meshMap[voxelType]->GenerateFace(x, y, z, Face::TOP);

                        if (z != 0 && checkNeighbor(x, y, z - 1))
                            meshMap[voxelType]->GenerateFace(x, y, z, Face::BACK);

                        if (z != chunkSize - 1 && checkNeighbor(x, y, z + 1))
                            meshMap[voxelType]->GenerateFace(x, y, z, Face::FRONT);

                        if (voxelType != VoxelType::WATER)
                            break;
                    }
                }
            }
        }
    }

    void Chunk::GenerateBuffers()
    {
        for (auto pair : meshMap)
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
}

#endif