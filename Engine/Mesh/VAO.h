#pragma once
#ifndef VAO_H
#define VAO_H

#include "glad/glad.h"

#include "BO.h"

namespace Engine::MeshSpace
{
    /**
     * @brief A class for handling vertex array objects in OpenGL.
     *
     */
    class VAO : Bindable
    {
    public:
        VAO() { glGenVertexArrays(1, &id); };
        ~VAO() { glDeleteVertexArrays(1, &id); };
        void Bind() override { glBindVertexArray(id); };
        void Unbind() override { glBindVertexArray(0); };
        void LinkBO(BO &bo);

    private:
        unsigned int id;
    };

    /**
     * @brief Link an array buffer object to the vertex array object.
     * @details This function binds the vertex array object and the buffer object, and sets up the vertex attribute pointers.
     * * This function assumes that the buffer object is a vertex buffer object (VBO) with Vertex data, and sets up the vertex attribute pointers accordingly.
     * * @note This function does not support array objects with integer data types, only vertex data types.
     *
     * @param bo The buffer object to link.
     */
    void VAO::LinkBO(BO &bo)
    {
        Bind();
        bo.Bind();
        switch (bo.GetType())
        {
        case BOType::VERTEX:
            glVertexAttribPointer(0, 3, GL_INT, GL_FALSE, sizeof(Vertex), (void *)0);
            glVertexAttribPointer(1, 3, GL_INT, GL_FALSE, sizeof(Vertex), (void *)(sizeof(int) * 3));
            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)(sizeof(int) * 6));
            break;
        case BOType::INT:
            glVertexAttribPointer(0, 3, GL_INT, GL_FALSE, 6 * sizeof(int) + 2 * sizeof(float), (void *)0);
            glVertexAttribPointer(1, 3, GL_INT, GL_FALSE, 6 * sizeof(int) + 2 * sizeof(float), (void *)(sizeof(int) * 3));
            glVertexAttribPointer(2, 2, GL_INT, GL_FALSE, 6 * sizeof(int) + 2 * sizeof(float), (void *)(sizeof(int) * 6));
            break;
        default:
            break;
        }
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);
    }
};

#endif