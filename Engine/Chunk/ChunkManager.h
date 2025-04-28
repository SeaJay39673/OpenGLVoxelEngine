#ifndef CHUNKMANAGER_H
#define CHUNKMANAGER_H

#include "Chunk.h"
#include "Config.h"
#include "../Utility/Utility.h"
#include "../Texture.h"
#include "../Concurrent/ContainerThreadExecutor.h"
#include "../Entities/Player.h"

#include <mutex>
#include <semaphore>
#include <unordered_set>
#include <queue>
#include <chrono>

using glm::vec4, std::mutex, std::lock_guard, std::counting_semaphore, std::queue, std::priority_queue, std::pair, Engine::Entities::Player;
using namespace Engine::Concurrent;
using namespace Engine::Utility;
using namespace std::chrono;

namespace std
{
    template <>
    struct hash<glm::vec3>
    {
        size_t operator()(const glm::vec3 &v) const
        {
            return ((hash<int>()((int)v.x) ^ (hash<int>()((int)v.y) << 1)) >> 1) ^ (hash<int>()((int)v.z) << 1);
        }
    };
}

namespace Engine::ChunkSpace
{

    struct ComparePair
    {
        bool operator()(const std::pair<float, glm::vec3> &a, const std::pair<float, glm::vec3> &b) const
        {
            return a.first > b.first; // Min-heap behavior: smaller float has higher priority
        }
    };

    class ChunkManager
    {
    private:
        static vec3 currentPos;

        static priority_queue<pair<float, vec3>, vector<pair<float, vec3>>, ComparePair> chunksToCreate;

        static counting_semaphore<1> meshSem;
        static unordered_map<vec3, Chunk *> chunksToMesh;

        static counting_semaphore<1> initSem;
        static unordered_map<vec3, Chunk *> chunksToInitialize;

        static counting_semaphore<1> bufferSem;
        static unordered_map<vec3, Chunk *> chunksToBuffer;

        static counting_semaphore<1> renderSem;
        static unordered_map<vec3, Chunk *> chunksToRender;

        static counting_semaphore<1> loadSem;
        static unordered_map<vec3, Chunk *> chunksLoaded;

        static counting_semaphore<1> deleteSem;
        static unordered_set<vec3> chunksToDelete;

        static ContainerThreadExecutor ThreadExecutor;

        static void generateInitialChunks(Player &player);

        static unordered_map<vec3, Chunk *> setChunkNeighbors(Chunk *chunk, vec3 pos);
        static void generateChunks(vec3 cameraPos, Frustum frustum);
        static void unloadChunks(vec3 cameraPos);

        static void removeChunk(vec3 pos);
        static void meshChunks();
        static void bufferChunks();
        static void updateCollisions(Player &player, duration<float> dt);

    public:
        static void InitChunkManager(Player &player, int renderDistance = 8)
        {
            glDisable(GL_CULL_FACE);
            Texture::InitializeTextures();
            Config::SetRenderDistance(renderDistance);
            generateInitialChunks(player);
        }

        static void UpdateChunks(const Frustum &frustum, Player &player);
        static void RenderChunks(Shader &shader, const Frustum &frustum, Player &player, duration<float> dt);
    };

    vec3 ChunkManager::currentPos;

    priority_queue<pair<float, vec3>, vector<pair<float, vec3>>, ComparePair> ChunkManager::chunksToCreate;

    counting_semaphore<1> ChunkManager::meshSem(1);
    unordered_map<vec3, Chunk *> ChunkManager::chunksToMesh;

    counting_semaphore<1> ChunkManager::initSem(1);
    unordered_map<vec3, Chunk *> ChunkManager::chunksToInitialize;

    counting_semaphore<1> ChunkManager::bufferSem(1);
    unordered_map<vec3, Chunk *> ChunkManager::chunksToBuffer;

    counting_semaphore<1> ChunkManager::renderSem(1);
    unordered_map<vec3, Chunk *> ChunkManager::chunksToRender;

    counting_semaphore<1> ChunkManager::loadSem(1);
    unordered_map<vec3, Chunk *> ChunkManager::chunksLoaded;

    counting_semaphore<1> ChunkManager::deleteSem(1);
    unordered_set<vec3> ChunkManager::chunksToDelete;

    ContainerThreadExecutor ChunkManager::ThreadExecutor;

    void ChunkManager::unloadChunks(vec3 cameraPos)
    {
        int chunkSize = Config::GetChunkSize();
        vec3 cameraChunkPos = floor(cameraPos / (float)chunkSize);

        int radius = Config::GetRenderDistance();
        int radiusSquared = radius * radius;

        vector<vec3> chunksToUnload;

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

        ThreadExecutor.Execute<vec3>(
            chunksToUnload,
            [](vec3 pos)
            {
                removeChunk(pos);

                deleteSem.acquire();
                chunksToDelete.insert(pos);
                deleteSem.release();
            });
    }

    unordered_map<vec3, Chunk *> ChunkManager::setChunkNeighbors(Chunk *chunk, vec3 pos)
    {
        unordered_map<vec3, Chunk *> reload;
        // Set neighbors by checking adjacent chunk positions
        unordered_map<ChunkNeighbor, vec3> neighborMap =
            {
                {ChunkNeighbor::LEFT, pos + vec3(-Config::GetChunkSize(), 0, 0)},
                {ChunkNeighbor::RIGHT, pos + vec3(Config::GetChunkSize(), 0, 0)},
                {ChunkNeighbor::BACK, pos + vec3(0, 0, -Config::GetChunkSize())},
                {ChunkNeighbor::FRONT, pos + vec3(0, 0, Config::GetChunkSize())},
                {ChunkNeighbor::BOTTOM, pos + vec3(0, -Config::GetChunkSize(), 0)},
                {ChunkNeighbor::TOP, pos + vec3(0, Config::GetChunkSize(), 0)}};

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

    void ChunkManager::removeChunk(vec3 pos)
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

    void ChunkManager::generateChunks(vec3 cameraPos, Frustum frustum)
    {
        int chunkSize = Config::GetChunkSize();
        vec3 cameraChunkPos = floor(cameraPos / (float)chunkSize);

        if (cameraChunkPos != currentPos)
        {
            int radius = Config::GetRenderDistance();
            int radiusSquared = radius * radius;

            for (int x = -radius; x <= radius; x++)
                for (int z = -radius; z <= radius; z++)
                    for (int y = -radius; y <= radius; y++)
                    {
                        vec3 chunkPos = (cameraChunkPos + vec3(x, y, z)) * (float)chunkSize;

                        loadSem.acquire();
                        if (chunksLoaded.find(chunkPos) == chunksLoaded.end())
                        {
                            if (x * x + z * z + y * y <= radiusSquared)
                            {
                                float dist = glm::distance(cameraPos, chunkPos);
                                if (frustum.IsBoxInFrustum(chunkPos, chunkPos * (float)chunkSize))
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

        vector<vec3> create;

        while (!chunksToCreate.empty())
        {
            create.push_back(chunksToCreate.top().second);
            chunksToCreate.pop();
        }

        int chunksCreated = ThreadExecutor.Execute<vec3>(
            create,
            [](vec3 pos)
            {
                int loc[3] = {(int)pos.x, (int)pos.y, (int)pos.z};
                Chunk *chunk = new Chunk(loc);
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
            unordered_map<vec3, Chunk *> temp = setChunkNeighbors(chunksLoaded[create[i]], create[i]);
            loadSem.release();
        }
    }

    void ChunkManager::UpdateChunks(const Frustum &frustum, Player &player)
    {
        vec3 playerPos = player.GetCamera().GetCameraPos();
        generateChunks(playerPos, frustum);
        unloadChunks(playerPos);

        initSem.acquire();
        unordered_map<vec3, Chunk *> copy = chunksToInitialize;
        initSem.release();

        if (copy.empty())
        {
            return;
        }

        vector<vec3> keys = ThreadExecutor.Execute<vec3, Chunk *>(
            copy,
            [](vec3 key, Chunk *chunk)
            {
                chunk->Initialize();
            });

        bufferSem.acquire();
        ThreadExecutor.Execute<vec3>(
            keys,
            [&](vec3 key)
            {
                chunksToBuffer[key] = copy[key];
            });
        bufferSem.release();

        initSem.acquire();
        ThreadExecutor.Execute<vec3>(
            keys,
            [](vec3 key)
            {
                chunksToInitialize.erase(key);
            });
        initSem.release();
    }

    void ChunkManager::meshChunks()
    {
        meshSem.acquire();
        unordered_map<vec3, Chunk *> copy = chunksToMesh;
        meshSem.release();

        if (copy.empty())
            return;

        vector<vec3> keys;

        for (const auto &pair : copy)
        {
            keys.push_back(pair.first);
            pair.second->CreateMeshes();
        }

        initSem.acquire();
        for (vec3 key : keys)
            chunksToInitialize[key] = copy[key];
        initSem.release();

        meshSem.acquire();
        for (vec3 key : keys)
            chunksToMesh.erase(key);
        meshSem.release();
    }

    void ChunkManager::bufferChunks()
    {
        vector<vec3> keys;

        bufferSem.acquire();
        unordered_map<vec3, Chunk *> copy = chunksToBuffer;
        bufferSem.release();

        for (const auto &pair : copy)
        {
            keys.push_back(pair.first);
            pair.second->GenerateBuffers();
        }

        renderSem.acquire();
        for (vec3 key : keys)
            chunksToRender[key] = copy[key];
        renderSem.release();

        bufferSem.acquire();
        for (vec3 key : keys)
            chunksToBuffer.erase(key);
        bufferSem.release();
    }

    void ChunkManager::updateCollisions(Player &player, duration<float> dt)
    {
        vec3 pos = player.GetPos();
        vec3 min = player.GetMin();
        vec3 max = player.GetMax();

        int chunkSize = Config::GetChunkSize();
        vec3 cameraChunkPos = floor(pos / (float)chunkSize);
        cameraChunkPos *= chunkSize;

        loadSem.acquire();
        if (chunksLoaded.find(cameraChunkPos) != chunksLoaded.end())
        {
            player.Update(chunksLoaded[cameraChunkPos]->ResolveCollisions(min, max), dt);
        }
        loadSem.release();
    }

    void ChunkManager::RenderChunks(Shader &shader, const Frustum &frustum, Player &player, duration<float> dt)
    {

        updateCollisions(player, dt);

        deleteSem.acquire();
        unordered_set<vec3> copyDelete = chunksToDelete;
        deleteSem.release();

        for (vec3 pos : copyDelete)
        {
            loadSem.acquire();
            delete chunksLoaded[pos];
            chunksLoaded.erase(pos);
            loadSem.release();
        }

        deleteSem.acquire();
        for (vec3 pos : copyDelete)
            chunksToDelete.erase(pos);
        deleteSem.release();

        meshChunks();

        bufferChunks();

        renderSem.acquire();
        unordered_map<vec3, Chunk *> copy = chunksToRender;
        renderSem.release();

        for (const auto &pair : copy)
            pair.second->Render(shader);
    }

    void ChunkManager::generateInitialChunks(Player &player)
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
                    int pos[3] = {xPos, yPos, zPos};
                    vec3 vecPos(xPos, yPos, zPos);
                    Chunk *chunk = new Chunk(pos);
                    chunksToMesh[vecPos] = chunk;
                    if (chunk->HasVoxels() && i == 0 && j == 0 && center == nullptr)
                    {
                        center = chunk;
                    }
                }
        if (center == nullptr)
        {
            player.GetCamera().SetCameraPos(vec3(0, 0, 0));
        }
        else
        {
            vec3 centerPos = center->GetHighestMiddleVoxel();
            player.GetCamera().SetCameraPos(centerPos + vec3(0, 3, 0));
        }
        currentPos = player.GetCamera().GetCameraPos() * vec3((float)chunkSize);
    }
};

#endif