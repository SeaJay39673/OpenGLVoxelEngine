#pragma once
#ifndef APP_H
#define APP_H
#include "./Window.h"
#include "../Game/Game.h"
#include "../IO/Mouse.h"
#include "../IO/Keyboard.h"

#include <chrono>
#include <thread>

using namespace std::chrono;

class App : public Window
{
private:
    Game *_game = nullptr;

public:
    App()
    {
        glViewport(0, 0, GetWidth(), GetHeight());
        RegisterMouseCallback("Mouse",
                              [](double x, double y)
                              {
                                  Mouse::MouseCallback(x, y);
                              });
        RegisterKeyCallback("Keyboard",
                            [](int key, int scancode, int action, int mods)
                            {
                                Keyboard::KeyboardCallback(key, scancode, action, mods);
                            });
    }
    void LoadGame(Game *game)
    {
        _game = game;
        _game->SetApp(this);
    }
    void Run()
    {
        if (_game == nullptr)
        {
            cout << "No Game Loaded\n";
            return;
        }

        _game->Start();

        const int targetTPS = 120; // ticks per second
        const milliseconds tickDuration(1000 / targetTPS);

        auto previousTime = high_resolution_clock::now();

        while (!ShouldClose())
        {
            auto currentTime = high_resolution_clock::now();
            auto elapsedTime = duration_cast<milliseconds>(currentTime - previousTime);
            if (elapsedTime >= tickDuration)
            {
                previousTime = currentTime;
                glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                _game->ProcessInput();
                if (_game->GameType() == GameType::SEQUENTIAL)
                    _game->Update();
                _game->Render();
                NextFrame();
            }
            else
            {
                std::this_thread::sleep_for(tickDuration - elapsedTime);
            }
        }

        glfwTerminate();
    }
};

#endif