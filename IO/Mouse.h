#ifndef MOUSE_H
#define MOUSE_H

namespace IO::Mouse
{
    static double x = 0, y = 0;
    static double delx = 0, dely = 0;
    static bool firstMouse = true;

    static bool buttons[1024] = {false}; // GLFW mouse buttons

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

    /**
     * @brief Callback function for mouse buttons.
     *
     * @param button The button that was pressed or released.
     * @param action The action that was performed (pressed or released).
     * @param mods Any modifier keys that were held down during the action.
     */
    void MouseButtonCallback(int button, int action, int mods)
    {
        if (action == GLFW_PRESS)
            buttons[button] = true;
        else if (action == GLFW_RELEASE)
            buttons[button] = false;
    }
}

#endif