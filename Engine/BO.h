#pragma once
#ifndef BO_H
#define BO_H

#include <glad/glad.h>

#include <vector>

#include "Bindable.h"
#include "Vertex.h"

using std::vector;

class BO : Bindable
{
private:
    unsigned int id;
    GLenum target;
    unsigned int count;

public:
    BO(vector<int> data) : target(GL_ARRAY_BUFFER)
    {
        glGenBuffers(1, &id);
        Bind();
        glBufferData(target, data.size() * sizeof(int), &data[0], GL_STATIC_DRAW);
        glBufferData(target, data.size() * sizeof(int), &data[0], GL_STATIC_DRAW);
        count = (unsigned int)data.size();
    }

    BO(vector<unsigned int> data) : target(GL_ELEMENT_ARRAY_BUFFER)
    {
        glGenBuffers(1, &id);
        Bind();
        glBufferData(target, data.size() * sizeof(unsigned int), &data[0], GL_STATIC_DRAW);
        count = (unsigned int)data.size();
    }

    ~BO()
    {
        glDeleteBuffers(1, &id);
    };

    unsigned int GetCount() const
    {
        return count;
    };

    void Bind() override
    {
        glBindBuffer(target, id);
    };

    void Unbind() override
    {
        glBindBuffer(target, 0);
    };
};

#endif