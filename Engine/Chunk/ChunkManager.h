#ifndef CHUNKMANAGER_H
#define CHUNKMANAGER_H

#include "Chunk.h"
#include "Config.h"
#include "../Utility/Utility.h"
#include "../Texture.h"
#include "../Concurrent/ContainerThreadExecutor.h"
#include <mutex>
#include <semaphore>
#include <unordered_set>
#include <queue>
#include <chrono>

using glm::vec4, std::mutex, std::lock_guard, std::counting_semaphore, std::queue, std::priority_queue, std::pair;
using namespace Engine::Concurrent;
using namespace Engine::Utility;
using namespace std::chrono;

namespace std
{
    template <>
    struct hash<glm::ivec3>
    {
        size_t operator()(const glm::ivec3 &v) const
        {
            return ((hash<int>()((int)v.x) ^ (hash<int>()((int)v.y) << 1)) >> 1) ^ (hash<int>()((int)v.z) << 1);
        }
    };
}

namespace Engine::ChunkSpace
{

    struct ComparePair
    {
        bool operator()(const std::pair<float, glm::ivec3> &a, const std::pair<float, glm::ivec3> &b) const
        {
            return a.first > b.first; // Min-heap behavior: smaller float has higher priority
        }
    };

    /**
     * @brief ChunkManager class for managing chunks in a 3D space.
     *
     */
    class ChunkManager
    {
    public:
        static void InitChunkManager(vec3 &playerPos, int renderDistance = 8)
        {
            glDisable(GL_CULL_FACE);
            Texture::InitializeTextures();
            Config::SetRenderDistance(renderDistance);
            generateInitialChunks(playerPos);
        }

        static void UpdateChunks(const Frustum &frustum, const vec3 &playerPos);
        static void RenderChunks(Shader &shader, const Frustum &frustum);
        static vec3 HandleCollisions(vec3 playerPos, vec3 min, vec3 max);
        static void HandleRay(const Ray &ray, ChunkOperations operation);

    private:
        static ivec3 currentPos;

        static priority_queue<pair<float, ivec3>, vector<pair<float, ivec3>>, ComparePair> chunksToCreate;

        static counting_semaphore<1> meshSem;
        static unordered_map<ivec3, Chunk *> chunksToMesh;

        static counting_semaphore<1> initSem;
        static unordered_map<ivec3, Chunk *> chunksToInitialize;

        static counting_semaphore<1> bufferSem;
        static unordered_map<ivec3, Chunk *> chunksToBuffer;

        static counting_semaphore<1> renderSem;
        static unordered_map<ivec3, Chunk *> chunksToRender;

        static counting_semaphore<1> loadSem;
        static unordered_map<ivec3, Chunk *> chunksLoaded;

        static counting_semaphore<1> deleteSem;
        static unordered_set<ivec3> chunksToDelete;

        static ContainerThreadExecutor ThreadExecutor;

        static void generateInitialChunks(vec3 &playerPos);

        static unordered_map<ivec3, Chunk *> setChunkNeighbors(Chunk *chunk, ivec3 pos);
        static void generateChunks(vec3 cameraPos, Frustum frustum);
        static void unloadChunks(vec3 cameraPos);

        static void removeChunk(ivec3 pos);
        static void meshChunks();
        static void bufferChunks();
    };

    ivec3 ChunkManager::currentPos;

    priority_queue<pair<float, ivec3>, vector<pair<float, ivec3>>, ComparePair> ChunkManager::chunksToCreate;

    counting_semaphore<1> ChunkManager::meshSem(1);
    unordered_map<ivec3, Chunk *> ChunkManager::chunksToMesh;

    counting_semaphore<1> ChunkManager::initSem(1);
    unordered_map<ivec3, Chunk *> ChunkManager::chunksToInitialize;

    counting_semaphore<1> ChunkManager::bufferSem(1);
    unordered_map<ivec3, Chunk *> ChunkManager::chunksToBuffer;

    counting_semaphore<1> ChunkManager::renderSem(1);
    unordered_map<ivec3, Chunk *> ChunkManager::chunksToRender;

    counting_semaphore<1> ChunkManager::loadSem(1);
    unordered_map<ivec3, Chunk *> ChunkManager::chunksLoaded;

    counting_semaphore<1> ChunkManager::deleteSem(1);
    unordered_set<ivec3> ChunkManager::chunksToDelete;

    ContainerThreadExecutor ChunkManager::ThreadExecutor;

    //====| Public Functions |=====//

    /**
     * @brief Update the chunks based on the player's position and the frustum.
     *
     * @param frustum The frustum for visibility culling.
     * @param playerPos The player's position in the world.
     */
    void ChunkManager::UpdateChunks(const Frustum &frustum, const vec3 &playerPos)
    {
        generateChunks(playerPos, frustum);
        unloadChunks(playerPos);

        initSem.acquire();
        unordered_map<ivec3, Chunk *> copy = chunksToInitialize;
        initSem.release();

        if (copy.empty())
        {
            return;
        }

        vector<ivec3> keys = ThreadExecutor.Execute<ivec3, Chunk *>(
            copy,
            [](ivec3 key, Chunk *chunk)
            {
                chunk->Initialize();
            });

        bufferSem.acquire();
        ThreadExecutor.Execute<ivec3>(
            keys,
            [&](ivec3 key)
            {
                chunksToBuffer[key] = copy[key];
            });
        bufferSem.release();

        initSem.acquire();
        ThreadExecutor.Execute<ivec3>(
            keys,
            [](ivec3 key)
            {
                chunksToInitialize.erase(key);
            });
        initSem.release();
    }

    /**
     * @brief Handle collisions for the player in the chunk space.
     *
     * @param playerPos The player's position in the world.
     * @param min The minimum bounding box coordinates.
     * @param max The maximum bounding box coordinates.
     * @return vec3 The new position after resolving collisions.
     */
    vec3 ChunkManager::HandleCollisions(vec3 playerPos, vec3 min, vec3 max)
    {
        int chunkSize = Config::GetChunkSize();
        vec3 cameraChunkPos = floor(playerPos / (float)chunkSize);
        cameraChunkPos *= chunkSize;

        vec3 newPos(0);

        loadSem.acquire();
        if (chunksLoaded.find(cameraChunkPos) != chunksLoaded.end())
        {
            newPos = chunksLoaded[cameraChunkPos]->ResolveCollisions(min, max);
        }
        loadSem.release();

        return newPos;
    }

    /**
     * @brief Handle ray operations in the chunk space.
     *
     * @param ray The ray to handle.
     * @param operation The operation to perform (BREAK or PLACE).
     */
    void ChunkManager::HandleRay(const Ray &ray, ChunkOperations operation)
    {
        int chunkSize = Config::GetChunkSize();
        vec3 rayPos = ray.GetOrigin();
        vec3 rayDir = ray.GetDirection();

        ivec3 currentChunk = glm::floor(rayPos / (float)chunkSize);
        ivec3 step = glm::sign(rayDir);
        ivec3 tMax = (glm::floor(rayPos / (float)chunkSize + vec3(step)) * (float)chunkSize - rayPos) / rayDir;
        ivec3 tDelta = glm::abs((float)chunkSize / rayDir);

        loadSem.acquire();
        unordered_map<ivec3, Chunk *> copy = chunksLoaded;
        loadSem.release();

        for (int i = 0; i < ray.GetLength(); ++i)
        {
            auto it = copy.find(currentChunk * chunkSize);
            if (it != copy.end())
            {
                Chunk *chunk = it->second;
                if (chunk->HandleRay(ray, operation))
                    break;
            }
            // Move to the next chunk
            if (tMax.x < tMax.y && tMax.x < tMax.z)
            {
                tMax.x += tDelta.x;
                currentChunk.x += step.x;
            }
            else if (tMax.y < tMax.z)
            {
                tMax.y += tDelta.y;
                currentChunk.y += step.y;
            }
            else
            {
                tMax.z += tDelta.z;
                currentChunk.z += step.z;
            }
        }
    }

    /**
     * @brief Render the chunks in the chunk space.
     *
     * @param shader The shader to use for rendering.
     * @param frustum The frustum for visibility culling.
     * @param dt The delta time for rendering.
     */
    void ChunkManager::RenderChunks(Shader &shader, const Frustum &frustum)
    {
        deleteSem.acquire();
        unordered_set<ivec3> copyDelete = chunksToDelete;
        deleteSem.release();

        for (ivec3 pos : copyDelete)
        {
            loadSem.acquire();
            delete chunksLoaded[pos];
            chunksLoaded.erase(pos);
            loadSem.release();
        }

        deleteSem.acquire();
        for (ivec3 pos : copyDelete)
            chunksToDelete.erase(pos);
        deleteSem.release();

        meshChunks();

        bufferChunks();

        renderSem.acquire();
        unordered_map<ivec3, Chunk *> copy = chunksToRender;
        renderSem.release();

        for (const auto &pair : copy)
            pair.second->Render(shader);
    }

    //====| Private Functions |=====//

    /**
     * @brief Mesh the chunks that are ready for meshing. Add to the initialize queue.
     *
     */
    void ChunkManager::meshChunks()
    {
        meshSem.acquire();
        unordered_map<ivec3, Chunk *> copy = chunksToMesh;
        meshSem.release();

        if (copy.empty())
            return;

        vector<ivec3> keys;

        for (const auto &pair : copy)
        {
            keys.push_back(pair.first);
            pair.second->CreateMeshes();
        }

        initSem.acquire();
        for (ivec3 key : keys)
            chunksToInitialize[key] = copy[key];
        initSem.release();

        meshSem.acquire();
        for (ivec3 key : keys)
            chunksToMesh.erase(key);
        meshSem.release();
    }

    /**
     * @brief Create the buffers for the chunks that are ready for buffering. Add to the render queue.
     *
     */
    void ChunkManager::bufferChunks()
    {
        vector<ivec3> keys;

        bufferSem.acquire();
        unordered_map<ivec3, Chunk *> copy = chunksToBuffer;
        bufferSem.release();

        for (const auto &pair : copy)
        {
            keys.push_back(pair.first);
            pair.second->GenerateBuffers();
        }

        renderSem.acquire();
        for (ivec3 key : keys)
            chunksToRender[key] = copy[key];
        renderSem.release();

        bufferSem.acquire();
        for (ivec3 key : keys)
            chunksToBuffer.erase(key);
        bufferSem.release();
    }

    /**
     * @brief Unload chunks that are outside the render distance. Add to the delete queue.
     *
     * @param cameraPos
     */
    void ChunkManager::unloadChunks(vec3 cameraPos)
    {
        int chunkSize = Config::GetChunkSize();
        vec3 cameraChunkPos = floor(cameraPos / (float)chunkSize);

        int radius = Config::GetRenderDistance();
        int radiusSquared = radius * radius;

        vector<ivec3> chunksToUnload;

        loadSem.acquire();
        for (const auto &pair : chunksLoaded)
        {
            vec3 pos = pair.first;
            vec3 relativePos = pos / (float)chunkSize - cameraChunkPos;

            if (relativePos.x * relativePos.x + relativePos.y * relativePos.y + relativePos.z * relativePos.z > radiusSquared)
            {
                chunksToUnload.push_back(pos);
            }
        }
        loadSem.release();

        ThreadExecutor.Execute<ivec3>(
            chunksToUnload,
            [](ivec3 pos)
            {
                removeChunk(pos);

                deleteSem.acquire();
                chunksToDelete.insert(pos);
                deleteSem.release();
            });
    }

    /**
     * @brief Set the neighbors of a chunk based on its position.
     *
     * @param chunk The chunk to set neighbors for.
     * @param pos The position of the chunk.
     * @return unordered_map<vec3, Chunk *> The chunks that need to be reloaded.
     */
    unordered_map<ivec3, Chunk *> ChunkManager::setChunkNeighbors(Chunk *chunk, ivec3 pos)
    {
        unordered_map<ivec3, Chunk *> reload;
        // Set neighbors by checking adjacent chunk positions
        unordered_map<ChunkNeighbor, vec3> neighborMap =
            {
                {ChunkNeighbor::LEFT, pos + ivec3(-Config::GetChunkSize(), 0, 0)},
                {ChunkNeighbor::RIGHT, pos + ivec3(Config::GetChunkSize(), 0, 0)},
                {ChunkNeighbor::BACK, pos + ivec3(0, 0, -Config::GetChunkSize())},
                {ChunkNeighbor::FRONT, pos + ivec3(0, 0, Config::GetChunkSize())},
                {ChunkNeighbor::BOTTOM, pos + ivec3(0, -Config::GetChunkSize(), 0)},
                {ChunkNeighbor::TOP, pos + ivec3(0, Config::GetChunkSize(), 0)}};

        // For each direction, check if the neighboring chunk exists
        for (const auto &pair : neighborMap)
        {
            vec3 neighborPos = pair.second;

            // If the neighbor exists in the chunksToMesh map, set the neighbor

            if (chunksLoaded.find(neighborPos) != chunksLoaded.end())
            {
                chunk->SetNeighbor(chunksLoaded[neighborPos], pair.first);
                reload[pos] = chunksLoaded[pos];
                reload[neighborPos] = chunksLoaded[neighborPos];
            }
        }
        return reload;
    }

    /**
     * @brief Remove a chunk from all queues and maps.
     *
     * @param pos The position of the chunk to remove.
     */
    void ChunkManager::removeChunk(ivec3 pos)
    {
        meshSem.acquire();
        chunksToMesh.erase(pos);
        meshSem.release();

        initSem.acquire();
        chunksToInitialize.erase(pos);
        initSem.release();

        bufferSem.acquire();
        chunksToBuffer.erase(pos);
        bufferSem.release();

        renderSem.acquire();
        chunksToRender.erase(pos);
        renderSem.release();
    }

    /**
     * @brief Generate chunks in a radius around the camera position based on the frustum.
     *
     * @param cameraPos The camera position in the world.
     * @param frustum The frustum for visibility culling.
     */
    void ChunkManager::generateChunks(vec3 cameraPos, Frustum frustum)
    {
        int chunkSize = Config::GetChunkSize();
        ivec3 cameraChunkPos = floor(cameraPos / (float)chunkSize);

        if (cameraChunkPos != currentPos)
        {
            int radius = Config::GetRenderDistance();
            int radiusSquared = radius * radius;

            for (int x = -radius; x <= radius; x++)
                for (int z = -radius; z <= radius; z++)
                    for (int y = -radius; y <= radius; y++)
                    {
                        ivec3 chunkPos = (cameraChunkPos + ivec3(x, y, z)) * chunkSize;

                        loadSem.acquire();
                        if (chunksLoaded.find(chunkPos) == chunksLoaded.end())
                        {
                            if (x * x + z * z + y * y <= radiusSquared)
                            {
                                float dist = glm::distance(cameraPos, vec3(chunkPos));
                                if (frustum.IsBoxInFrustum(chunkPos, chunkPos * chunkSize))
                                {
                                    dist /= 10;
                                }
                                chunksToCreate.emplace(dist, chunkPos);
                            }
                        }
                        loadSem.release();
                    }

            currentPos = cameraChunkPos;
        }

        vector<ivec3> create;

        while (!chunksToCreate.empty())
        {
            create.push_back(chunksToCreate.top().second);
            chunksToCreate.pop();
        }

        int chunksCreated = ThreadExecutor.Execute<ivec3>(
            create,
            [](ivec3 pos)
            {
                Chunk *chunk = new Chunk(pos);
                meshSem.acquire();
                chunksToMesh[pos] = chunk;
                meshSem.release();

                loadSem.acquire();
                chunksLoaded[pos] = chunk;
                loadSem.release();
            });

        for (int i = 0; i < chunksCreated; i++)
        {
            loadSem.acquire();
            unordered_map<ivec3, Chunk *> temp = setChunkNeighbors(chunksLoaded[create[i]], create[i]);
            loadSem.release();
        }
    }

    /**
     * @brief Generate the initial chunks around the player position.
     *
     * @param playerPos The player's position in the world.
     */
    void ChunkManager::generateInitialChunks(vec3 &playerPos)
    {
        int chunkSize = Config::GetChunkSize();
        int renderDistance = Config::GetRenderDistance();
        Chunk *center = nullptr;
        for (int i = -2; i < 2; i++)
            for (int j = -2; j < 2; j++)
                for (int k = Config::GetMaxHeight(); k >= 0; k--)
                {
                    int xPos = i * chunkSize;
                    int yPos = k * chunkSize;
                    int zPos = j * chunkSize;
                    ivec3 vecPos(xPos, yPos, zPos);
                    Chunk *chunk = new Chunk(vecPos);
                    chunk->CreateMeshes();
                    chunk->Initialize();
                    chunk->GenerateBuffers();
                    chunksToRender[vecPos] = chunk;
                    if (chunk->HasVoxels() && i == 0 && j == 0 && center == nullptr)
                    {
                        center = chunk;
                    }
                }
        if (center == nullptr)
        {
            playerPos = vec3(0);
        }
        else
        {
            vec3 centerPos = center->GetHighestMiddleVoxel();
            playerPos = centerPos + vec3(0, 3, 0);
        }
        currentPos = playerPos * vec3((float)chunkSize);
    }
};

#endif