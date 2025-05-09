#ifndef MESH_H
#define MESH_H

#include "VAO.h"
#include "../Shader.h"
#include "../Texture.h"
#include "../Chunk/Voxel.h"

#include <glm/glm.hpp>

using glm::ivec3;

using namespace Engine::ChunkSpace::Voxel;

namespace Engine::MeshSpace
{
    enum class Face
    {
        LEFT,
        RIGHT,
        TOP,
        BOTTOM,
        FRONT,
        BACK
    };

    /**
     * @brief A class for handling mesh data and rendering in OpenGL.
     * @note Create this class in the main thread.
     */
    class Mesh
    {
    public:
        Mesh(VoxelType type, ivec3 pos) : position(pos), type(type) {}
        ~Mesh()
        {
            delete vbo;
            delete ebo;
        }
        unsigned int Vertices() const
        {
            if (vbo != nullptr)
                return vbo->GetCount();
            return -1;
        }
        void GenerateFace(int i, int j, int k, Face face);
        void GenerateVoxel(int i, int j, int k);
        void CreateMesh();
        void Render(Shader &shader);

    private:
        VoxelType type;
        VAO vao;
        BO *vbo = nullptr, *ebo = nullptr;
        ivec3 position;
        vector<Vertex> vertices;
        vector<unsigned int> indices;
    };

    /**
     * @brief Generate the vertices and indices for a face of the voxel.
     * @details This function generates the vertices and indices for a specific face of the voxel based on its position and the specified face.
     * @note Run this function for either the main thread or the update thread.
     *
     * @param i The x-coordinate of the voxel.
     * @param j The y-coordinate of the voxel.
     * @param k The z-coordinate of the voxel.
     * @param face The face to generate (BACK, FRONT, LEFT, RIGHT, TOP, BOTTOM).
     */
    void Mesh::GenerateFace(int i, int j, int k, Face face)
    {
        unsigned int offset = (int)vertices.size();
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
        float texCoords[4][2];
        memcpy(texCoords, Texture::GetTexture(type).texCoords, sizeof(texCoords));

        // Generate vertices for each face of the cube
        switch (face)
        {
        case Face::BACK:
            // Back face (CCW)
            vertices.push_back(Vertex(positions[0], normals[0], texCoords[0]));
            vertices.push_back(Vertex(positions[1], normals[0], texCoords[1]));
            vertices.push_back(Vertex(positions[2], normals[0], texCoords[2]));
            // vertices.push_back(Vertex(positions[0], normals[0], texCoords[0]));
            // vertices.push_back(Vertex(positions[2], normals[0], texCoords[2]));
            vertices.push_back(Vertex(positions[3], normals[0], texCoords[3]));

            break;
        case Face::FRONT:
            // Front face (CCW)
            vertices.push_back(Vertex(positions[7], normals[1], texCoords[0]));
            vertices.push_back(Vertex(positions[6], normals[1], texCoords[1]));
            vertices.push_back(Vertex(positions[5], normals[1], texCoords[2]));
            // vertices.push_back(Vertex(positions[7], normals[1], texCoords[0]));
            // vertices.push_back(Vertex(positions[5], normals[1], texCoords[2]));
            vertices.push_back(Vertex(positions[4], normals[1], texCoords[3]));

            break;
        case Face::LEFT:
            // Left face (CCW)
            vertices.push_back(Vertex(positions[4], normals[2], texCoords[0]));
            vertices.push_back(Vertex(positions[5], normals[2], texCoords[1]));
            vertices.push_back(Vertex(positions[1], normals[2], texCoords[2]));
            // vertices.push_back(Vertex(positions[4], normals[2], texCoords[0]));
            // vertices.push_back(Vertex(positions[1], normals[2], texCoords[2]));
            vertices.push_back(Vertex(positions[0], normals[2], texCoords[3]));

            break;
        case Face::RIGHT:
            // Right face (CCW)
            vertices.push_back(Vertex(positions[3], normals[3], texCoords[0]));
            vertices.push_back(Vertex(positions[2], normals[3], texCoords[1]));
            vertices.push_back(Vertex(positions[6], normals[3], texCoords[2]));
            // vertices.push_back(Vertex(positions[3], normals[3], texCoords[0]));
            // vertices.push_back(Vertex(positions[6], normals[3], texCoords[2]));
            vertices.push_back(Vertex(positions[7], normals[3], texCoords[3]));

            break;
        case Face::TOP:
            // Top face (CCW)
            vertices.push_back(Vertex(positions[1], normals[4], texCoords[0]));
            vertices.push_back(Vertex(positions[5], normals[4], texCoords[1]));
            vertices.push_back(Vertex(positions[6], normals[4], texCoords[2]));
            // vertices.push_back(Vertex(positions[1], normals[4], texCoords[0]));
            // vertices.push_back(Vertex(positions[6], normals[4], texCoords[2]));
            vertices.push_back(Vertex(positions[2], normals[4], texCoords[3]));

            break;
        case Face::BOTTOM:
            // Bottom face (CCW) - FIXED
            vertices.push_back(Vertex(positions[4], normals[5], texCoords[0]));
            vertices.push_back(Vertex(positions[0], normals[5], texCoords[1]));
            vertices.push_back(Vertex(positions[3], normals[5], texCoords[2]));
            // vertices.push_back(Vertex(positions[4], normals[5], texCoords[0]));
            // vertices.push_back(Vertex(positions[3], normals[5], texCoords[2]));
            vertices.push_back(Vertex(positions[7], normals[5], texCoords[3]));

            break;
        }

        indices.insert(indices.begin(), {0 + offset, 1 + offset, 2 + offset, 0 + offset, 2 + offset, 3 + offset});
    }

    /**
     * @brief Generate the vertices and indices for a voxel by generating all six faces.
     * @note Run this function for either the main thread or the update thread.
     *
     * @param i The x-coordinate of the voxel.
     * @param j The y-coordinate of the voxel.
     * @param k The z-coordinate of the voxel.
     */
    void Mesh::GenerateVoxel(int i, int j, int k)
    {
        GenerateFace(i, j, k, Face::BACK);
        GenerateFace(i, j, k, Face::FRONT);
        GenerateFace(i, j, k, Face::TOP);
        GenerateFace(i, j, k, Face::BOTTOM);
        GenerateFace(i, j, k, Face::LEFT);
        GenerateFace(i, j, k, Face::RIGHT);
    }

    /**
     * @brief Create the mesh by binding the vertex array object and buffer objects.
     * @note Since this function calls OpenGL functions, it should be run in the main thread.
     *
     */
    void Mesh::CreateMesh()
    {
        if (vertices.empty())
        {
            return;
        }
        vao.Bind();
        vbo = new BO(vertices);
        if (indices.size())
            ebo = new BO(indices);
        vao.LinkBO(*vbo);
        vbo->Unbind();
        vao.Unbind();
        if (ebo != nullptr)
            ebo->Unbind();
        vertices.clear();
        indices.clear();
    }

    /**
     * @brief Render the mesh using the provided shader.
     * @note Since this function calls OpenGL functions, it should be run in the main thread.
     *
     * @param shader The shader to use for rendering.
     */
    void Mesh::Render(Shader &shader)
    {
        if (vbo == nullptr)
        {
            return;
        }
        if (type == VoxelType::WATER)
            shader.SetFloat("TexOpacity", .5);
        else
            shader.SetFloat("TexOpacity", 1);
        shader.Use();
        vao.Bind();
        Texture::GetTexture(type).Bind();
        shader.SetInt("texture1", 0); // Set the sampler to use texture unit 0
        if (ebo != nullptr)
            glDrawElements(GL_TRIANGLES, ebo->GetCount(), GL_UNSIGNED_INT, 0);
        else
            glDrawArrays(GL_TRIANGLES, 0, vbo->GetCount());
        Texture::GetTexture(type).Unbind();
        vao.Unbind();
    }
};

#endif