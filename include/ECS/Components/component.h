#pragma once
#include<memory>
namespace ECS::Components
{
    using ComponentID = size_t;

    class ComponentBase{};

    struct ComponentData
    {
        //Identifier of what the component type is
        ComponentID componentID;
        //Pointer to the data buffer
        void* data;
        //Size of a single data element
        size_t elementSize;
        //Current number of data elements in the buffer
        size_t elementCount;
    };

    struct ComponentInfo
    {
        //Identifier of what the component type is
        ComponentID componentID;
        //Size of a single data element
        size_t componentSize;
    };
}