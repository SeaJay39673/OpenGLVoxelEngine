#pragma once
#ifndef WORLD_H
#define WORLD_H

#include <chrono>
#include <thread>
#include <mutex>

#include "../Game/Game.h"
#include "../Engine/ChunkManager.h"
#include "../Engine/Shader.h"
#include "../Engine/Camera.h"
#include "../Engine/Frustum.h"
#include "../IO/Mouse.h"
#include "../IO/Keyboard.h"

using std::string, std::mutex, std::lock_guard;
using namespace std::chrono;

class World : Game
{
private:
    mat4 view;
    mat4 *projection;
    Camera camera;
    Shader shader;
    Frustum frustum;
    vector<function<void()>> updateFunctions;
    mutex updateFunctionsMutex;
    void update() override;

public:
    World() : shader("./Resources/Shaders/basic.vert", "./Resources/Shaders/basic.frag"), camera(&view)
    {
        view = mat4(1.0f);
        camera.SetShader(&shader);
        ChunkManager::InitChunkManager(camera.GetCameraPos(), 8);
    };

    void processInput() override
    {
        camera.ProcessInput();
        if (Keyboard::keys[GLFW_KEY_R])
        {
            lock_guard<mutex> lock(updateFunctionsMutex);
            updateFunctions.push_back([]()
                                      { glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); });
        }
        if (Keyboard::keys[GLFW_KEY_F])
        {
            lock_guard<mutex> lock(updateFunctionsMutex);
            updateFunctions.push_back([]()
                                      { glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); });
        }
        if (Keyboard::keys[GLFW_KEY_ESCAPE])
            _app->Shutdown();
        Mouse::delx = 0;
        Mouse::dely = 0;
    };

    void Render() override
    {
        lock_guard<mutex> lock(updateFunctionsMutex);
        for (auto it = updateFunctions.begin(); it != updateFunctions.end();)
        {
            (*it)();
            it = updateFunctions.erase(it);
        }
        shader.Use();
        shader.SetMat4f("view", view);
        frustum.Update(shader.GetProjection(), view);
        ChunkManager::RenderChunks(shader, frustum);
    };

    void Start() override
    {
        _app->RegisterFrameSizeCallback("ViewPort",
                                        [this](int width, int height)
                                        {
                                            glViewport(0, 0, width, height);
                                            shader.UpdatePerspective(_app->GetWidth(), _app->GetHeight());
                                        });
        shader.UpdatePerspective(_app->GetWidth(), _app->GetHeight());
        _app->DisableCursor();
        std::thread updateThread(&World::update, this);
        updateThread.detach();
    };
};

void World::update()
{
    const int targetTPS = 60; // ticks per second
    const milliseconds tickDuration(1000 / targetTPS);

    auto previousTime = high_resolution_clock::now();

    while (true)
    {
        auto currentTime = high_resolution_clock::now();
        auto elapsedTime = duration_cast<milliseconds>(currentTime - previousTime);
        if (elapsedTime >= tickDuration)
        {
            previousTime = currentTime;
            processInput();
        }
        else
        {
            std::this_thread::sleep_for(tickDuration - elapsedTime);
        }
    }
}

#endif