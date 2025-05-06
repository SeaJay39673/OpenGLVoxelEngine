#ifndef GENERATOR_H
#define GENERATOR_H

#include "../../Perlin/PerlinNoise.hpp"

namespace Engine::Utility
{
    /**
     * @brief Generator class for generating Perlin noise.
     * @details This class uses a Perlin noise library to generate 2D and 3D noise values.
     */
    class Generator
    {
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

    private:
        static siv::PerlinNoise::seed_type seed;
        static float frequency;
        static siv::PerlinNoise perlin;
    };
    float Generator::frequency = .01f;
    siv::PerlinNoise::seed_type Generator::seed = 12345u;
    siv::PerlinNoise Generator::perlin{seed};
};

#endif