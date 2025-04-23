#ifndef CHUNKMANAGER_H
#define CHUNKMANAGER_H

#include "Chunk.h"
#include "Config.h"
#include "../Utility/Utility.h"
#include "../Texture.h"
#include "../Concurrent/VectorThreadExecutor.h"

#include <mutex>
#include <semaphore>
#include <queue>

using glm::vec4, std::mutex, std::lock_guard, std::counting_semaphore, std::queue, std::priority_queue, std::pair;
using namespace Engine::Concurrent;
using namespace Engine::Utility;

namespace Engine::ChunkSpace
{
    struct CompareChunks
    {
        bool operator()(const pair<float, vec3> &a, const pair<float, vec3> &b) const
        {
            return a.first > b.first; // Min-heap: smallest float at top
        }
    };

    class ChunkManager
    {
    private:
        static mutex chunkMutex;
        static counting_semaphore<1> chunkSem;
        static vector<Chunk *> chunks;
        static counting_semaphore<1> createSem;
        static priority_queue<pair<float, vec3>, vector<pair<float, vec3>>, CompareChunks> chunksToCreate;
        static counting_semaphore<1> initSem;
        static vector<Chunk *> chunksToInitialize;
        static VectorThreadExecutor<Chunk *> Initializer;

        static void generateChunks(vec3 cameraPos);

    public:
        static void InitChunkManager(vec3 cameraPos, int renderDistance = 8)
        {
            Texture::InitializeTextures();
            Config::SetRenderDistance(renderDistance);
            chunks.reserve(renderDistance * renderDistance * Config::GetMaxHeight());
            generateChunks(cameraPos);
        }

        static void UpdateChunks(const Frustum &frustum);
        static void RenderChunks(Shader &shader, const Frustum &frustum);
    };

    counting_semaphore<1> ChunkManager::chunkSem(1);
    counting_semaphore<1> ChunkManager::createSem(1);
    counting_semaphore<1> ChunkManager::initSem(1);
    mutex ChunkManager::chunkMutex;
    vector<Chunk *> ChunkManager::chunks;
    priority_queue<pair<float, vec3>, vector<pair<float, vec3>>, CompareChunks> ChunkManager::chunksToCreate;
    vector<Chunk *> ChunkManager::chunksToInitialize;
    VectorThreadExecutor<Chunk *> ChunkManager::Initializer;

    void ChunkManager::RenderChunks(Shader &shader, const Frustum &frustum)
    {
        int count = 0;
        createSem.acquire();
        priority_queue<pair<float, vec3>, vector<pair<float, vec3>>, CompareChunks> copy = chunksToCreate;
        createSem.release();
        while (!copy.empty())
        {
            pair<float, vec3> top = copy.top();
            vec3 chunkPos = top.second;
            int pos[3] = {(int)chunkPos.x, (int)chunkPos.y, (int)chunkPos.z};

            Chunk *created = new Chunk(pos);
            initSem.acquire();
            chunksToInitialize.push_back(created);
            initSem.release();

            copy.pop();

            count++;
        }

        createSem.acquire();
        for (int i = 0; i < count; i++)
        {
            chunksToCreate.pop();
        }
        createSem.release();

        chunkSem.acquire();
        vector<Chunk *> chunksCopy = chunks;
        chunkSem.release();
        for (Chunk *chunk : chunksCopy)
        {
            vec3 min(chunk->GetPosition()[0], chunk->GetPosition()[1], chunk->GetPosition()[2]);
            vec3 max = min + vec3((float)(Config::GetChunkSize()));

            if (frustum.IsBoxInFrustum(min, max))
            {
                chunk->Render(shader);
            }
        }
    }

    void ChunkManager::UpdateChunks(const Frustum &frustum)
    {
        int count = 0;

        Initializer.AssignTask(
            [](Chunk *chunk)
            {
                chunk->Initialize();
                chunkSem.acquire();
                chunks.push_back(chunk);
                chunkSem.release();
            });

        int initializedCount = Initializer.Execute(chunksToInitialize);
        initSem.acquire();
        chunksToInitialize.erase(chunksToInitialize.begin(), chunksToInitialize.begin() + initializedCount);
        initSem.release();
    }

    void ChunkManager::generateChunks(vec3 cameraPos)
    {
        int x = (int)(cameraPos.x / Config::GetChunkSize());
        int y = (int)(cameraPos.y / Config::GetChunkSize());
        int z = (int)(cameraPos.z / Config::GetChunkSize());

        for (int i = -Config::GetRenderDistance(); i <= Config::GetRenderDistance(); i++) // X
        {
            for (int j = -Config::GetRenderDistance(); j <= Config::GetRenderDistance(); j++)
            {
                for (int k = 0; k < Config::GetMaxHeight(); k++) // Z
                {
                    int xPos = (x + i) * Config::GetChunkSize();
                    int yPos = (y + k) * Config::GetChunkSize();
                    int zPos = (z + j) * Config::GetChunkSize();
                    vec3 chunkPos(xPos, yPos, zPos);
                    float dist = glm::distance(cameraPos, chunkPos);
                    createSem.acquire();
                    chunksToCreate.emplace(dist, chunkPos);
                    createSem.release();
                }
            }
        }
    }
};

#endif