#pragma once

#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include <Object.h>

class Texture {
  public:
    GLuint id;

    Texture(int width, int height);
    void use();
    void setData(GLint textureID, float *raster);

    int width;
    int height;

    // virtual void render();

  private:
    int textureNum;
};
