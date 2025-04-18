#pragma once
#ifndef WORLD_H
#define WORLD_H

// #include <glad/glad.h>
// #include <GLFW/glfw3.h>

#include "../Game/Game.h"
#include "../Engine/Shader.h"
#include "../Engine/Chunk.h"
#include "../Engine/Camera.h"
#include "../IO/Mouse.h"
#include "../IO/Keyboard.h"

using std::string;

class World : Game
{
private:
    mat4 view;
    Camera camera;
    Shader shader;
    Chunk *chunk; // Create a chunk at the origin
public:
    World() : shader("./Resources/Shaders/basic.vert", "./Resources/Shaders/basic.frag"), camera(&view)
    {
        view = mat4(1.0f);
        int pos[3] = {-8, -8, -50};
        chunk = new Chunk(pos);
        camera.SetShader(&shader);
        shader.Use();
    };

public:
    void ProcessInput() override
    {

        camera.ProcessInput();
        if (Keyboard::keys[GLFW_KEY_R])
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        if (Keyboard::keys[GLFW_KEY_F])
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        if (Keyboard::keys[GLFW_KEY_ESCAPE])
            _app->Shutdown();
        Mouse::delx = 0;
        Mouse::dely = 0;
    };
    void Render() override
    {
        shader.Use();
        shader.SetMat4f("view", view);
        chunk->Render(shader);
    };
    void Start() override
    {
        _app->RegisterFrameSizeCallback("ViewPort",
                                        [this](int width, int height)
                                        {
                                            glViewport(0, 0, width, height);
                                            shader.UpdatePerspective(_app->GetWidth(), _app->GetHeight());
                                        });
        shader.UpdatePerspective(_app->GetWidth(), _app->GetHeight());
        _app->DisableCursor();
    };
};

#endif