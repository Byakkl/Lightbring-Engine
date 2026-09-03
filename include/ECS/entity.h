#pragma once
#include <memory>
#include <typeindex>

#include "component.h"

namespace ECS
{
    class Entity
    {
    public:
        void AddComponent(const ComponentBase& a_comp);
        void RemoveComponent(const std::type_index a_compType);
    private:
        class EntityImpl;
        std::unique_ptr<EntityImpl> pImpl;
    };
}