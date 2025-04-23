#ifndef CHUNK_H
#define CHUNK_H

#include "../Mesh/Mesh.h"
#include "Voxel.h"
#include "../Utility/Generator.h"
#include "Config.h"

#include <vector>
#include <atomic>

#include <glm/glm.hpp>

using std::vector, glm::vec3, glm::vec2, glm::mat4, std::atomic;
using namespace Engine::Utility;
using namespace Engine::MeshSpace;
using namespace Engine::Utility;
using namespace Engine::ChunkSpace::Voxel;

namespace Engine::ChunkSpace
{

    class Chunk
    {
    private:
        atomic<bool> loaded, generated;
        int position[3];
        const int chunkSize;
        vector<VoxelType> voxels;
        unordered_map<VoxelType, Mesh *> meshMap;
        void loadChunk();
        void generate();

    public:
        Chunk(int pos[3]);
        ~Chunk() {};
        void Initialize();
        void Render(Shader &shader);
        int *GetPosition() { return position; }
        VoxelType &GetVoxel(int x, int y, int z)
        {
            return voxels[x * chunkSize * chunkSize + y * chunkSize + z];
        }
    };

    Chunk::Chunk(int pos[3]) : chunkSize(Config::GetChunkSize())
    {
        voxels.resize(chunkSize * chunkSize * chunkSize, VoxelType::AIR);
        loaded.store(false);
        generated.store(false);
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
                        if (type != VoxelType::AIR && meshMap.find(type) == meshMap.end())
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
                    if (y < 30 && GetVoxel(i, j, k) == VoxelType::AIR)
                    {
                        GetVoxel(i, j, k) = VoxelType::WATER;
                    }
                }
        if (meshMap.find(VoxelType::WATER) == meshMap.end())
            meshMap[VoxelType::WATER] = new Mesh(VoxelType::WATER, position);
    }

    void Chunk::Initialize()
    {
        loadChunk();
        loaded.store(true);
    }

    void Chunk::loadChunk()
    {
        for (int x = 0; x < chunkSize; x++)
        {
            for (int z = 0; z < chunkSize; z++)
            {
                // Find the highest solid voxel in this (x, z) column
                for (int y = chunkSize - 1; y >= 0; y--)
                {
                    if (GetVoxel(x, y, z) != VoxelType::AIR)
                    {
                        VoxelType voxelType = GetVoxel(x, y, z);

                        // Generate faces exposed to AIR or WATER
                        if (x == 0 || GetVoxel(x - 1, y, z) == VoxelType::AIR || GetVoxel(x - 1, y, z) == VoxelType::WATER)
                            meshMap[voxelType]->GenerateFace(x, y, z, Face::LEFT);

                        if (x == chunkSize - 1 || GetVoxel(x + 1, y, z) == VoxelType::AIR || GetVoxel(x + 1, y, z) == VoxelType::WATER)
                            meshMap[voxelType]->GenerateFace(x, y, z, Face::RIGHT);

                        if (y == 0 || GetVoxel(x, y - 1, z) == VoxelType::AIR || GetVoxel(x, y - 1, z) == VoxelType::WATER)
                            meshMap[voxelType]->GenerateFace(x, y, z, Face::BOTTOM);

                        if (y == chunkSize - 1 || GetVoxel(x, y + 1, z) == VoxelType::AIR || GetVoxel(x, y + 1, z) == VoxelType::WATER)
                            meshMap[voxelType]->GenerateFace(x, y, z, Face::TOP);

                        if (z == 0 || GetVoxel(x, y, z - 1) == VoxelType::AIR || GetVoxel(x, y, z - 1) == VoxelType::WATER)
                            meshMap[voxelType]->GenerateFace(x, y, z, Face::BACK);

                        if (z == chunkSize - 1 || GetVoxel(x, y, z + 1) == VoxelType::AIR || GetVoxel(x, y, z + 1) == VoxelType::WATER)
                            meshMap[voxelType]->GenerateFace(x, y, z, Face::FRONT);

                        if (voxelType != VoxelType::WATER)
                            break; // Stop after finding the top voxel in this column
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
}

#endif