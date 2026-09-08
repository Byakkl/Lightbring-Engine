#pragma once

#include "Components/component.h"
#include <vector>

namespace ECS
{
    using ArchetypeID = size_t;

    class Archetype
    {
    public:
        //Identifier for this Archetype
        ArchetypeID archetypeID;    

        /// @brief
        /// @return Const vector of the ComponentInfo for each ComponentID used in this Archetype 
        const std::vector<ECS::Components::ComponentInfo> GetArchetypeComponentInfo();
    private:
        std::vector<ECS::Components::ComponentInfo> componentInfo;
        std::vector<ECS::Components::ComponentData> componentData;
    };
}