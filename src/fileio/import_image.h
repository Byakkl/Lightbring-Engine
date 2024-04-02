#pragma once
#define STB_IMAGE_IMPLEMENTATION

#include <stdexcept>
#include "stb_image.h"
#include "structs.h"

static Texture* importImageFile(const char* filePath){
    Texture* imageData = new Texture();
    imageData->rawData = stbi_load(filePath, &imageData->width, &imageData->height, &imageData->channels, STBI_rgb_alpha);
    
    if(!imageData->rawData)
        throw std::runtime_error("Failed to load texture image");

    return imageData;
}