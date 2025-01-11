#pragma once

#include "renderer/Texture.h"
#include "renderer/CubemapArray.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>

class Framebuffer {
  public:
    Framebuffer();
    void setDepthTexture(Texture *texture);
    void setDepthTexture(CubemapArray *array, int index);
    void setupDepth();
    void cleanDepth(int width, int height);
    void use();

  private:
    GLuint FBO;
    Texture *depthTexture = nullptr;
};
