#ifndef CHUNK_H
#define CHUNK_H

#include "../Mesh/Mesh.h"
#include "Voxel.h"
#include "../Utility/Generator.h"
#include "Config.h"
#include "../Physics/Collisions.h"

#include <vector>
#include <unordered_set>

#include <glm/glm.hpp>

using std::vector, glm::vec3, glm::vec2, glm::mat4, std::unordered_set;
using namespace Engine::Utility;
using namespace Engine::MeshSpace;
using namespace Engine::Utility;
using namespace Engine::ChunkSpace::Voxel;
using namespace Engine::Physics;

namespace Engine::ChunkSpace
{

    enum class ChunkNeighbor
    {
        LEFT = 0,
        RIGHT,
        TOP,
        BOTTOM,
        FRONT,
        BACK
    };

    class Chunk
    {
    private:
        int position[3];
        const int chunkSize;
        vector<VoxelType> voxels;
        unordered_set<VoxelType> voxelsHash;
        unordered_map<VoxelType, Mesh *> meshMap;
        Chunk *neighbors[6];
        bool checkNeighbor(int x, int y, int z);

    public:
        Chunk(int pos[3]);
        ~Chunk();
        bool HasVoxels();
        void DeleteMeshes();
        void CreateMeshes();
        void Initialize();
        void GenerateBuffers();
        void Render(Shader &shader);
        int *GetPosition() { return position; }
        void SetNeighbor(Chunk *neighbor, ChunkNeighbor direction);
        bool HasNeighbor(ChunkNeighbor direction)
        {
            return neighbors[(int)direction] != nullptr;
        }
        vec3 GetHighestMiddleVoxel()
        {
            int half = chunkSize / 2;
            for (int y = chunkSize - 1; y >= 0; y--)
                if (GetVoxel(half, y, half) != VoxelType::AIR)
                {
                    return vec3(half + position[0], y + position[1], half + position[2]);
                }
            return vec3(half + position[0], position[1], half + position[2]);
        }
        VoxelType &GetVoxel(int x, int y, int z)
        {
            return voxels[x * (chunkSize + 2) * (chunkSize + 2) + y * (chunkSize + 2) + z];
        }
        vec3 ResolveCollisions(const vec3 &cameraMin, const vec3 &cameraMax);
    };

    Chunk::Chunk(int pos[3]) : chunkSize(Config::GetChunkSize())
    {
        for (int i = 0; i < 6; i++)
        {
            neighbors[i] = nullptr;
        }
        int extendedSize = chunkSize + 2;
        voxels.resize((extendedSize) * (extendedSize) * (extendedSize), VoxelType::AIR);
        memcpy(position, pos, sizeof(position));

        for (int i = 0; i < extendedSize; i++)     // X
            for (int j = 0; j < extendedSize; j++) // Z
            {
                double noiseValue = Generator::Noise2D_01((float)(position[0] + i - 1), (float)(position[2] + j - 1));
                int height = (int)(noiseValue * Config::GetMaxHeight() * chunkSize);
                for (int k = 0; k < extendedSize; k++) // Y
                {
                    int y = (k + position[1] - 1);
                    if (y < height)
                    {
                        VoxelType type = VoxelType::AIR;
                        noiseValue = Generator::Noise3D_01((float)position[0] + i - 1, (float)position[1] + k - 1, (float)position[2] + j - 1);
                        if (noiseValue > .5)
                            type = VoxelType::BRICK;
                        else
                            type = VoxelType::BRICK_RED;

                        GetVoxel(i, k, j) = type;
                        if (type != VoxelType::AIR && voxelsHash.find(type) == voxelsHash.end())
                        {
                            voxelsHash.insert(type);
                        }
                    }
                }
            }
        for (int i = 1; i < chunkSize + 1; i++)
            for (int j = 1; j < chunkSize + 1; j++)
                for (int k = 1; k < chunkSize + 1; k++)
                {
                    int y = (j + position[1] - 1);
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

    Chunk::~Chunk()
    {

        for (const auto &pair : meshMap)
            delete pair.second;
        for (int i = 0; i < 6; i++)
        {
            if (neighbors[i] == nullptr)
                continue;
            switch ((ChunkNeighbor)i)
            {
            case ChunkNeighbor::LEFT:
                neighbors[i]->SetNeighbor(nullptr, ChunkNeighbor::RIGHT);
                break;
            case ChunkNeighbor::RIGHT:
                neighbors[i]->SetNeighbor(nullptr, ChunkNeighbor::LEFT);
                break;
            case ChunkNeighbor::TOP:
                neighbors[i]->SetNeighbor(nullptr, ChunkNeighbor::BOTTOM);
                break;
            case ChunkNeighbor::BOTTOM:
                neighbors[i]->SetNeighbor(nullptr, ChunkNeighbor::TOP);
                break;
            case ChunkNeighbor::FRONT:
                neighbors[i]->SetNeighbor(nullptr, ChunkNeighbor::BACK);
                break;
            case ChunkNeighbor::BACK:
                neighbors[i]->SetNeighbor(nullptr, ChunkNeighbor::FRONT);
                break;
            }
        }
        meshMap.clear();
    }

    bool Chunk::checkNeighbor(int x, int y, int z)
    {
        VoxelType neighbor = GetVoxel(x, y, z);
        return neighbor == VoxelType::AIR || neighbor == VoxelType::WATER;
    }

    void Chunk::SetNeighbor(Chunk *neighbor, ChunkNeighbor direction)
    {
        neighbors[(int)direction] = neighbor;

        ChunkNeighbor opposite;
        switch (direction)
        {
        case ChunkNeighbor::LEFT:
            opposite = ChunkNeighbor::RIGHT;
            break;
        case ChunkNeighbor::RIGHT:
            opposite = ChunkNeighbor::LEFT;
            break;
        case ChunkNeighbor::TOP:
            opposite = ChunkNeighbor::BOTTOM;
            break;
        case ChunkNeighbor::BOTTOM:
            opposite = ChunkNeighbor::TOP;
            break;
        case ChunkNeighbor::FRONT:
            opposite = ChunkNeighbor::BACK;
            break;
        case ChunkNeighbor::BACK:
            opposite = ChunkNeighbor::FRONT;
            break;
        }

        if (neighbor != nullptr && !neighbor->HasNeighbor(opposite))
        {
            neighbor->SetNeighbor(this, opposite);
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
        for (int x = 1; x < chunkSize + 1; x++)
        {
            for (int z = 1; z < chunkSize + 1; z++)
            {
                // Find the highest solid voxel in this (x, z) column
                for (int y = chunkSize; y >= 1; y--)
                {
                    VoxelType voxelType = GetVoxel(x, y, z);
                    if (voxelType != VoxelType::AIR)
                    {
                        if (checkNeighbor(x - 1, y, z))
                            meshMap[voxelType]->GenerateFace(x - 1, y - 1, z - 1, Face::LEFT);

                        if (checkNeighbor(x + 1, y, z))
                            meshMap[voxelType]->GenerateFace(x - 1, y - 1, z - 1, Face::RIGHT);

                        if (checkNeighbor(x, y - 1, z))
                            meshMap[voxelType]->GenerateFace(x - 1, y - 1, z - 1, Face::BOTTOM);

                        if (checkNeighbor(x, y + 1, z))
                            meshMap[voxelType]->GenerateFace(x - 1, y - 1, z - 1, Face::TOP);

                        if (checkNeighbor(x, y, z - 1))
                            meshMap[voxelType]->GenerateFace(x - 1, y - 1, z - 1, Face::BACK);

                        if (checkNeighbor(x, y, z + 1))
                            meshMap[voxelType]->GenerateFace(x - 1, y - 1, z - 1, Face::FRONT);

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

    vec3 Chunk::ResolveCollisions(const vec3 &cameraMin, const vec3 &cameraMax)
    {
        vec3 correction(0.0f);
        for (int x = 0; x <= chunkSize + 1; ++x)
        {
            for (int y = 0; y <= chunkSize + 1; ++y)
            {
                for (int z = 0; z <= chunkSize + 1; ++z)
                {
                    VoxelType t = GetVoxel(x, y, z);
                    if (t == VoxelType::AIR || t == VoxelType::WATER)
                        continue;
                    vec3 vMin{position[0] + (x - 1),
                              position[1] + (y - 1),
                              position[2] + (z - 1)};
                    vec3 vMax = vMin + vec3(1.0f);
                    vec3 cmn = cameraMin + correction;
                    vec3 cmx = cameraMax + correction;
                    vec3 mtv = Collisions::Collide(cmn, cmx, vMin, vMax);
                    correction += mtv;
                }
            }
        }
        return correction;
    }
}

#endif