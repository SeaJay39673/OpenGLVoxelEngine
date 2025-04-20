#include <iostream>

#include "./App/App.h"
#include "./World/World_TS.h"
#include "World/World.h"

int main(int, char **)
{
    App app;
    World_TS world;
    app.LoadGame((Game *)&world);
    app.Run();
}
