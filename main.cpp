#include <iostream>

#include "./App/App.h"
#include "./World/World.h"

int main(int, char **)
{
    App app;
    World world;
    app.LoadGame((Game *)&world);
    app.Run();
}
