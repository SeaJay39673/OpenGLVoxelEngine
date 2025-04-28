#pragma once
#ifndef GAME_H
#define GAME_H

#include <functional>
#include <chrono>

using std::function;
using namespace std::chrono;

namespace Application
{
    class App;
};

using namespace Application;

namespace GameSpace
{
    enum class GameType
    {
        CONCURRENT,
        SEQUENTIAL
    };

    class Game
    {
    protected:
        App *_app = nullptr;
        GameType type;

    public:
        virtual ~Game() {}
        GameType GameType() { return type; }
        virtual void ProcessInput() = 0;
        virtual void Update(duration<float> dt) = 0;
        virtual void Render(duration<float> dt) = 0;
        void SetApp(App *app) { _app = app; };
        virtual void Start() = 0;
    };
}

#endif