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
    virtual void update() = 0;
    virtual void processInput() = 0;

public:
    virtual ~Game() {}
    virtual void Render() = 0;
    void SetApp(App *app) { _app = app; };
    virtual void Start() = 0;
};

#endif