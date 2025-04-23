#ifndef GENERATOR_H
#define GENERATOR_H

#include "../../Perlin/PerlinNoise.hpp"

namespace Engine::Utility
{
    class Generator
    {
    private:
        static siv::PerlinNoise::seed_type seed;
        static float frequency;
        static siv::PerlinNoise perlin;

    public:
        void SetFrequency(float freq)
        {
            frequency = freq;
        }
        void SetSeed(siv::PerlinNoise::seed_type s)
        {
            seed = s;
            perlin = siv::PerlinNoise{seed};
        }
        static double Noise2D_01(float x, float z)
        {
            return perlin.noise2D_01(x * frequency, z * frequency);
        }
        static double Noise3D_01(float x, float y, float z)
        {
            return perlin.noise3D_01(x * frequency, y * frequency, z * frequency);
        }
    };
    float Generator::frequency = .01f;
    siv::PerlinNoise::seed_type Generator::seed = 12345u;
    siv::PerlinNoise Generator::perlin{seed};
};

#endif