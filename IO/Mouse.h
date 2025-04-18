#pragma once

#include <iostream>

using std::cout, std::endl;

namespace Mouse
{
    static double x, y;
    static double delx, dely;
    void MouseCallback(double xpos, double ypos)
    {
        delx = xpos - x;
        dely = ypos - y;
        x = xpos;
        y = ypos;
    }
}