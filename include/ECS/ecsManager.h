#pragma once

#include "component.h"
#include "entity.h"
#include <typeindex>
#include <memory>

namespace ECS
{
    using ArchetypeID = size_t;
    using ComponentID = size_t;

    class ECSManager
    {
    public:
        Entity CreateEntity();
        Entity CreateEntity(ComponentBase (&x)[]);

        ComponentID GetComponentID(std::type_index) const;
        ComponentID AddComponentToECS(std::type_index, size_t);
    private:
        class ECSManagerImpl;
        std::unique_ptr<ECSManagerImpl> pImpl;
    };
}