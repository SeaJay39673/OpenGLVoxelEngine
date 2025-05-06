#pragma once
#ifndef WORLD_H
#define WORLD_H

#include "../Engine/Engine.h"
#include "../App/App.h"

#include <thread>
#include <chrono>
#include <atomic>

using std::string, std::thread, Engine::ChunkSpace::ChunkManager, Engine::ChunkSpace::Config, Engine::Entities::Player;

using namespace std::chrono;
using namespace Application;
using namespace Engine;

/**
 * @brief Main game class.
 *
 */
class World : public App
{
public:
    World();
    void Run() override;

private:
    void start();
    void processInput() override;
    void update(float dt) override;
    void render(float dt) override;

    atomic<bool> threadShouldClose;
    Player player;
    Shader shader;
    Frustum frustum;
};

//====| Public Functions |====//

/**
 * @brief Construct a new World::World object
 *
 */
World::World() : shader("./Resources/Shaders/basic.vert", "./Resources/Shaders/basic.frag"), player(*(new Camera(&shader)))
{
    RegisterFrameSizeCallback("ViewPort",
                              [this](int width, int height)
                              {
                                  glViewport(0, 0, width, height);
                                  shader.UpdatePerspective(width, height, (float)Config::GetRenderDistance() * Config::GetChunkSize() - Config::GetChunkSize());
                              });
    shader.UpdatePerspective(GetWidth(), GetHeight(), (float)Config::GetRenderDistance() * Config::GetChunkSize() - Config::GetChunkSize());
    DisableCursor();

    threadShouldClose.store(false);
    ChunkManager::InitChunkManager(player.GetPos());
};

/**
 * @brief Runs the main game loop.
 *
 */
void World::Run()
{

    start();

    const int targetFPS = 120;
    const auto frameDuration = duration<double, std::milli>(1000.0 / targetFPS);

    auto deltaPrevious = high_resolution_clock::now();

    auto previousTime = high_resolution_clock::now();
    auto secondTime = high_resolution_clock::now();

    int fps = 0;

    while (!ShouldClose())
    {
        auto frameStart = high_resolution_clock::now();

        // --- Game Logic & Rendering ---
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // --- Frame Timing Control ---

        auto now = high_resolution_clock::now();
        duration<float> dt(now - deltaPrevious);
        deltaPrevious = now;

        player.Update(dt.count());
        processInput();
        render(dt.count());

        NextFrame();

        auto frameEnd = high_resolution_clock::now();
        auto elapsed = frameEnd - frameStart;
        if (elapsed < frameDuration)
        {
            // Busy-wait for the final precision
            while (high_resolution_clock::now() - frameStart < frameDuration)
            {
                // Spinlock - do nothing
            }
        }

        // fps++;
        // --- FPS Counter ---
        // if (high_resolution_clock::now() - secondTime >= seconds(1))
        // {
        //     cout << "FPS: " << fps << endl;
        //     secondTime = high_resolution_clock::now();
        //     fps = 0;
        // }
    }

    Terminate();
}

//====| Private Functions |====//

/**
 * @brief Starts the update thread.
 *
 */
void World::start()
{
    thread update(
        [this]()
        {
            while (!threadShouldClose.load())
            {
                update(0.0f);
                std::this_thread::sleep_for(std::chrono::milliseconds(60));
            }
        });
    update.detach();
};

/**
 * @brief Process input from the user.
 *
 */
void World::processInput()
{
    player.ProcessInput();
    if (Keyboard::keys[GLFW_KEY_R])
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    if (Keyboard::keys[GLFW_KEY_F])
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    if (Keyboard::keys[GLFW_KEY_ESCAPE])
    {
        threadShouldClose.store(true);
        Shutdown();
    }
    Mouse::delx = 0;
    Mouse::dely = 0;
};

/**
 * @brief Update the world.
 *
 * @param dt
 */
void World::update(float dt)
{
    frustum.Update(shader.GetProjection(), player.GetCamera().GetCameraView());
    ChunkManager::UpdateChunks(frustum, player.GetPos());
}

/**
 * @brief Render the world.
 *
 * @param dt The delta time since the last frame.
 */
void World::render(float dt)
{
    shader.Use();
    ChunkManager::RenderChunks(shader, frustum);
};

#endif