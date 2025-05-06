#ifndef BINDABLE_H
#define BINDABLE_H

namespace Engine::MeshSpace
{
    /**
     * @brief An interface for bindable objects in the rendering pipeline.
     *
     */
    class Bindable
    {
    public:
        virtual void Bind() = 0;
        virtual void Unbind() = 0;
        virtual ~Bindable() = default;
    };
};

#endif