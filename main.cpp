#include <iostream>

#include "./App/App.h"
#include "World/World.h"

using namespace Application;
using namespace GameSpace;

int main(int, char **)
{
    App app;
    World world;
    app.LoadGame((Game *)&world);
    app.Run();
}
