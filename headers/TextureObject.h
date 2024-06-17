#pragma once

#include "Object.h"
#include "Raster.h"
#include "Texture.h"

class TextureObject : public Object {
  public:
    TextureObject(std::string shaderName);
    void setTexture(Texture *texture);
    void render(Raster *raster);
    Texture *texture;
};
