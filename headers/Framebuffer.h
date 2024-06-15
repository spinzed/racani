#pragma once

#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include "Object.h"
#include "Texture.h"

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
