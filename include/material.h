#pragma once

#include "component.h"
#include "texture.h"

class Material : public Component{
public:
    Texture* albedo;

    Material();
};