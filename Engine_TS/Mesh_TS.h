#ifndef MESH_TS_H
#define MESH_TS_H

#include "../Engine/VAO.h"
#include "Shader_TS.h"
#include "../Engine/Texture.h"

#include <mutex>

using std::mutex, std::lock_guard;

using namespace Voxel;

enum class Mesh_TS_Faces
{
    LEFT,
    RIGHT,
    TOP,
    BOTTOM,
    FRONT,
    BACK
};

class Mesh_TS
{
private:
    VoxelType type;
    VAO *vao = nullptr;
    BO *vbo = nullptr, *ebo = nullptr;
    int position[3];
    vector<Vertex> vertices;
    mutex loadedMutex;
    bool loaded;
    bool created;

public:
    Mesh_TS(VoxelType type, int pos[3]) : type(type), loaded(false), created(false)
    {
        memcpy(position, pos, sizeof(position));
    }
    ~Mesh_TS()
    {
        delete vao;
        delete vbo;
        delete ebo;
    }

    void Reset()
    {
        lock_guard<mutex> lock(loadedMutex);
        loaded = false;
        created = false;
        delete vao;
        delete vbo;
        delete ebo;
        vao = nullptr;
        vbo = nullptr;
        ebo = nullptr;
    }

    void Done()
    {
        lock_guard<mutex> lock(loadedMutex);
        loaded = true;
    }

    bool IsDone()
    {
        lock_guard<mutex> lock(loadedMutex);
        return loaded;
    }

    void CreateMesh()
    {
        cout << "Creating Mesh\n";
        if (vertices.empty())
        {
            std::cerr << "Vertices Empty, Can't create mesh\n";
            return;
        }
        vao = new VAO;
        vao->Bind();
        vbo = new BO(vertices);
        // ebo = new BO(indices);
        vao->LinkBO(*vbo);
        vbo->Unbind();
        vao->Unbind();
        // ebo->Unbind();
        created = true;
        cout << "Mesh Created\n";
    }

    void Render(Shader_TS &shader)
    {
        if (!IsDone())
            return;
        if (!created)
            CreateMesh();
        shader.Use();
        vao->Bind();
        Texture::GetTexture(type).Bind();
        shader.SetInt("texture1", 0); // Set the sampler to use texture unit 0
        glDrawArrays(GL_TRIANGLES, 0, vbo->GetCount());
        Texture::GetTexture(type).Unbind();
        vao->Unbind();
    }

    void GenerateFace(int i, int j, int k, Mesh_TS_Faces face)
    {

        if (vao != nullptr)
        {
            Reset();
        }
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

        switch (face)
        {
        case Mesh_TS_Faces::FRONT:
            // Front face (CCW)
            vertices.push_back(Vertex(positions[0], normals[0], texCoords[0]));
            vertices.push_back(Vertex(positions[1], normals[0], texCoords[1]));
            vertices.push_back(Vertex(positions[2], normals[0], texCoords[2]));
            vertices.push_back(Vertex(positions[0], normals[0], texCoords[0]));
            vertices.push_back(Vertex(positions[2], normals[0], texCoords[2]));
            vertices.push_back(Vertex(positions[3], normals[0], texCoords[3]));
        case Mesh_TS_Faces::BACK:
            // Back face (CCW)
            vertices.push_back(Vertex(positions[7], normals[1], texCoords[0]));
            vertices.push_back(Vertex(positions[6], normals[1], texCoords[1]));
            vertices.push_back(Vertex(positions[5], normals[1], texCoords[2]));
            vertices.push_back(Vertex(positions[7], normals[1], texCoords[0]));
            vertices.push_back(Vertex(positions[5], normals[1], texCoords[2]));
            vertices.push_back(Vertex(positions[4], normals[1], texCoords[3]));
        case Mesh_TS_Faces::LEFT:
            // Left face (CCW)
            vertices.push_back(Vertex(positions[4], normals[2], texCoords[0]));
            vertices.push_back(Vertex(positions[5], normals[2], texCoords[1]));
            vertices.push_back(Vertex(positions[1], normals[2], texCoords[2]));
            vertices.push_back(Vertex(positions[4], normals[2], texCoords[0]));
            vertices.push_back(Vertex(positions[1], normals[2], texCoords[2]));
            vertices.push_back(Vertex(positions[0], normals[2], texCoords[3]));
        case Mesh_TS_Faces::RIGHT:
            // Right face (CCW)
            vertices.push_back(Vertex(positions[3], normals[3], texCoords[0]));
            vertices.push_back(Vertex(positions[2], normals[3], texCoords[1]));
            vertices.push_back(Vertex(positions[6], normals[3], texCoords[2]));
            vertices.push_back(Vertex(positions[3], normals[3], texCoords[0]));
            vertices.push_back(Vertex(positions[6], normals[3], texCoords[2]));
            vertices.push_back(Vertex(positions[7], normals[3], texCoords[3]));
        case Mesh_TS_Faces::TOP:
            // Top face (CCW)
            vertices.push_back(Vertex(positions[1], normals[4], texCoords[0]));
            vertices.push_back(Vertex(positions[5], normals[4], texCoords[1]));
            vertices.push_back(Vertex(positions[6], normals[4], texCoords[2]));
            vertices.push_back(Vertex(positions[1], normals[4], texCoords[0]));
            vertices.push_back(Vertex(positions[6], normals[4], texCoords[2]));
            vertices.push_back(Vertex(positions[2], normals[4], texCoords[3]));
        case Mesh_TS_Faces::BOTTOM:
            // Bottom face (CCW) - FIXED
            vertices.push_back(Vertex(positions[4], normals[5], texCoords[0]));
            vertices.push_back(Vertex(positions[0], normals[5], texCoords[1]));
            vertices.push_back(Vertex(positions[3], normals[5], texCoords[2]));
            vertices.push_back(Vertex(positions[4], normals[5], texCoords[0]));
            vertices.push_back(Vertex(positions[3], normals[5], texCoords[2]));
            vertices.push_back(Vertex(positions[7], normals[5], texCoords[3]));
        }
    }

    void GenerateVoxel(int i, int j, int k)
    {
        GenerateFace(i, j, k, Mesh_TS_Faces::LEFT);
        GenerateFace(i, j, k, Mesh_TS_Faces::RIGHT);
        GenerateFace(i, j, k, Mesh_TS_Faces::TOP);
        GenerateFace(i, j, k, Mesh_TS_Faces::BOTTOM);
        GenerateFace(i, j, k, Mesh_TS_Faces::FRONT);
        GenerateFace(i, j, k, Mesh_TS_Faces::BACK);
    }
};

#endif