#include <cstdlib>
#include "texture.h"
#include "rendererData.h"

Texture::Texture()
    : pRendererData(std::make_unique<RendererData>()){
    width = 0;
    height = 0;
    channels = 0;

    pRendererData->rawData = nullptr;
    pRendererData->rendererData = nullptr;
}

Texture::Texture(int _width, int _height, int _channels = 4)
    : pRendererData(std::make_unique<RendererData>()){
    width = _width;
    height = _height;
    channels = _channels;

    pRendererData->rawData = nullptr;
    pRendererData->rendererData = nullptr;
}