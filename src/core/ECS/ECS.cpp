#include "ECS.h"

ComponentInfo ECSRegistry::GetComponentInfo(std::type_index a_compType) const
{
    //Find component if it exists
    // if(componentRegistry.contains(a_compType))
    //     return componentRegistry[a_compType];

    //Return existing or invalid info structure

   // return info;
   ComponentInfo blank;
   return blank;
}

ComponentInfo ECSRegistry::AddComponentToRegistry(const std::type_index a_compType, size_t a_compSize)
{
    //Find component if it exists
    if(const auto search = componentRegistry.find(a_compType); search != componentRegistry.cend())
        return componentRegistry[a_compType];

    //Create the ComponentInfo and add if it doesn't exist
    ComponentInfo newComp = ComponentInfo(++componentRegistryIdx, a_compSize);
    componentRegistry[a_compType] = newComp;

    //Return component info structure
    return newComp;
}

const std::vector<ComponentInfo> Archetype::GetArchetypeComponentInfo()
{
    return componentInfo;
}

EntityID::EntityID()
{
    idIndex = {};
}

EntityID::EntityID(size_t a_id)
{
    idIndex = {a_id};
}

bool EntityID::operator==(const EntityID& other) const
{
    return idIndex == other.idIndex;
}

size_t EntityID::GetHash() const
{
    return std::hash<size_t>()(idIndex);
}

EntityID ECSDataController::CreateEntity()
{
    Entity newEntity;
    EntityID newEntityID;
    entityMap[newEntityID] = newEntity;

    return newEntityID;
}

EntityID ECSDataController::CreateEntity(ComponentBase (&a_components)[])
{
    Entity newEntity;
    EntityID newEntityID{++entityIdCtr};

    entityMap[newEntityID] = newEntity;

    return newEntityID;
}