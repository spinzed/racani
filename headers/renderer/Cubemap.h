#pragma once

#include "renderer/Texture.h"
#include "models/Raster.h"

#include <memory>
#include <vector>

class Cubemap: public Texture {
  public:
    Cubemap(int width, int height);
    template <typename T> void setCubemapData(int side, Raster<T> *raster);
    template <typename T> void setCubemapData(int side, int channels, T *data);

    static Cubemap Load(std::vector<std::string> faces);

  private:
    std::vector<std::unique_ptr<Texture>> textures;
};
