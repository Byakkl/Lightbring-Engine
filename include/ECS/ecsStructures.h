#pragma once

#include <vector>
#include "component.h"

namespace ECS
{
    using SystemID = size_t;
    using ArchetypeID = size_t;
    using ComponentID = size_t;
    
    struct ArchetypeQuery
    {
        std::vector<ECS::Components::ComponentInfo> target;
    };

}