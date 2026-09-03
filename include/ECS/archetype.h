#pragma once

#include "Components/component.h"
#include <vector>

namespace ECS
{
    class Archetype
    {
    public:
    
    private:
        std::vector<ECS::Components::ComponentData> componentData;
    };
}