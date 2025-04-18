#pragma once
#ifndef GAME_H
#define GAME_H

#include <functional>

#include "../App/App.h"

using std::function;

class App;
class Game
{
protected:
    App *_app = nullptr;

public:
    virtual ~Game() {}
    virtual void ProcessInput() = 0;
    virtual void Render() = 0;
    void SetApp(App *app) { _app = app; };
    virtual void Start() = 0;
};

#endif