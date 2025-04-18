#pragma once
#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace Keyboard
{
    static bool keys[1024];
    void KeyboardCallback(int key, int scancode, int action, int mods)
    {
        if (action == GLFW_PRESS)
            keys[key] = true;
        else if (action == GLFW_RELEASE)
            keys[key] = false;
    }
};

#endif