#pragma once

#include <glad/glad.h>

class CubemapArray {
  public:
    unsigned int ID;
    int glImageType;
    int glPixelType = GL_UNSIGNED_BYTE;
    int width;
    int height;

    CubemapArray(int GLImageType, int width, int height);

    void setSide(int index, int sideIndex, float *data);

  private:
    int cubemapNum = 10;

};

CubemapArray::CubemapArray(int GLtype, int width, int height) {
    this->glImageType = GLtype;
    glGenTextures(1, &ID);
    glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, ID);

    // Allocate storage
    glTexStorage3D(GL_TEXTURE_CUBE_MAP_ARRAY, 4, glImageType, width, height, cubemapNum * 6);

    // Configure texture parameters (wrapping, filtering)
    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glGenerateMipmap(GL_TEXTURE_CUBE_MAP_ARRAY);
}

void CubemapArray::setSide(int index, int faceIndex, float *data) {
    glTexSubImage3D(GL_TEXTURE_CUBE_MAP_ARRAY, 0, 0, 0, index * 6 + faceIndex, width, height, 1, glImageType, glPixelType, data);
}
