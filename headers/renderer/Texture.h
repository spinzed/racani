#pragma once

#include "objects/Object.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>

class Texture {
  public:
    GLuint id;

    Texture(int width, int height);
    void use(int textureID);
    void setSize(int width, int height);
    void setData(int textureID, float *raster);

    int width;
    int height;

    // virtual void render();

  private:
    int textureNum;
};
