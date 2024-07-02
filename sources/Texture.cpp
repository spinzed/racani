#include "Texture.h"

Texture::Texture(int width, int height) {
    glGenTextures(1, &id);
    textureNum = 1;
    setSize(width, height);
}

void Texture::setSize(int width, int height) {
    this->width = width;
    this->height = height;
    glBindTexture(GL_TEXTURE_2D, id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = {1.0, 1.0, 1.0, 1.0};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    // NULL na kraju jer nemamo default raster
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
    // glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glBindImageTexture(0, id, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F); // za compute shader
    //glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture
}

void Texture::use(int textureID) {
    glActiveTexture(GL_TEXTURE0 + textureID);
    glBindTexture(GL_TEXTURE_2D, id);
}

void Texture::setData(int textureID, float *raster) {
    use(textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, (const void *)raster);
}
