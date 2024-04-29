#pragma once

#include <memory>

class RendererData;
class Texture{
public:
    std::unique_ptr<RendererData> pRendererData;

    int width;
    int height;
    int channels;

    Texture();
    Texture(int, int, int);
};