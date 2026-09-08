#pragma once

#include "component.h"
#include "system.h"
#include "entity.h"
#include "ecsStructures.h"
#include <typeindex>
#include <memory>
#include <vector>

namespace ECS
{

    class ECSManager
    {
    public:
        Entity CreateEntity();
        Entity CreateEntity(ECS::Components::ComponentBase (&x)[]);

        ComponentID GetComponentID(std::type_index) const;
        ComponentID AddComponentToECS(std::type_index, size_t);

        void RegisterSystem(const ECS::Systems::SystemBase);
        void UnregisterSystem(const ECS::Systems::SystemBase);

        const std::vector<ArchetypeID> Query(const ArchetypeQuery&) const;
    private:
        class ECSManagerImpl;
        std::unique_ptr<ECSManagerImpl> pImpl;
    };
}