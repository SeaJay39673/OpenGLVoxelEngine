#pragma once
#ifndef WORLD_H
#define WORLD_H

#include "../Game/Game.h"
#include "IO/IO.h"
#include "../Engine/Engine.h"

#include <thread>
#include <chrono>
#include <atomic>

using std::string, std::thread, Engine::Camera, Engine::Shader, Engine::ChunkSpace::ChunkManager, Engine::ChunkSpace::Chunk, Engine::ChunkSpace::Config;

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
        ChunkManager::InitChunkManager(camera);
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
                                        shader.UpdatePerspective(width, height, (float)Config::GetRenderDistance() * Config::GetChunkSize() - Config::GetChunkSize());
                                    });
    shader.UpdatePerspective(_app->GetWidth(), _app->GetHeight(), (float)Config::GetRenderDistance() * Config::GetChunkSize() - Config::GetChunkSize());
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