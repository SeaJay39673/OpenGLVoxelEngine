#pragma once
#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <fstream>
#include <sstream>

using std::string, std::cout, std::endl, std::ifstream, std::stringstream, glm::mat4, glm::vec3, glm::value_ptr;

namespace Engine
{
    /**
     * @brief OpenGL Shader abstraction class.
     * @details This class handles the loading, compiling, and linking of OpenGL shaders. It also provides methods to set uniform variables in the shader program.
     *      Since this shader creates the OpenGL shader program and updates the shader uniforms, the shader methods must be called on the main thread.
     */
    class Shader
    {
    public:
        Shader(const string &vertexPath, const string &fragmentPath);
        ~Shader();
        void UpdatePerspective(const int &width, const int &height, float depth = 256)
        {
            projection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.001f, depth);
        }
        void Use()
        {
            SetMat4f("projection", projection);
            glUseProgram(_id);
        };
        void SetMat4f(const string &name, const mat4 &mat)
        {
            glUniformMatrix4fv(glGetUniformLocation(_id, name.c_str()), 1, GL_FALSE, value_ptr(mat));
        };
        void SetInt(const std::string &name, int value)
        {
            glUniform1i(glGetUniformLocation(_id, name.c_str()), value);
        }
        void SetFloat(const string &name, float value)
        {
            glUniform1f(glGetUniformLocation(_id, name.c_str()), value);
        }
        mat4 const GetProjection() const { return projection; };

    private:
        unsigned int _id;
        mat4 projection;
        bool loadShaderCode(const string &path, string &code);
        bool compileShaderCode(const string code, unsigned int &id, bool isVertex = true);
    };

    //====| Constructors/Destructors |====//

    /**
     * @brief Construct a new Shader object
     *
     * @param vertexPath Path to the vertex shader file.
     * @param fragmentPath Path to the fragment shader file.
     */
    Shader::Shader(const string &vertexPath, const string &fragmentPath) : projection(mat4(1.0f))
    {
        string code;
        unsigned int vert, frag;
        int success;
        char infoLog[512];
        if (!loadShaderCode(vertexPath, code) || !compileShaderCode(code, vert))
            exit(1);
        if (!loadShaderCode(fragmentPath, code) || !compileShaderCode(code, frag, false))
            exit(1);

        _id = glCreateProgram();
        glAttachShader(_id, vert);
        glAttachShader(_id, frag);
        glLinkProgram(_id);
        // check for linking errors
        glGetProgramiv(_id, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(_id, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::Shader: LINKING_FAILED\n"
                      << infoLog << std::endl;
        }
        glDeleteShader(vert);
        glDeleteShader(frag);
    }

    Shader::~Shader()
    {
        glDeleteProgram(_id);
        _id = 0;
    }

    //====| Private Methods |====//

    /**
     * @brief Load shader code from file.
     *
     * @param path Path to the shader file.
     * @param code Reference to a string to store the loaded shader code.
     * @return true if the shader code was loaded successfully, false otherwise.
     */
    bool Shader::loadShaderCode(const string &path, string &code)
    {
        ifstream shaderFile;
        // Ensure ifstream objects can throw exceptions:
        shaderFile.exceptions(ifstream::failbit | ifstream::badbit);
        try
        {
            // Open files
            shaderFile.open(path);
            stringstream shaderStream;

            // Read file's buffer contents into streams
            shaderStream << shaderFile.rdbuf();

            // Close file handlers
            shaderFile.close();

            code = shaderStream.str();

            return true;
        }
        catch (ifstream::failure e)
        {
            cout << "ERROR::SHADER::loadShaderCode: FILE_NOT_SUCCESFULLY_READ" << endl;
            return false;
        }
    }

    /**
     * @brief Compile shader code.
     *
     * @param code Shader code as a string.
     * @param id Reference to an unsigned int to store the shader ID.
     * @param isVertex Flag indicating whether the shader is a vertex shader (true) or fragment shader (false).
     * @return true if the shader was compiled successfully, false otherwise.
     */
    bool Shader::compileShaderCode(const string code, unsigned int &id, bool isVertex)
    {
        const char *shaderCode = code.c_str();
        int success;
        char infoLog[512];
        // Load and compile the fragment shader
        id = glCreateShader(isVertex ? GL_VERTEX_SHADER : GL_FRAGMENT_SHADER);

        glShaderSource(id, 1, &shaderCode, NULL);
        glCompileShader(id);
        glGetShaderiv(id, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(id, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::" << (isVertex ? "VERTEX" : "FRAGMENT") << "::COMPILATION_FAILED\n"
                      << infoLog << std::endl;
        }
        return success;
    }

};

#endif