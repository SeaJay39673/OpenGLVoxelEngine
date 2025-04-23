#ifndef BINDABLE_H
#define BINDABLE_H

namespace Engine::MeshSpace
{
    class Bindable
    {
    public:
        virtual void Bind() = 0;
        virtual void Unbind() = 0;
        virtual ~Bindable() = default;
    };
};

#endif