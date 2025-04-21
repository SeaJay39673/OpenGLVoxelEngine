
#pragma once
#ifndef VOXEL_H
#define VOXEL_H

#include <iostream>
using namespace std;

namespace Voxel
{
    enum class VoxelType
    {
        AIR,
        BRICK,
        BRICK_RED,
        BLOCK,
        EYE,
        BLOOD,
        WOOD,
        WATER,
        TREE,
        POTION,
        SWORD,
        HEART,
        JAR,

        COUNT // Add COUNT as the last element
    };
    constexpr int VoxelTypeCount = static_cast<int>(VoxelType::COUNT);
    VoxelType GetRandomVoxel()
    {
        return (VoxelType)(rand() % VoxelTypeCount);
    }
}

#endif