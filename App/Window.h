#pragma once
#ifndef WINDOW_H
#define WINDOW_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <functional>
#include <map>

using std::cout, std::endl, std::unordered_map, std::function, std::string;
/**
 * @brief Class for abstracting an OpenGL window using GLFW3
 *
 */
class Window
{
private:
    static GLFWwindow *window;
    int _width;
    int _height;
    unordered_map<string, function<void(int, int)>> frameSizeCallbacks;
    unordered_map<string, function<void(double, double)>> mouseCallbacks;
    unordered_map<string, function<void(int, int, int, int)>> keyCallbacks;
    void initializeGLFW();
    bool initializeWindow();
    bool initializeGlad();

public:
    Window();
    void Shutdown();
    bool ShouldClose() { return glfwWindowShouldClose(window); }
    void NextFrame();
    int GetWidth() { return _width; }
    int GetHeight() { return _height; }
    void RegisterFrameSizeCallback(string name, function<void(int, int)> callback);
    void DeregisterFrameSizeCallback(string name);
    void RegisterMouseCallback(string name, function<void(double, double)> callback);
    void DeregisterMouseCallback(string name);
    void RegisterKeyCallback(string name, function<void(int, int, int, int)> callback);
    void DeregisterKeyCallback(string name);
    void DisableCursor()
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
    void EnableCursor()
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
};

GLFWwindow *Window::window = nullptr;

//====| Private Methods |====//

void Window::initializeGLFW()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_MAXIMIZED, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GL_FALSE);
}

bool Window::initializeWindow()
{
    window = glfwCreateWindow(800, 600, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        cout << "Failed to create GLFW window" << endl;
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(window);
    glfwGetWindowSize(window, &_width, &_height);
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window,
                                   [](GLFWwindow *window, int width, int height)
                                   {
                                       Window *instance = (Window *)glfwGetWindowUserPointer(window);
                                       if (instance)
                                       {
                                           instance->_width = width;
                                           instance->_height = height;
                                           for (const auto &pair : instance->frameSizeCallbacks)
                                           {
                                               cout << "framesize callback: " << pair.first << endl;
                                               pair.second(width, height);
                                           }
                                       }
                                       else
                                       {
                                           cout << "No Window Instance\n";
                                       }
                                   });
    glfwSetCursorPosCallback(window,
                             [](GLFWwindow *window, double xpos, double ypos)
                             {
                                 Window *instance = (Window *)glfwGetWindowUserPointer(window);
                                 if (instance)
                                 {
                                     for (const auto &pair : instance->mouseCallbacks)
                                     {
                                         cout << "mouse callback: " << pair.first << endl;
                                         pair.second(xpos, ypos);
                                     }
                                 }
                                 else
                                 {
                                     cout << "No Window Instance\n";
                                 }
                             });
    glfwSetKeyCallback(window,
                       [](GLFWwindow *window, int key, int scancode, int action, int mods)
                       {
                           Window *instance = (Window *)glfwGetWindowUserPointer(window);
                           if (instance)
                           {
                               for (const auto &pair : instance->keyCallbacks)
                               {
                                   cout << "key callback: " << pair.first << endl;
                                   pair.second(key, scancode, action, mods);
                               }
                           }
                           else
                           {
                               cout << "No Window Instance\n";
                           }
                       });
    return true;
}

bool Window::initializeGlad()
{
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return false;
    }
    return true;
}

//====| Constructors/Desctructors |====//

/**
 * @brief Construct a new window object, initialize new GLFWWindow if it doesn't exist already
 *
 */
Window::Window()
{
    if (window)
        return;
    initializeGLFW();
    if (!initializeWindow())
    {
        Shutdown();
        exit(1);
    }
    if (!initializeGlad())
    {
        exit(1);
    }
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

//====| Public Methods |====//

/**
 * @brief Close window and terminate GLFW program
 *
 */
void Window::Shutdown()
{
    glfwSetWindowShouldClose(window, true);
}

/**
 * @brief Advance to the next frame
 *
 */
void Window::NextFrame()
{
    glfwSwapBuffers(window);
    glfwPollEvents();
}

void Window::RegisterFrameSizeCallback(string name, function<void(int, int)> callback)
{
    frameSizeCallbacks.insert({name, callback});
}

void Window::DeregisterFrameSizeCallback(string name)
{
    frameSizeCallbacks.erase(name);
}
void Window::RegisterMouseCallback(string name, function<void(double, double)> callback)
{
    mouseCallbacks.insert({name, callback});
}
void Window::DeregisterMouseCallback(string name)
{
    mouseCallbacks.erase(name);
}

void Window::RegisterKeyCallback(string name, function<void(int, int, int, int)> callback)
{
    keyCallbacks.insert({name, callback});
}

void Window::DeregisterKeyCallback(string name)
{
    keyCallbacks.erase(name);
}

#endif