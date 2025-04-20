#pragma once
#ifndef KEYBOARD_TS_H
#define KEYBOARD_TS_H

#include <mutex>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

using std::mutex, std::lock_guard;

class Keyboard_TS
{
private:
    static bool keys[1024];
    static mutex KeyboardMutex;

public:
    static void KeyboardCallback(int key, int scancode, int action, int mods)
    {
        lock_guard<mutex> lock(KeyboardMutex);
        if (action == GLFW_PRESS)
            keys[key] = true;
        else if (action == GLFW_RELEASE)
            keys[key] = false;
    }
    static bool HasKey(int action)
    {
        lock_guard<mutex> lock(KeyboardMutex);
        return keys[action];
    }
};

bool Keyboard_TS::keys[1024];
mutex Keyboard_TS::KeyboardMutex;

#endif