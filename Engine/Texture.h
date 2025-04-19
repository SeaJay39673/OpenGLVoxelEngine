#pragma once
#ifndef TEXTURE_H
#define TEXTURE_H

#include <iostream>
#include <unordered_map>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "Chunk.h"
#include "Voxel.h"

using std::string, std::unordered_map, Voxel::VoxelType;

class Texture
{
private:
    struct texture
    {
        texture(const int &id) : id(id) {}
        texture()
        {
            glGenTextures(1, &id);
        }
        unsigned int id;
        int width, height, channels;
        string path;
        float texCoords[4][2];
        void Bind()
        {
            glBindTexture(GL_TEXTURE_2D, id);
        };
        void Unbind()
        {
            glBindTexture(GL_TEXTURE_2D, 0);
        };
    };
    static unordered_map<VoxelType, texture> textures;
    static bool loadTexture(texture &tex);
    static void calcTexCoordds(texture &tex, int numTex, int padding, int x, int y);

public:
    static void RegisterTexture(VoxelType type, const string &path, int numTex, int padding, int x, int y)
    {
        for (auto &pair : textures)
        {
            if (pair.second.path == path)
            {
                textures[type] = texture(pair.second.id);
                calcTexCoordds(textures[type], numTex, padding, x, y);
                return;
            }
        }
        texture tex;
        tex.path = path;
        if (loadTexture(tex))
            textures[type] = tex;
        calcTexCoordds(textures[type], numTex, padding, x, y);
    }
    static texture &GetTexture(VoxelType type)
    {
        return textures[type];
    }
    static void InitializeTextures()
    {
        Texture::RegisterTexture(VoxelType::BRICK, "./Resources/Textures/brick.jpg", 1, 0, 0, 0);
        Texture::RegisterTexture(VoxelType::TREE, "./Resources/Textures/Textures.png", 4, 4, 2, 1);
        Texture::RegisterTexture(VoxelType::POTION, "./Resources/Textures/Textures.png", 4, 4, 1, 2);
        Texture::RegisterTexture(VoxelType::SWORD, "./Resources/Textures/Textures.png", 4, 4, 0, 2);
        Texture::RegisterTexture(VoxelType::HEART, "./Resources/Textures/Textures.png", 4, 4, 1, 1);
        Texture::RegisterTexture(VoxelType::JAR, "./Resources/Textures/Textures.png", 4, 4, 2, 0);
    }
};

unordered_map<VoxelType, Texture::texture> Texture::textures;

bool Texture::loadTexture(texture &tex)
{
    unsigned char *data = stbi_load(tex.path.c_str(), &tex.width, &tex.height, &tex.channels, 0);
    if (!data)
    {
        std::cerr << "Failed to load texture: " << tex.path << std::endl;
        return false;
    }
    GLenum format = tex.channels == 1 ? GL_RED : tex.channels == 3 ? GL_RGB
                                             : tex.channels == 4   ? GL_RGBA
                                                                   : 0;

    if (!format)
    {
        std::cerr << "Unsupported number of channels: " << tex.channels << std::endl;
        stbi_image_free(data);
        return false;
    }

    tex.Bind();

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, format, tex.width, tex.height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);
    tex.Unbind();
    return true;
}

void Texture::calcTexCoordds(texture &tex, int numTex, int padding, int x, int y)
{

    float texWidth = static_cast<float>(tex.width);
    float texHeight = static_cast<float>(tex.height);

    // Calculate the size of each texture cell (including padding)
    float cellWidth = (texWidth - (numTex + 1) * padding) / numTex;
    float cellHeight = (texHeight - (numTex + 1) * padding) / numTex;

    // Calculate the pixel coordinates of the cell
    float x0 = padding + x * (cellWidth + padding);
    float y0 = padding + y * (cellHeight + padding);
    float x1 = x0 + cellWidth;
    float y1 = y0 + cellHeight;

    // Convert to normalized texture coordinates [0,1]
    float u0 = x0 / texWidth;
    float v0 = y0 / texHeight;
    float u1 = x1 / texWidth;
    float v1 = y1 / texHeight;

    float bottomLeft[2];
    float bottomRight[2];
    float topLeft[2];
    float topRight[2];

    // OpenGL's texture coordinates: (0,0) is bottom-left
    bottomLeft[0] = u0;
    bottomLeft[1] = v1;
    bottomRight[0] = u1;
    bottomRight[1] = v1;
    topLeft[0] = u0;
    topLeft[1] = v0;
    topRight[0] = u1;
    topRight[1] = v0;

    memcpy(tex.texCoords[0], bottomLeft, sizeof(float) * 2);
    memcpy(tex.texCoords[1], topLeft, sizeof(float) * 2);
    memcpy(tex.texCoords[2], topRight, sizeof(float) * 2);
    memcpy(tex.texCoords[3], bottomRight, sizeof(float) * 2);
}

#endif