#pragma once
#ifndef GAME_H
#define GAME_H

#include <functional>

using std::function;
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
        virtual void Update() = 0;
        virtual void Render() = 0;
        void SetApp(App *app) { _app = app; };
        virtual void Start() = 0;
    };
}

#endif