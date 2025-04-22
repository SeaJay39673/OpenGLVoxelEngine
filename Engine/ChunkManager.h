#ifndef CHUNKMANAGER_H
#define CHUNKMANAGER_H

#include "Chunk.h"
#include "Texture.h"
#include "Frustum.h"
#include "Generator.h"

#include <mutex>
#include <semaphore>
#include <queue>

using glm::vec4, std::mutex, std::lock_guard, std::counting_semaphore, std::queue, std::priority_queue, std::pair;
using namespace Generator;

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
    static int renderDistance;
    static mutex chunkMutex;
    static counting_semaphore<1> chunkSem;
    static vector<Chunk *> chunks;
    static counting_semaphore<1> createSem;
    static priority_queue<pair<float, vec3>, vector<pair<float, vec3>>, CompareChunks> chunksToCreate;
    // static queue<vec3> chunksToCreate;
    static counting_semaphore<1> initSem;
    static vector<Chunk *> chunksToInitialize;

    static void generateChunks(vec3 cameraPos);

public:
    static void InitChunkManager(vec3 cameraPos, int renderDistance = 16)
    {
        Texture::InitializeTextures();
        ChunkManager::renderDistance = renderDistance;
        chunks.reserve(renderDistance * renderDistance * MaxHeight);
        generateChunks(cameraPos);
    }

    static void SetRenderDistance(int distance) { renderDistance = distance; }
    static int GetRenderDistance() { return renderDistance; }
    static void UpdateChunks(const Frustum &frustum);
    static void RenderChunks(Shader &shader, const Frustum &frustum);
};

counting_semaphore<1> ChunkManager::chunkSem(1);
counting_semaphore<1> ChunkManager::createSem(1);
counting_semaphore<1> ChunkManager::initSem(1);
mutex ChunkManager::chunkMutex;
int ChunkManager::renderDistance = 0;
vector<Chunk *> ChunkManager::chunks;
priority_queue<pair<float, vec3>, vector<pair<float, vec3>>, CompareChunks> ChunkManager::chunksToCreate;

// queue<vec3> ChunkManager::chunksToCreate;
vector<Chunk *> ChunkManager::chunksToInitialize;

void ChunkManager::RenderChunks(Shader &shader, const Frustum &frustum)
{
    int count = 0;
    createSem.acquire();
    while (!chunksToCreate.empty() && count < 2)
    {
        pair<float, vec3> top = chunksToCreate.top();
        vec3 chunkPos = top.second;
        // vec3 front = chunksToCreate.front();
        int pos[3] = {(int)chunkPos.x, (int)chunkPos.y, (int)chunkPos.z};

        initSem.acquire();
        chunksToInitialize.push_back(new Chunk(pos));
        initSem.release();

        chunksToCreate.pop();

        count++;
    }
    createSem.release();

    chunkSem.acquire();
    for (Chunk *chunk : chunks)
    {
        // Calculate the bounding box of the chunk
        vec3 min(chunk->GetPosition()[0], chunk->GetPosition()[1], chunk->GetPosition()[2]);
        vec3 max = min + vec3((float)(Chunk::ChunkSize()));

        // Check if the chunk is in the frustum
        if (frustum.IsBoxInFrustum(min, max))
        {
            chunk->Render(shader);
        }
    }
    chunkSem.release();
}

void ChunkManager::UpdateChunks(const Frustum &frustum)
{
    int count = 0;

    initSem.acquire();

    for (auto it = chunksToInitialize.begin(); it != chunksToInitialize.end() && count < 2;)
    {
        vec3 min((*it)->GetPosition()[0], (*it)->GetPosition()[1], (*it)->GetPosition()[2]);
        vec3 max = vec3(min[0], min[1], min[2]) + vec3((float)(Chunk::ChunkSize()));
        if (frustum.IsBoxInFrustum(min, max))
        {
            (*it)->Initialize();

            chunkSem.acquire();
            chunks.push_back(*it);
            chunkSem.release();

            it = chunksToInitialize.erase(it);
            count++;
        }
        else
        {
            ++it;
        }
    }

    initSem.release();
}

void ChunkManager::generateChunks(vec3 cameraPos)
{
    int x = (int)(cameraPos.x / Chunk::ChunkSize());
    int y = (int)(cameraPos.y / Chunk::ChunkSize());
    int z = (int)(cameraPos.z / Chunk::ChunkSize());

    for (int i = -renderDistance; i <= renderDistance; i++) // X
    {
        for (int j = -renderDistance; j <= renderDistance; j++)
        {
            for (int k = 0; k < MaxHeight; k++) // Z
            {
                int xPos = (x + i) * Chunk::ChunkSize();
                int yPos = (y + k) * Chunk::ChunkSize();
                int zPos = (z + j) * Chunk::ChunkSize();
                vec3 chunkPos(xPos, yPos, zPos);
                float dist = glm::distance(cameraPos, chunkPos);
                createSem.acquire();
                chunksToCreate.emplace(dist, chunkPos);
                createSem.release();
            }
        }
    }
}

#endif