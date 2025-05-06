#pragma once
#ifndef WINDOW_H
#define WINDOW_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <functional>
#include <map>

using std::cout, std::endl, std::unordered_map, std::function, std::string;

namespace Application
{
    /**
     * @brief OpenGL Window abstraction class.
     *
     */
    class Window
    {
    public:
        Window();
        void Shutdown();
        void Terminate();
        const bool ShouldClose() const { return glfwWindowShouldClose(window); }
        void NextFrame();
        const int GetWidth() const { return _width; }
        const int GetHeight() const { return _height; }
        void RegisterFrameSizeCallback(string name, function<void(int, int)> callback);
        void DeregisterFrameSizeCallback(string name);
        void RegisterMouseCursorCallback(string name, function<void(double, double)> callback);
        void DeregisterMouseCursorCallback(string name);
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

    private:
        void initializeGLFW();
        bool initializeWindow();
        bool initializeGlad();

        static GLFWwindow *window;
        static int _width;
        static int _height;
        static unordered_map<string, function<void(int, int)>> frameSizeCallbacks;
        static unordered_map<string, function<void(double, double)>> mouseCursorCallbacks;
        static unordered_map<string, function<void(int, int, int, int)>> keyCallbacks;
    };

    GLFWwindow *Window::window = nullptr;
    int Window::_width;
    int Window::_height;
    unordered_map<string, function<void(int, int)>> Window::frameSizeCallbacks;
    unordered_map<string, function<void(double, double)>> Window::mouseCursorCallbacks;
    unordered_map<string, function<void(int, int, int, int)>> Window::keyCallbacks;

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
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK); // Cull back faces (default)
        glFrontFace(GL_CCW); // Counter-clockwise vertices define the front face
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
     * @brief Terminate GLFW program
     *
     */
    void Window::Terminate()
    {
        glfwTerminate();
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

    /**
     * @brief Register a callback for the frame size event.
     *
     * @param name The name of the callback.
     * @param callback The callback function to be called when the frame size changes.
     */
    void Window::RegisterFrameSizeCallback(string name, function<void(int, int)> callback)
    {
        frameSizeCallbacks.insert({name, callback});
    }

    /**
     * @brief Deregister a callback for the frame size event.
     *
     * @param name The name of the callback to be deregistered.
     */
    void Window::DeregisterFrameSizeCallback(string name)
    {
        frameSizeCallbacks.erase(name);
    }

    /**
     * @brief Register a callback for the mouse cursor event.
     *
     * @param name The name of the callback.
     * @param callback The callback function to be called when the mouse cursor moves.
     */
    void Window::RegisterMouseCursorCallback(string name, function<void(double, double)> callback)
    {
        mouseCursorCallbacks.insert({name, callback});
    }

    /**
     * @brief Deregister a callback for the mouse cursor event.
     *
     * @param name The name of the callback to be deregistered.
     */
    void Window::DeregisterMouseCursorCallback(string name)
    {
        mouseCursorCallbacks.erase(name);
    }

    /**
     * @brief Register a callback for the keyboard key event.
     *
     * @param name The name of the callback.
     * @param callback The callback function to be called when a key is pressed or released.
     */
    void Window::RegisterKeyCallback(string name, function<void(int, int, int, int)> callback)
    {
        keyCallbacks.insert({name, callback});
    }

    /**
     * @brief Deregister a callback for the keyboard key event.
     *
     * @param name The name of the callback to be deregistered.
     */
    void Window::DeregisterKeyCallback(string name)
    {
        keyCallbacks.erase(name);
    }

    //====| Private Methods |====//

    /**
     * @brief Initialize GLFW library and set window hints.
     *
     */
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

    /**
     * @brief Initialize the GLFW window.
     *
     * @return true if the window was created successfully, false otherwise.
     */
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
        glfwSetFramebufferSizeCallback(
            window,
            [](GLFWwindow *window, int width, int height)
            {
                Window *instance = (Window *)glfwGetWindowUserPointer(window);
                if (instance)
                {
                    instance->_width = width;
                    instance->_height = height;
                    for (const auto &pair : instance->frameSizeCallbacks)
                    {
                        pair.second(width, height);
                    }
                }
                else
                {
                    cout << "No Window Instance\n";
                }
            });
        glfwSetCursorPosCallback(
            window,
            [](GLFWwindow *window, double xpos, double ypos)
            {
                Window *instance = (Window *)glfwGetWindowUserPointer(window);
                if (instance)
                {
                    for (const auto &pair : instance->mouseCursorCallbacks)
                    {
                        pair.second(xpos, ypos);
                    }
                }
                else
                {
                    cout << "No Window Instance\n";
                }
            });
        glfwSetKeyCallback(
            window,
            [](GLFWwindow *window, int key, int scancode, int action, int mods)
            {
                Window *instance = (Window *)glfwGetWindowUserPointer(window);
                if (instance)
                {
                    for (const auto &pair : instance->keyCallbacks)
                    {
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

    /**
     * @brief Initialize GLAD library.
     *
     * @return true if GLAD was initialized successfully, false otherwise.
     */
    bool Window::initializeGlad()
    {
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            std::cout << "Failed to initialize GLAD" << std::endl;
            return false;
        }
        return true;
    }
}

#endif