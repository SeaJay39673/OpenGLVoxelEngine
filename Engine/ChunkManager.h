#ifndef CHUNKMANAGER_H
#define CHUNKMANAGER_H

#include "Chunk.h"
#include "Texture.h"
#include "Frustum.h"

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
        Texture::RegisterTexture(VoxelType::DIRT, "./Resources/Textures/brick.jpg");
        ChunkManager::renderDistance = renderDistance;
        chunks.reserve(renderDistance * renderDistance * renderDistance);
        generateChunks(cameraPos);
    }

    static void SetRenderDistance(int distance) { renderDistance = distance; }
    static int GetRenderDistance() { return renderDistance; }
    static void RenderChunks(Shader &shader, const Frustum &frustum);
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

void ChunkManager::RenderChunks(Shader &shader, const Frustum &frustum)
{
    for (Chunk *chunk : chunks)
    {
        // Calculate the bounding box of the chunk
        vec3 min(chunk->GetPosition()[0], chunk->GetPosition()[1], chunk->GetPosition()[2]);
        vec3 max = min + vec3(Chunk::ChunkSize());

        // Check if the chunk is in the frustum
        if (frustum.IsBoxInFrustum(min, max))
        {
            chunk->Render(shader);
        }
    }
}

#endif