#pragma once
#ifndef BINDABLE_H
#define BINDABLE_H
#include <glad/glad.h>

class Bindable
{
public:
    virtual void Bind() = 0;
    virtual void Unbind() = 0;
    virtual ~Bindable() = default;
};

#endif