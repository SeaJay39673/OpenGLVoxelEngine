#pragma once
#ifndef CHUNK_H
#define CHUNK_H

#include "VAO.h"
#include "Shader.h"
#include "Texture.h"

#include <vector>

#include <glm/glm.hpp>

using std::vector, glm::vec3, glm::vec2, glm::mat4;

enum class VoxelType
{
    AIR,
    DIRT,
};

class Chunk
{
private:
    const static int chunkSize = 16;
    VAO vao;
    BO *vbo, *ebo;

    int position[3];
    VoxelType voxels[chunkSize][chunkSize][chunkSize];
    void loadChunk();
    void generateVoxel(vector<Vertex> &vertices, int i, int j, int k);

public:
    Chunk(int pos[3]);
    ~Chunk();
    void Render(Shader &shader);
    static int ChunkSize() { return chunkSize; };
    int *Chunk::GetPosition() { return position; }
};

Chunk::Chunk(int pos[3])
{
    memcpy(position, pos, sizeof(position));
    for (int i = 0; i < chunkSize; i++)
        for (int j = 0; j < chunkSize; j++)
            for (int k = 0; k < chunkSize; k++)
                voxels[i][j][k] = rand() % 100 < 98 ? VoxelType::DIRT : VoxelType::AIR;
    loadChunk();
}

Chunk::~Chunk()
{
    delete vbo;
    delete ebo;
}

void Chunk::generateVoxel(vector<Vertex> &vertices, int i, int j, int k)
{
    // Define the 8 corner positions of the cube relative to the voxel's position
    int positions[8][3] = {
        {i + position[0], j + position[1], k + position[2]},             // bottom left
        {i + position[0], j + 1 + position[1], k + position[2]},         // top left
        {i + 1 + position[0], j + 1 + position[1], k + position[2]},     // top right
        {i + 1 + position[0], j + position[1], k + position[2]},         // bottom right
        {i + position[0], j + position[1], k + 1 + position[2]},         // near bottom left
        {i + position[0], j + 1 + position[1], k + 1 + position[2]},     // near top left
        {i + 1 + position[0], j + 1 + position[1], k + 1 + position[2]}, // near top right
        {i + 1 + position[0], j + position[1], k + 1 + position[2]}      // near bottom right
    };

    // Define normals for each face
    int normals[6][3] = {
        {0, 0, -1}, // Front face
        {0, 0, 1},  // Back face
        {-1, 0, 0}, // Left face
        {1, 0, 0},  // Right face
        {0, 1, 0},  // Top face
        {0, -1, 0}  // Bottom face
    };

    // Define texture coordinates for each vertex
    int texCoords[4][2] = {
        {0, 0}, // Bottom left
        {0, 1}, // Top left
        {1, 1}, // Top right
        {1, 0}  // Bottom right
    };

    // Generate vertices for each face of the cube

    // Front face (CCW)
    vertices.push_back(Vertex(positions[0], normals[0], texCoords[0]));
    vertices.push_back(Vertex(positions[1], normals[0], texCoords[1]));
    vertices.push_back(Vertex(positions[2], normals[0], texCoords[2]));
    vertices.push_back(Vertex(positions[0], normals[0], texCoords[0]));
    vertices.push_back(Vertex(positions[2], normals[0], texCoords[2]));
    vertices.push_back(Vertex(positions[3], normals[0], texCoords[3]));

    // Back face (CCW)
    vertices.push_back(Vertex(positions[7], normals[1], texCoords[0]));
    vertices.push_back(Vertex(positions[6], normals[1], texCoords[1]));
    vertices.push_back(Vertex(positions[5], normals[1], texCoords[2]));
    vertices.push_back(Vertex(positions[7], normals[1], texCoords[0]));
    vertices.push_back(Vertex(positions[5], normals[1], texCoords[2]));
    vertices.push_back(Vertex(positions[4], normals[1], texCoords[3]));

    // Left face (CCW)
    vertices.push_back(Vertex(positions[4], normals[2], texCoords[0]));
    vertices.push_back(Vertex(positions[5], normals[2], texCoords[1]));
    vertices.push_back(Vertex(positions[1], normals[2], texCoords[2]));
    vertices.push_back(Vertex(positions[4], normals[2], texCoords[0]));
    vertices.push_back(Vertex(positions[1], normals[2], texCoords[2]));
    vertices.push_back(Vertex(positions[0], normals[2], texCoords[3]));

    // Right face (CCW)
    vertices.push_back(Vertex(positions[3], normals[3], texCoords[0]));
    vertices.push_back(Vertex(positions[2], normals[3], texCoords[1]));
    vertices.push_back(Vertex(positions[6], normals[3], texCoords[2]));
    vertices.push_back(Vertex(positions[3], normals[3], texCoords[0]));
    vertices.push_back(Vertex(positions[6], normals[3], texCoords[2]));
    vertices.push_back(Vertex(positions[7], normals[3], texCoords[3]));

    // Top face (CCW)
    vertices.push_back(Vertex(positions[1], normals[4], texCoords[0]));
    vertices.push_back(Vertex(positions[5], normals[4], texCoords[1]));
    vertices.push_back(Vertex(positions[6], normals[4], texCoords[2]));
    vertices.push_back(Vertex(positions[1], normals[4], texCoords[0]));
    vertices.push_back(Vertex(positions[6], normals[4], texCoords[2]));
    vertices.push_back(Vertex(positions[2], normals[4], texCoords[3]));

    // Bottom face (CCW) - FIXED
    vertices.push_back(Vertex(positions[4], normals[5], texCoords[0]));
    vertices.push_back(Vertex(positions[0], normals[5], texCoords[1]));
    vertices.push_back(Vertex(positions[3], normals[5], texCoords[2]));
    vertices.push_back(Vertex(positions[4], normals[5], texCoords[0]));
    vertices.push_back(Vertex(positions[3], normals[5], texCoords[2]));
    vertices.push_back(Vertex(positions[7], normals[5], texCoords[3]));
}

void Chunk::loadChunk()
{
    vector<Vertex> vertices;

    unsigned int offset = 0;
    for (int i = 0; i < chunkSize; i++)
    {
        for (int j = 0; j < chunkSize; j++)
        {
            for (int k = 0; k < chunkSize; k++)
            {
                if (voxels[i][j][k] == VoxelType::DIRT)
                {
                    if (!i || !j || !k || i == chunkSize - 1 || j == chunkSize - 1 || k == chunkSize - 1)
                    {
                        generateVoxel(vertices, i, j, k);
                        continue;
                    }
                    if (voxels[i - 1][j][k] == VoxelType::AIR ||
                        i + 1 < chunkSize && voxels[i + 1][j][k] == VoxelType::AIR ||
                        voxels[i][j - 1][k] == VoxelType::AIR ||
                        j + 1 < chunkSize && voxels[i][j + 1][k] == VoxelType::AIR ||
                        voxels[i][j][k - 1] == VoxelType::AIR ||
                        k + 1 < chunkSize && voxels[i][j][k + 1] == VoxelType::AIR)
                    {
                        generateVoxel(vertices, i, j, k);
                    }
                }
            }
        }
    }

    vao.Bind();
    vbo = new BO(vertices);
    // ebo = new BO(indices);
    vao.LinkBO(*vbo);
    vbo->Unbind();
    vao.Unbind();
    // ebo->Unbind();
}

void Chunk::Render(Shader &shader)
{
    shader.Use();
    vao.Bind();
    Texture::GetTexture(VoxelType::DIRT).Bind();
    shader.SetInt("texture1", 0); // Set the sampler to use texture unit 0
    glDrawArrays(GL_TRIANGLES, 0, vbo->GetCount());
    Texture::GetTexture(VoxelType::DIRT).Unbind();
    vao.Unbind();
}

#endif