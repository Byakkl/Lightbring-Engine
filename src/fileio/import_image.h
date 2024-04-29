#pragma once
#define STB_IMAGE_IMPLEMENTATION

#include <stdexcept>
#include "stb_image.h"
#include "texture.h"
#include "rendererData.h"

static Texture* importImageFile(const char* filePath){
    Texture* imageData = new Texture();
    imageData->pRendererData->rawData = stbi_load(filePath, &imageData->width, &imageData->height, &imageData->channels, STBI_rgb_alpha);
    
    if(!imageData->pRendererData->rawData)
        throw std::runtime_error("Failed to load texture image");

    return imageData;
}