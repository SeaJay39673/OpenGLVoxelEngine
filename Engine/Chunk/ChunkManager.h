#ifndef CHUNKMANAGER_H
#define CHUNKMANAGER_H

#include "Chunk.h"
#include "Config.h"
#include "../Utility/Utility.h"
#include "../Texture.h"
#include "../Concurrent/ContainerThreadExecutor.h"

#include <mutex>
#include <semaphore>
#include <queue>

using glm::vec4, std::mutex, std::lock_guard, std::counting_semaphore, std::queue, std::priority_queue, std::pair;
using namespace Engine::Concurrent;
using namespace Engine::Utility;

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
    class ChunkManager
    {
    private:
        static vec3 currentPos;

        static counting_semaphore<1> meshSem;
        static unordered_map<vec3, Chunk *> chunksToMesh;

        static counting_semaphore<1> initSem;
        static unordered_map<vec3, Chunk *> chunksToInitialize;

        static counting_semaphore<1> bufferSem;
        static unordered_map<vec3, Chunk *> chunksToBuffer;

        static counting_semaphore<1> renderSem;
        static unordered_map<vec3, Chunk *> chunksToRender;

        static unordered_map<vec3, Chunk *> chunksLoaded;

        static ContainerThreadExecutor ThreadExecutor;

        static void generateChunks(Camera &camera);

        static void meshChunks();
        static void bufferChunks();

    public:
        static void InitChunkManager(Camera &camera, int renderDistance = 8)
        {
            Texture::InitializeTextures();
            Config::SetRenderDistance(renderDistance);
            generateChunks(camera);
        }

        static void UpdateChunks(const Frustum &frustum, const vec3 &cameraPos);
        static void RenderChunks(Shader &shader, const Frustum &frustum);
    };

    vec3 ChunkManager::currentPos;

    counting_semaphore<1> ChunkManager::meshSem(1);
    unordered_map<vec3, Chunk *> ChunkManager::chunksToMesh;

    counting_semaphore<1> ChunkManager::initSem(1);
    unordered_map<vec3, Chunk *> ChunkManager::chunksToInitialize;

    counting_semaphore<1> ChunkManager::bufferSem(1);
    unordered_map<vec3, Chunk *> ChunkManager::chunksToBuffer;

    counting_semaphore<1> ChunkManager::renderSem(1);
    unordered_map<vec3, Chunk *> ChunkManager::chunksToRender;

    unordered_map<vec3, Chunk *> ChunkManager::chunksLoaded;

    ContainerThreadExecutor ChunkManager::ThreadExecutor;

    void ChunkManager::UpdateChunks(const Frustum &frustum, const vec3 &cameraPos)
    {
        int chunkSize = Config::GetChunkSize();
        vec3 cameraChunkPos = floor(cameraPos / (float)chunkSize);

        if (cameraChunkPos != currentPos)
        {
            int radius = Config::GetRenderDistance();
            int radiusSquared = radius * radius;

            for (int x = -radius; x <= radius; x++)
                for (int z = -radius; z <= radius; z++)
                    for (int y = Config::GetMaxHeight(); y >= 0; y--)
                    {
                        vec3 chunkPos = (cameraChunkPos + vec3(x, y, z)) * (float)chunkSize;

                        if (chunksLoaded.find(chunkPos) == chunksLoaded.end())
                        {
                            if (x * x + z * z <= radiusSquared)
                            {
                                int pos[3] = {(int)chunkPos.x, (int)chunkPos.y, (int)chunkPos.z};
                                Chunk *chunk = new Chunk(pos);

                                meshSem.acquire();
                                chunksToMesh[chunkPos] = chunk;
                                meshSem.release();

                                chunksLoaded[chunkPos] = chunk;

                                if (chunk->HasVoxels())
                                    break;
                            }
                        }
                    }

            currentPos = cameraChunkPos;
        }

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

    void ChunkManager::RenderChunks(Shader &shader, const Frustum &frustum)
    {
        meshChunks();

        bufferChunks();

        renderSem.acquire();
        unordered_map<vec3, Chunk *> copy = chunksToRender;
        renderSem.release();

        for (const auto &pair : copy)
            pair.second->Render(shader);
    }

    void ChunkManager::generateChunks(Camera &camera)
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
                    // chunksLoaded[vecPos] = chunk;
                    if (chunk->HasVoxels())
                    {
                        if (i == 0 && j == 0)
                            center = chunk;
                        break;
                    }
                }
        if (center == nullptr)
        {
            camera.SetCameraPos(vec3(0, 0, 0));
        }
        else
        {
            vec3 centerPos = center->GetHighestMiddleVoxel();
            camera.SetCameraPos(centerPos + vec3(0, 3, 0));
        }
        currentPos = camera.GetCameraPos() * vec3((float)chunkSize);
    }
};

#endif