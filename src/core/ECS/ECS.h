#pragma once

#include<vector>
#include<typeindex>
#include<unordered_map>

// struct ComponentInfo
// {
//     size_t id;
//     size_t componentSize;

//     ComponentInfo()
//     {
//         id = {};
//         componentSize = {};
//     }
    
//     ComponentInfo(size_t a_id, size_t a_compSize)
//     {
//         id = a_id;
//         componentSize = a_compSize;
//     }
// };

// struct ComponentData
// {
//     //Identifier of what type of component this is
//     ComponentInfo componentInfo;
//     //Pointer to the data buffer
//     void* data;
//     //Size of a data element
//     size_t elementSize;
//     //Current number of data elements
//     size_t elementCount;
// };

// class Archetype
// {
// public:
//     size_t archetypeID;
//     //Const container of the ComponentType from each of the ComponentData entries in 'components'
//     const std::vector<ComponentInfo> GetArchetypeComponentInfo();// => componentInfo;
//     Archetype()
//     {
//         archetypeID = {};
//     }
//     Archetype(size_t a_id)
//     {
//         archetypeID = a_id;
//     }
// private:
//     std::vector<ComponentInfo> componentInfo;
//     std::vector<ComponentData> componentData;
// };



class EntityID
{
public:
    void AddComponent(const ComponentBase& a_comp);
    void RemoveComponent(const std::type_index a_compType);

    EntityID();
    EntityID(size_t a_id);
    bool operator==(const EntityID&) const;
    size_t GetHash()const ;
private:
    size_t idIndex;

};

template<>
struct std::hash<EntityID>
{
    std::size_t operator()(const EntityID& entity) const
    {
        using std::size_t;
        using std::hash;

        return entity.GetHash();
    }
};

struct Entity
{
    size_t archetypeID;
    int index;
};

// class ECSRegistry
// {
// public:
//     //Constant function that returns a value from the componentRegistry
//     ComponentInfo GetComponentInfo(std::type_index) const;
//     //Adds a new entry for a given type_index representing a component and returns the index. Will return the index if the component is already tracked
//     ComponentInfo AddComponentToRegistry(std::type_index, size_t);
// private:
//     size_t componentRegistryIdx{};
//     std::unordered_map<std::type_index, ComponentInfo> componentRegistry;
// };

// class ECSDataController
// {
// public:
//     EntityID CreateEntity();
//     EntityID CreateEntity(ComponentBase (&x)[]);
// private:
//     size_t entityIdCtr{};
//     size_t archetypeIdx{};
//     std::unordered_map<EntityID, Entity> entityMap;
//     std::unordered_map<size_t, Archetype> archetypeMap;
// };
