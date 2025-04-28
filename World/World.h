#pragma once
#ifndef WORLD_H
#define WORLD_H

#include "../Game/Game.h"
#include "IO/IO.h"
#include "../Engine/Engine.h"

#include <thread>
#include <chrono>
#include <atomic>

using std::string, std::thread, Engine::Camera, Engine::Shader, Engine::ChunkSpace::ChunkManager, Engine::ChunkSpace::Chunk, Engine::ChunkSpace::Config, Engine::Entities::Player;

using namespace std::chrono;

class World : public Game
{
private:
    atomic<bool> threadShouldClose;
    Player player;
    Shader shader;
    Frustum frustum;

public:
    World() : shader("./Resources/Shaders/basic.vert", "./Resources/Shaders/basic.frag"), player(*(new Camera(&shader)))
    {
        threadShouldClose.store(false);
        type = GameType::CONCURRENT;
        ChunkManager::InitChunkManager(player);
    };
    void Update(duration<float> dt) override;
    void ProcessInput() override;
    void Render(duration<float> dt) override;
    void Start() override;
};

void World::ProcessInput()
{
    player.ProcessInput();
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

void World::Update(duration<float> dt)
{
    frustum.Update(shader.GetProjection(), player.GetCamera().GetCameraView());
    ChunkManager::UpdateChunks(frustum, player);
}

void World::Render(duration<float> dt)
{
    shader.Use();
    ChunkManager::RenderChunks(shader, frustum, player, dt);
};

void World::Start()
{
    _app->RegisterFrameSizeCallback("ViewPort",
                                    [this](int width, int height)
                                    {
                                        cout << width << ", " << height << endl;
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
                Update(duration<float>(0));
                std::this_thread::sleep_for(std::chrono::milliseconds(60));
            }
        });
    update.detach();
};

#endif