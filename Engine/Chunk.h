#pragma once
#ifndef CHUNK_H
#define CHUNK_H

#include "VAO.h"
#include "Shader.h"

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
    void loadSquare();

public:
    Chunk(int pos[3]);
    void Render(Shader &shader);
};

Chunk::Chunk(int pos[3])
{
    memcpy(position, pos, sizeof(position));
    for (int i = 0; i < chunkSize; i++)
        for (int j = 0; j < chunkSize; j++)
            for (int k = 0; k < chunkSize; k++)
                voxels[i][j][k] = VoxelType::DIRT;
    loadChunk();
    // vector<int> vertices;
    // vector<unsigned int> indices;
    // vector<int> vertices = {
    //     1, 1, -10,   // top right
    //     1, -1, -10,  // bottom right
    //     -1, -1, -10, // bottom left
    //     -1, 1, -10   // top left
    // };
    // vector<unsigned int> indices = {
    //     // note that we start from 0!
    //     0, 1, 3, // first Triangle
    //     1, 2, 3  // second Triangle
    // };

    // vao.Bind();
    // vbo = new BO(vertices);
    // ebo = new BO(indices);
    // vao.LinkBO(*vbo);
    // vbo->Unbind();
    // vao.Unbind();
    // ebo->Unbind();
}

void Chunk::loadSquare()
{
    vector<int> vertices = {
        1, 1, -10,   // top right
        1, -1, -10,  // bottom right
        -1, -1, -10, // bottom left
        -1, 1, -10   // top left
    };
    vector<unsigned int> indices = {
        // note that we start from 0!
        0, 1, 3, // first Triangle
        1, 2, 3  // second Triangle
    };

    vao.Bind();
    vbo = new BO(vertices);
    ebo = new BO(indices);
    vao.LinkBO(*vbo);
    vbo->Unbind();
    vao.Unbind();
    ebo->Unbind();
}

void Chunk::loadChunk()
{
    vector<int> vertices;
    vector<unsigned int> indices;

    unsigned int offset = 0;
    for (int i = 0; i < chunkSize; i++)
    {
        for (int j = 0; j < chunkSize; j++)
        {
            for (int k = 0; k < chunkSize; k++)
            {
                if (voxels[i][j][k] != VoxelType::AIR)
                {
                    vertices.insert(
                        vertices.end(),
                        {
                            i + position[0], j + position[1], k + position[2],             // bottom left
                            i + position[0], j + 1 + position[1], k + position[2],         // top left
                            i + 1 + position[0], j + 1 + position[1], k + position[2],     // top right
                            i + 1 + position[0], j + position[1], k + position[2],         // bottom right
                            i + position[0], j + position[1], k + 1 + position[2],         // Near Bottom Left
                            i + position[0], j + 1 + position[1], k + 1 + position[2],     // Near Top Left
                            i + 1 + position[0], j + 1 + position[1], k + 1 + position[2], // Near Top Right
                            i + 1 + position[0], j + position[1], k + 1 + position[2],     // Near Bottom Right
                        });
                    // Front face
                    indices.insert(indices.end(), {offset + 0, offset + 1, offset + 2,
                                                   offset + 0, offset + 2, offset + 3});

                    // Back face
                    indices.insert(indices.end(), {offset + 4, offset + 5, offset + 6,
                                                   offset + 4, offset + 6, offset + 7});

                    // Left face
                    indices.insert(indices.end(), {offset + 0, offset + 4, offset + 7,
                                                   offset + 0, offset + 7, offset + 3});

                    // Right face
                    indices.insert(indices.end(), {offset + 1, offset + 5, offset + 6,
                                                   offset + 1, offset + 6, offset + 2});

                    // Top face
                    indices.insert(indices.end(), {offset + 1, offset + 5, offset + 4,
                                                   offset + 1, offset + 4, offset + 0});

                    // Bottom face
                    indices.insert(indices.end(), {offset + 3, offset + 7, offset + 6,
                                                   offset + 3, offset + 6, offset + 2});

                    offset += 8; // Each cube adds 8 vertices
                }
            }
        }
    }

    // for (int i = 0; i < chunkSize; i++)
    // {
    //     if (voxels[i][0][0] != VoxelType::AIR)
    //     {
    //         vertices.insert(vertices.end(), {
    //                                             1 + i, 1, -10,   // top right
    //                                             1 + i, -1, -10,  // bottom right
    //                                             -1 + i, -1, -10, // bottom left
    //                                             -1 + i, 1, -10   // top left
    //                                         });

    //         unsigned int offset = i * 6;
    //         indices.insert(indices.end(), {
    //                                           offset + 0, offset + 1, offset + 3, // first Triangle
    //                                           offset + 1, offset + 2, offset + 3  // second Triangle
    //                                       });
    //     }
    // }

    vao.Bind();
    vbo = new BO(vertices);
    ebo = new BO(indices);
    vao.LinkBO(*vbo);
    vbo->Unbind();
    vao.Unbind();
    ebo->Unbind();
}

void Chunk::Render(Shader &shader)
{
    shader.Use();
    vao.Bind();
    glDrawElements(GL_TRIANGLES, ebo->GetCount(), GL_UNSIGNED_INT, 0);
    vao.Unbind();
}

#endif