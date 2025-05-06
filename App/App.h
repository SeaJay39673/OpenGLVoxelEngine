#pragma once
#ifndef APP_H
#define APP_H
#include "./Window.h"
// #include "../Game/Game.h"
#include "IO/IO.h"

#include <chrono>
#include <thread>

using namespace IO;
using namespace std::chrono;

namespace Application
{
    /**
     * @brief An abstract class that defines the main application interface.
     *
     */
    class App : public Window
    {
    public:
        App()
        {
            glViewport(0, 0, GetWidth(), GetHeight());
            RegisterMouseCursorCallback(
                "Mouse",
                [](double x, double y)
                {
                    Mouse::MouseCursorCallback(x, y);
                });
            RegisterKeyCallback(
                "Keyboard",
                [](int key, int scancode, int action, int mods)
                {
                    Keyboard::KeyboardCallback(key, scancode, action, mods);
                });
        }
        virtual void Run() = 0;

    private:
        virtual void processInput() = 0;
        virtual void update(float dt) = 0;
        virtual void render(float dt) = 0;
    };
}

#endif