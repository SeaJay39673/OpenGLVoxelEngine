#pragma once
#ifndef VAO_H
#define VAO_H

#include "glad/glad.h"

#include "BO.h"

class VAO : Bindable
{
private:
    unsigned int id;

public:
    VAO()
    {
        glGenVertexArrays(1, &id);
    };
    ~VAO()
    {
        glDeleteVertexArrays(1, &id);
    };
    void Bind() override
    {
        glBindVertexArray(id);
    };
    void Unbind() override
    {
        glBindVertexArray(0);
    };

    void LinkBO(BO &bo)
    {
        Bind();
        bo.Bind();
        glVertexAttribPointer(0, 3, GL_INT, GL_FALSE, 3 * sizeof(int), (void *)0);
        // glVertexAttribPointer(1, 3, GL_INT, GL_FALSE, sizeof(int), (void *)(sizeof(int) * 3));
        // glVertexAttribPointer(2, 2, GL_INT, GL_FALSE, sizeof(int), (void *)(sizeof(int) * 6));
        glEnableVertexAttribArray(0);
        // glEnableVertexAttribArray(1);
        // glEnableVertexAttribArray(2);
    }
};

#endif