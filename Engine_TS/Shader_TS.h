#pragma once
#ifndef SHADER_TS_H
#define SHADER_TS_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <fstream>
#include <sstream>

#include "Lockable.h"

using std::string, std::cout, std::endl, std::ifstream, std::stringstream, glm::mat4, glm::vec3, glm::value_ptr;

class Shader_TS
{
private:
    unsigned int _id;
    Lockable<mat4> projection;
    bool loadShaderCode(const string &path, string &code);
    bool compileShaderCode(const string code, unsigned int &id, bool isVertex = true);

public:
    Shader_TS(const string &vertexPath, const string &fragmentPath);
    ~Shader_TS();

    void UpdatePerspective(const int &width, const int &height)
    {
        lock_guard<mutex> lock(projection.mut);
        *projection.ptr = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.001f, 100.0f);
    }
    void Use()
    {
        lock_guard<mutex> lock(projection.mut);
        SetMat4f("projection", *projection.ptr);
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

    Lockable<mat4> *GetProjection() { return &projection; };
};

bool Shader_TS::loadShaderCode(const string &path, string &code)
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

bool Shader_TS::compileShaderCode(const string code, unsigned int &id, bool isVertex)
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

Shader_TS::Shader_TS(const string &vertexPath, const string &fragmentPath)
{
    lock_guard<mutex> lock(projection.mut);
    projection.ptr = new mat4(1.f);
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

Shader_TS::~Shader_TS()
{
    glDeleteProgram(_id);
    _id = 0;
}

#endif