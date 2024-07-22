#pragma once

#include "models/Raster.h"
#include "objects/Object.h"
#include "renderer/Texture.h"

class TextureObject : public Object {
  public:
    TextureObject(std::string name, std::string shaderName);
    ~TextureObject();
    void setTexture(Texture *texture);
    void loadRaster(Raster *raster);

    using Object::render;

    Texture *texture;
};
