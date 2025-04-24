#ifndef CONFIG_H
#define CONFIG_H

namespace Engine::ChunkSpace
{
    class Config
    {
    private:
        static int renderDistance;
        static int maxHeight;
        static int chunkSize;

    public:
        static void SetRenderDistance(int distance)
        {
            renderDistance = distance;
        }
        static int const GetRenderDistance()
        {
            return renderDistance;
        }
        static void SetMaxHeight(int height)
        {
            maxHeight = height;
        }
        static int const GetMaxHeight()
        {
            return maxHeight;
        }
        static void SetChunkSize(int size)
        {
            chunkSize = size;
        }
        static int const GetChunkSize()
        {
            return chunkSize;
        }
    };
};

int Engine::ChunkSpace::Config::renderDistance = 4;
int Engine::ChunkSpace::Config::maxHeight = 8;
int Engine::ChunkSpace::Config::chunkSize = 16;

#endif