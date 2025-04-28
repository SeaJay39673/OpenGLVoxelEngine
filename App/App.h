#pragma once
#ifndef APP_H
#define APP_H
#include "./Window.h"
#include "../Game/Game.h"
#include "IO/IO.h"

#include <chrono>
#include <thread>

using namespace GameSpace;
using namespace IO;
using namespace std::chrono;

namespace Application
{
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
                _game->ProcessInput();
                if (_game->GameType() == GameType::SEQUENTIAL)
                    _game->Update(dt);
                _game->Render(dt);

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

            glfwTerminate();
        }
    };
}

#endif