#pragma once

#include "ecsManager.h"
#include "archetype.h"
#include <unordered_map>

namespace ECS
{

    class ECSManager::ECSManagerImpl
    {
    public:
        Entity CreateEntity();
        Entity CreateEntity(ComponentBase (&x)[]);

        ComponentID GetComponentID(std::type_index) const;
        ComponentID AddComponentToRegistry(std::type_index, size_t);
    private:
        size_t entityIdCtr{};
        size_t archetypeIdCtr{};
        std::unordered_map<ArchetypeID, Archetype> archetypeMap;

        size_t componentRegistryIdx{};
        std::unordered_map<std::type_index, ComponentID> componentRegistry;
    };
}