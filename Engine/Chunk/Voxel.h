
#pragma once
#ifndef VOXEL_H
#define VOXEL_H

#include <iostream>
using namespace std;

namespace Engine::ChunkSpace::Voxel
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
        JAR
    };
}

#endif