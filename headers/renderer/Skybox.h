#pragma once

#include "renderer/Texture.h"

#include <vector>
#include <memory>

class Skybox {
  public:
    Skybox(std::vector<std::string> images);

  private:
    std::vector<std::unique_ptr<Texture>> textures;
};
