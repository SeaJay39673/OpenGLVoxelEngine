#ifndef TEXTURE_H
#define TEXTURE_H

#include <iostream>
#include <unordered_map>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "Chunk.h"

using std::string, std::unordered_map;

enum class VoxelType;

class Texture
{
private:
    struct texture
    {
        texture()
        {
            glGenTextures(1, &id);
        }
        unsigned int id;
        int width, height, channels;
        string path;
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

public:
    static void RegisterTexture(VoxelType type, const string &path)
    {
        texture tex;
        tex.path = path;
        if (loadTexture(tex))
            textures[type] = tex;
    }
    static texture &GetTexture(VoxelType type)
    {
        return textures[type];
    }
};

unordered_map<VoxelType, Texture::texture> Texture::textures;

bool Texture::loadTexture(texture &tex)
{
    stbi_set_flip_vertically_on_load(true);
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

#endif