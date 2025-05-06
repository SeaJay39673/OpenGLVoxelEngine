#pragma once
#ifndef BO_H
#define BO_H

#include <glad/glad.h>

#include <vector>

#include "Bindable.h"
#include "Vertex.h"

using std::vector;

namespace Engine::MeshSpace
{
    enum class BOType
    {
        VERTEX,
        INT,
    };

    /**
     * @brief A class for handling buffer objects in OpenGL.
     *
     */
    class BO : Bindable
    {
    public:
        BO(vector<int> data);
        BO(vector<unsigned int> data);
        BO(vector<Vertex> data);
        ~BO() { glDeleteBuffers(1, &id); };

        unsigned int GetCount() const { return count; };
        BOType GetType() const { return type; };
        void Bind() override { glBindBuffer(target, id); };
        void Unbind() override { glBindBuffer(target, 0); };

    private:
        unsigned int id;
        GLenum target;
        unsigned int count;
        BOType type;
    };

    /**
     * @brief Constructor for the BO class.
     * @details This constructor initializes the array buffer object with a vector of integers.
     *
     * @param data
     */
    BO::BO(vector<int> data) : target(GL_ARRAY_BUFFER), type(BOType::INT)
    {
        glGenBuffers(1, &id);
        Bind();
        glBufferData(target, data.size() * sizeof(int), &data[0], GL_STATIC_DRAW);
        count = (unsigned int)data.size();
    }

    /**
     * @brief Constructor for the BO class.
     * @details This constructor initializes the element array buffer object with a vector of unsigned integers.
     *
     * @param data
     */
    BO::BO(vector<unsigned int> data) : target(GL_ELEMENT_ARRAY_BUFFER)
    {
        glGenBuffers(1, &id);
        Bind();
        glBufferData(target, data.size() * sizeof(unsigned int), &data[0], GL_STATIC_DRAW);
        count = (unsigned int)data.size();
    }

    /**
     * @brief Constructor for the BO class.
     * @details This constructor initializes the array buffer object with a vector of Vertex objects.
     *
     * @param data
     */
    BO::BO(vector<Vertex> data) : target(GL_ARRAY_BUFFER), type(BOType::VERTEX)
    {
        glGenBuffers(1, &id);
        Bind();
        glBufferData(target, data.size() * sizeof(Vertex), &data[0], GL_STATIC_DRAW);
        count = (unsigned int)data.size();
    }
};

#endif