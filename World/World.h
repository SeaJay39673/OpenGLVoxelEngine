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

#include <thread>
#include <chrono>
#include <atomic>

using std::string, std::thread;

using namespace std::chrono;

class World : public Game
{
private:
    atomic<bool> threadShouldClose;
    Camera camera;
    Shader shader;
    Frustum frustum;

public:
    World() : shader("./Resources/Shaders/basic.vert", "./Resources/Shaders/basic.frag"), camera(&shader)
    {
        threadShouldClose.store(false);
        type = GameType::CONCURRENT;
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
    {
        threadShouldClose.store(true);
        _app->Shutdown();
    }
    Mouse::delx = 0;
    Mouse::dely = 0;
};

void World::Update()
{
    frustum.Update(shader.GetProjection(), camera.GetCameraView());
    ChunkManager::UpdateChunks(frustum);
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
                                        shader.UpdatePerspective(_app->GetWidth(), _app->GetHeight(), ChunkManager::GetRenderDistance() * Chunk::ChunkSize() - Chunk::ChunkSize());
                                    });
    shader.UpdatePerspective(_app->GetWidth(), _app->GetHeight(), ChunkManager::GetRenderDistance() * Chunk::ChunkSize() - Chunk::ChunkSize());
    _app->DisableCursor();

    thread update(
        [this]()
        {
            while (!threadShouldClose.load())
            {
                Update();
                std::this_thread::sleep_for(std::chrono::milliseconds(60));
            }
        });
    update.detach();
};

#endif