#ifndef GENERATOR_H
#define GENERATOR_H

#include "../Perlin/PerlinNoise.hpp"

namespace Generator
{
    static const siv::PerlinNoise::seed_type seed = 12345u;
    static const int MaxHeight = 4;
    static float frequency = .05f;
    static siv::PerlinNoise Perlin{seed};
    static double Noise2D_01(float x, float z)
    {
        return Perlin.noise2D_01(x * frequency, z * frequency);
    }
    static double Noise3D_01(float x, float y, float z)
    {
        return Perlin.noise3D_01(x * frequency, y * frequency, z * frequency);
    }
}
#endif