#ifndef WORLD_TS_H
#define WORLD_TS_H

#include <chrono>
#include <thread>

#include "../Game/Game.h"
#include "../Engine_TS/Shader_TS.h"
#include "../IO_TS/Keyboard_TS.h"
#include "../IO_TS/Mouse_TS.h"
#include "../Engine_TS/Camera_TS.h"
#include "../Engine_TS/Chunk_TS.h"
#include "../Engine/Texture.h"

#include <functional>
#include <vector>

using std::function, std::vector;

using namespace std::chrono;

class World_TS : Game
{
private:
    Shader_TS shader;
    bool processingInput = false;
    mutex updateFunctionMutex;
    vector<function<void()>> updateFunctions;
    Camera_TS camera;

    Chunk_TS *chunk;

public:
    void Update() override;
    void ProcessInput() override;
    World_TS() : shader("./Resources/Shaders/basic.vert", "./Resources/Shaders/basic.frag"), camera(&shader)
    {
        Texture::InitializeTextures();
        int pos[3] = {-8, -8, -45};
        chunk = new Chunk_TS(pos);
        // camera.RegisterCameraCallback("UpdateChunks",
        //                               [this](vec3 cameraPos, vec3 cameraDir)
        //                               {
        //                                   std::thread([this, cameraPos]()
        //                                               { chunk->LoadChunk(cameraPos); })
        //                                       .detach();
        //                               });
    };
    void Render() override;
    void Start() override;
};

void World_TS::Start()
{
    _app->RegisterFrameSizeCallback("ViewPort",
                                    [this](int width, int height)
                                    {
                                        glViewport(0, 0, width, height);
                                        shader.UpdatePerspective(_app->GetWidth(), _app->GetHeight());
                                    });
    shader.UpdatePerspective(_app->GetWidth(), _app->GetHeight());
    _app->DisableCursor();
    std::thread updateThread(&World_TS::Update, this);
    updateThread.detach();
    std::thread inputThread(&World_TS::ProcessInput, this);
    inputThread.detach();
}

void World_TS::Render()
{
    lock_guard<mutex> lock(updateFunctionMutex);
    for (auto it = updateFunctions.begin(); it != updateFunctions.end();)
    {
        (*it)();
        it = updateFunctions.erase(it);
    }
    camera.SetView();
    chunk->Render(shader);
}

void World_TS::Update()
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
            if (!chunk->IsLoadedOrUpdating())
            {
                cout << "Loading Chunk\n";
                chunk->LoadChunk(camera.GetCameraPos());
                cout << "Chunk Loaded\n";
            }
        }
        else
        {
            std::this_thread::sleep_for(tickDuration - elapsedTime);
        }
    }
}

void World_TS::ProcessInput()
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
            if (Keyboard_TS::HasKey(GLFW_KEY_ESCAPE))
                _app->Shutdown();
            previousTime = currentTime;
            camera.ProcessInput();
            if (Keyboard_TS::HasKey(GLFW_KEY_R))
            {
                lock_guard<mutex> lock(updateFunctionMutex);
                updateFunctions.push_back([]()
                                          { glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); });
            }
            if (Keyboard_TS::HasKey(GLFW_KEY_F))
            {
                lock_guard<mutex> lock(updateFunctionMutex);
                updateFunctions.push_back([]()
                                          { glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); });
            }
            Mouse_TS::ResetMouseDeltas();
        }
        else
        {
            std::this_thread::sleep_for(tickDuration - elapsedTime);
        }
    }
}

#endif