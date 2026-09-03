#pragma once

#include "component.h"
#include "texture.h"

class Material : public ECS::Components::ComponentBase{
public:
    Texture* albedo;

    Material();
};