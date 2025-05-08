#ifndef CHUNK_H
#define CHUNK_H

#include "../Mesh/Mesh.h"
#include "Voxel.h"
#include "../Utility/Utility.h"
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

    enum class ChunkOperations
    {
        BREAK,
        PLACE
    };

    enum class ChunkNeighbor
    {
        LEFT = 0,
        RIGHT,
        TOP,
        BOTTOM,
        FRONT,
        BACK
    };

    /**
     * @brief A class representing a chunk of voxels in a 3D space.
     *
     * @details This class handles the generation, rendering, and collision detection of voxels within a chunk.
     * It also manages neighboring chunks and their interactions.
     */
    class Chunk
    {
    public:
        Chunk(int pos[3]);
        ~Chunk();
        bool HasVoxels();
        void CreateMeshes();
        void DeleteMeshes();
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
        bool HandleRay(const Ray &ray, ChunkOperations operation);
        void RegenerateMeshes();
        void AddVoxel(int x, int y, int z, VoxelType type);

    private:
        int position[3];
        const int chunkSize;
        vector<VoxelType> voxels;
        unordered_set<VoxelType> voxelsHash;
        unordered_map<VoxelType, Mesh *> meshMap;
        Chunk *neighbors[6];
        bool checkNeighbor(int x, int y, int z);
        void updateVoxel(int x, int y, int z, VoxelType type);
    };

    //====| Constructors/Destructors |=====//

    /**
     * @brief Constructor for the Chunk class.
     * @details Initializes the chunk with a given position and generates voxels based on noise functions.
     * @note Call the constructor in either the main thread or a worker thread.
     *
     * @param pos The position of the chunk in 3D space.
     */
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

    /**
     * @brief Destructor for the Chunk class.
     *
     */
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

    //====| Public Functions |=====//

    /**
     * @brief Sets a neighbor chunk in the specified direction.
     *
     * @param neighbor The neighbor chunk to set.
     * @param direction The direction of the neighbor chunk.
     */
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

    /**
     * @brief Checks if the chunk has any voxels.
     *
     * @return true if the chunk has voxels.
     * @return false otherwise.
     */
    bool Chunk::HasVoxels()
    {
        for (const VoxelType &type : voxelsHash)
            return true;
        return false;
    }

    /**
     * @brief Creates meshes for each voxel type in the chunk.
     * @note Since this function calls OpenGL functions, it should be called in the main thread.
     *
     */
    void Chunk::CreateMeshes()
    {
        for (const VoxelType &type : voxelsHash)
        {
            meshMap[type] = new Mesh(type, position);
        }
    }

    /**
     * @brief Deletes all meshes in the chunk.ww
     *
     */
    void Chunk::DeleteMeshes()
    {
        for (auto pair : meshMap)
        {
            delete pair.second;
        }
        meshMap.clear();
    }

    /**
     * @brief Initializes the chunk by generating faces for each voxel.
     * @note This function can be called on either the main thread or a worker thread.
     *
     */
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

    /**
     * @brief Generates OpenGL buffers for each mesh in the chunk.
     * @note Since this function calls OpenGL functions, it should be called in the main thread.
     *
     */
    void Chunk::GenerateBuffers()
    {
        for (auto pair : meshMap)
        {
            pair.second->CreateMesh();
        }
    }

    /**
     * @brief Renders the chunk using the provided shader.
     * @note Since this function calls OpenGL functions, it should be called in the main thread.
     *
     * @param shader
     */
    void Chunk::Render(Shader &shader)
    {
        for (auto &pair : meshMap)
        {
            pair.second->Render(shader);
        }
    }

    /**
     * @brief Resolves collisions between the camera and the chunk's voxels.
     *
     * @param cameraMin The minimum coordinates of the camera's bounding box.
     * @param cameraMax The maximum coordinates of the camera's bounding box.
     * @return vec3 The correction vector to resolve the collisions.
     */
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

    /**
     * @brief Handles ray operations (BREAK or PLACE) on the chunk's voxels.
     *
     * @param ray The ray to handle.
     * @param operation The operation to perform (BREAK or PLACE).
     * @return true if the operation was successful.
     * @return false otherwise.
     */
    bool Chunk::HandleRay(const Engine::Utility::Ray &ray, ChunkOperations operation)
    {
        float closestT = std::numeric_limits<float>::infinity();
        glm::ivec3 hitVoxel{-1, -1, -1};
        std::optional<Engine::Utility::RaycastHit> bestHitInfo;
        bool didHit = false;

        // iterate all solid voxels…
        for (int x = 1; x <= chunkSize; ++x)
            for (int y = 1; y <= chunkSize; ++y)
                for (int z = 1; z <= chunkSize; ++z)
                {
                    if (GetVoxel(x, y, z) == VoxelType::AIR)
                        continue;

                    // world‐space AABB
                    vec3 vMin{position[0] + x - 1, position[1] + y - 1, position[2] + z - 1};
                    vec3 vMax = vMin + vec3(1.0f);

                    if (auto hitInfo = ray.Intersects(vMin, vMax))
                    {
                        float t = glm::dot(hitInfo->hitPosition - ray.GetOrigin(),
                                           ray.GetDirection());
                        if (t >= 0.0f && t < closestT)
                        {
                            closestT = t;
                            hitVoxel = glm::ivec3(x, y, z);
                            bestHitInfo = *hitInfo;
                            didHit = true;
                        }
                    }
                }

        if (!didHit)
            return false;

        switch (operation)
        {
        case ChunkOperations::BREAK:
            updateVoxel(hitVoxel.x, hitVoxel.y, hitVoxel.z, VoxelType::AIR);
            break;

        case ChunkOperations::PLACE:
        {
            // place in the adjacent voxel along the ray
            glm::ivec3 place = hitVoxel + glm::ivec3(bestHitInfo->hitNormal);
            updateVoxel(place.x, place.y, place.z, VoxelType::BRICK);
        }
        break;
        }

        RegenerateMeshes();
        return true;
    }

    /**
     * @brief Regenerates the meshes for the chunk.
     * @note This function should be called in the main thread.
     *
     */
    void Chunk::RegenerateMeshes()
    {
        for (auto pair : meshMap)
        {
            delete pair.second;
        }
        CreateMeshes();
        Initialize();
        GenerateBuffers();
    }

    /**
     * @brief Adds a voxel of the specified type at the given coordinates.
     *
     * @param x The x-coordinate of the voxel.
     * @param y The y-coordinate of the voxel.
     * @param z The z-coordinate of the voxel.
     * @param type The type of the voxel to add.
     */
    void Chunk::AddVoxel(int x, int y, int z, VoxelType type)
    {
        GetVoxel(x, y, z) = type;
        if (type != VoxelType::AIR && voxelsHash.find(type) == voxelsHash.end())
            voxelsHash.insert(type);
    }

    //====| Private Functions |=====//

    /**
     * @brief Checks the type of a voxel at the given coordinates.
     *
     * @param x The x-coordinate of the voxel.
     * @param y The y-coordinate of the voxel.
     * @param z The z-coordinate of the voxel.
     * @return true if the voxel is AIR or WATER.
     * @return false otherwise.
     */
    bool Chunk::checkNeighbor(int x, int y, int z)
    {
        VoxelType neighbor = GetVoxel(x, y, z);
        return neighbor == VoxelType::AIR || neighbor == VoxelType::WATER;
    }

    /**
     * @brief Updates the voxel at the given coordinates and updates neighboring chunks if necessary.
     *
     * @param x The x-coordinate of the voxel.
     * @param y The y-coordinate of the voxel.
     * @param z The z-coordinate of the voxel.
     * @param type The new type of the voxel.
     */
    void Chunk::updateVoxel(int x, int y, int z, VoxelType type)
    {
        AddVoxel(x, y, z, type);
        if (x == 1 && neighbors[(int)ChunkNeighbor::LEFT] != nullptr)
        {
            neighbors[(int)ChunkNeighbor::LEFT]->AddVoxel(chunkSize + 1, y, z, type);
            neighbors[(int)ChunkNeighbor::LEFT]->RegenerateMeshes();
        }
        if (x == chunkSize && neighbors[(int)ChunkNeighbor::RIGHT] != nullptr)
        {
            neighbors[(int)ChunkNeighbor::RIGHT]->AddVoxel(0, y, z, type);
            neighbors[(int)ChunkNeighbor::RIGHT]->RegenerateMeshes();
        }
        if (y == 1 && neighbors[(int)ChunkNeighbor::BOTTOM] != nullptr)
        {
            neighbors[(int)ChunkNeighbor::BOTTOM]->AddVoxel(x, chunkSize + 1, z, type);
            neighbors[(int)ChunkNeighbor::BOTTOM]->RegenerateMeshes();
        }
        if (y == chunkSize && neighbors[(int)ChunkNeighbor::TOP] != nullptr)
        {
            neighbors[(int)ChunkNeighbor::TOP]->AddVoxel(x, 0, z, type);
            neighbors[(int)ChunkNeighbor::TOP]->RegenerateMeshes();
        }
        if (z == 1 && neighbors[(int)ChunkNeighbor::BACK] != nullptr)
        {
            neighbors[(int)ChunkNeighbor::BACK]->AddVoxel(x, y, chunkSize + 1, type);
            neighbors[(int)ChunkNeighbor::BACK]->RegenerateMeshes();
        }
        if (z == chunkSize && neighbors[(int)ChunkNeighbor::FRONT] != nullptr)
        {
            neighbors[(int)ChunkNeighbor::FRONT]->AddVoxel(x, y, 0, type);
            neighbors[(int)ChunkNeighbor::FRONT]->RegenerateMeshes();
        }
    }
}

#endif