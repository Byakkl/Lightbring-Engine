#pragma once

#include <stdexcept>
#include <stb_image.h>
#include "structs.h"

static Image* importImageFile(const char* filePath){
    Image imageData;
    imageData.data = stbi_load(filePath, &imageData.width, &imageData.height, &imageData.channels, STBI_rgb_alpha);
    
    if(!imageData.data)
        throw std::runtime_error("Failed to load texture image");

    return &imageData;
}