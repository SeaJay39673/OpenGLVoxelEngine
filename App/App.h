#pragma once
#ifndef APP_H
#define APP_H
#include "./Window.h"
#include "../Game/Game.h"
#include "../IO/Mouse.h"
#include "../IO/Keyboard.h"

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
        _game->Start();
    }
    void Run()
    {
        if (_game == nullptr)
        {
            cout << "No Game Loaded\n";
            return;
        }
        while (!ShouldClose())
        {
            glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            _game->ProcessInput();
            _game->Render();
            NextFrame();
        }
        glfwTerminate();
    }
};

#endif