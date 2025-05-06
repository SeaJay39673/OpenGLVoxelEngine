#ifndef MOUSE_H
#define MOUSE_H

namespace IO::Mouse
{
    static double x = 0, y = 0;
    static double delx = 0, dely = 0;
    static bool firstMouse = true;

    /**
     * @brief Callback function for mouse movement.
     *
     * @param xpos
     * @param ypos
     */
    void MouseCursorCallback(double xpos, double ypos)
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