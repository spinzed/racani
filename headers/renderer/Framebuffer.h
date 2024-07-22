#pragma once

#include "objects/Object.h"
#include "renderer/Texture.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>

class Framebuffer {
  public:
    Framebuffer();
    void setDepthTexture(Texture *texture);
    void setupDepth();
    void cleanDepth();
    void use();

  private:
    GLuint FBO;
    Texture *depthTexture = nullptr;
};
