#pragma once

#include <iostream>

using std::cout, std::endl;

namespace Mouse
{
    static double x = 0, y = 0;
    static double delx = 0, dely = 0;
    static bool firstMouse = true;
    void MouseCallback(double xpos, double ypos)
    {
        if (firstMouse)
        {
            x = xpos;
            y = ypos;
            firstMouse = false;
            return;
        }
        delx = xpos - x;
        dely = ypos - y;
        x = xpos;
        y = ypos;
    }
}