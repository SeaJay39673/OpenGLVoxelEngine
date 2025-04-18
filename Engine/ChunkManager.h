#ifndef CHUNKMANAGER_H
#define CHUNKMANAGER_H

#include "Chunk.h"

using glm::vec4;

class ChunkManager
{
private:
    static int renderDistance;
    static vector<Chunk *> chunks;
    static void generateChunks(vec3 cameraPos);

public:
    static void InitChunkManager(vec3 cameraPos, int renderDistance = 5)
    {
        ChunkManager::renderDistance = renderDistance;
        chunks.reserve(renderDistance * renderDistance * renderDistance);
        generateChunks(cameraPos);
    }

    static void SetRenderDistance(int distance) { renderDistance = distance; }
    static int GetRenderDistance() { return renderDistance; }
    static void RenderChunks(Shader &shader);
};

int ChunkManager::renderDistance = 0;
vector<Chunk *> ChunkManager::chunks;

void ChunkManager::generateChunks(vec3 cameraPos)
{
    int x = cameraPos.x / Chunk::ChunkSize();
    int y = cameraPos.y / Chunk::ChunkSize();
    int z = cameraPos.z / Chunk::ChunkSize();

    for (int i = -renderDistance; i <= renderDistance; i++)
    {
        for (int j = -renderDistance; j <= renderDistance; j++)
        {
            for (int k = -renderDistance; k <= renderDistance; k++)
            {
                int pos[3] = {(x + i) * Chunk::ChunkSize(), (y + j) * Chunk::ChunkSize(), (z + k) * Chunk::ChunkSize()};
                if (rand() % 100 < 50)
                    chunks.push_back(new Chunk(pos));
            }
        }
    }
}

void ChunkManager::RenderChunks(Shader &shader)
{
    for (Chunk *chunk : chunks)
        chunk->Render(shader);
}

#endif