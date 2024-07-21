#pragma once

#include "Object.h"
#include "Raster.h"
#include "Texture.h"

class TextureObject : public Object {
  public:
    TextureObject(std::string name, std::string shaderName);
    ~TextureObject();
    void setTexture(Texture *texture);
    void loadRaster(Raster *raster);

    using Object::render;

    Texture *texture;
};
