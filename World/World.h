#pragma once
#ifndef WORLD_H
#define WORLD_H

#include "../Game/Game.h"
#include "../Engine/ChunkManager.h"
#include "../Engine/Shader.h"
#include "../Engine/Camera.h"
#include "../Engine/Frustum.h"
#include "../IO/Mouse.h"
#include "../IO/Keyboard.h"

using std::string;

class World : Game
{
private:
    mat4 *projection;
    Camera camera;
    Shader shader;
    Frustum frustum;

public:
    World() : shader("./Resources/Shaders/basic.vert", "./Resources/Shaders/basic.frag"), camera(&shader)
    {
        ChunkManager::InitChunkManager(camera.GetCameraPos(), 8);
    };

    void Update() override;
    void ProcessInput() override;
    void Render() override;
    void Start() override;
};

void World::ProcessInput()
{
    camera.ProcessInput();
    if (Keyboard::keys[GLFW_KEY_R])
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    if (Keyboard::keys[GLFW_KEY_F])
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    if (Keyboard::keys[GLFW_KEY_ESCAPE])
        _app->Shutdown();
    Mouse::delx = 0;
    Mouse::dely = 0;
};

void World::Update()
{
    frustum.Update(shader.GetProjection(), camera.GetCameraView());
    ChunkManager::UpdateChunks(frustum, camera.GetCameraPos());
}

void World::Render()
{
    shader.Use();
    ChunkManager::RenderChunks(shader, frustum);
};

void World::Start()
{
    _app->RegisterFrameSizeCallback("ViewPort",
                                    [this](int width, int height)
                                    {
                                        glViewport(0, 0, width, height);
                                        shader.UpdatePerspective(_app->GetWidth(), _app->GetHeight());
                                    });
    shader.UpdatePerspective(_app->GetWidth(), _app->GetHeight());
    _app->DisableCursor();
};

#endif