#pragma once

#include "models/Raster.h"
#include "objects/Object.h"
#include "renderables/MeshRenderer.h"
#include "renderer/Texture.h"

class TextureObject : public Object {
  public:
    TextureObject(std::string name, std::string shaderName);
    ~TextureObject();
    void setTexture(Texture *texture);
    template <typename T> void loadRaster(Raster<T> *raster);

    using Object::render;

    Texture *texture = nullptr;
};

template <typename T> void TextureObject::loadRaster(Raster<T> *raster) {
    assert(texture != nullptr);
    shader->use();
    texture->setData(raster);
}
