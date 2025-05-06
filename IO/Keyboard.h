#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace IO::Keyboard
{
    static bool keys[1024];
    /**
     * @brief Callback function for keyboard input.
     *
     * @param key The key that was pressed or released.
     * @param scancode The scancode of the key.
     * @param action The action that was performed (pressed or released).
     * @param mods
     */
    void KeyboardCallback(int key, int scancode, int action, int mods)
    {
        if (action == GLFW_PRESS)
            keys[key] = true;
        else if (action == GLFW_RELEASE)
            keys[key] = false;
    }

};

#endif