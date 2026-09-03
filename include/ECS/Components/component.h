#pragma once
#include<memory>
namespace ECS::Components
{
    using ComponentID = size_t;

    class ComponentBase{};

    struct ComponentData
    {
        ComponentID componentID;
        void* data;
        size_t elementSize;
        size_t elementCount;
    };
}