#ifndef MOUSE_H
#define MOUSE_H

namespace IO::Mouse
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

#endif