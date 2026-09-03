#pragma once

#include "entity.h"

namespace ECS
{
    class Entity::EntityImpl
    {
    public:
        void AddComponent(const ComponentBase& a_comp);
        void RemoveComponent(const std::type_index a_compType);

    private:
        ArchetypeID archetype;
        size_t index;
    };
}