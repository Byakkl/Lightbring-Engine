#pragma once

#include "component.h"
#include "transform.h"

namespace ECS::Components
{
    class Comp_Transform : public ECS::Components::ComponentBase, public Transform
    {
    public:
    };
}