#include "Texture.h"

Texture::Texture(int width, int height) {
    // Kreiranje teksture za dubinsku mapu
    glGenTextures(1, &id);
    textureNum = 1;
    this->width = width;
    this->height = height;

    glBindTexture(GL_TEXTURE_2D, id);
    // glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT,
                 NULL); // NULL jer nemamo default raster
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = {1.0, 1.0, 1.0, 1.0};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
}

void Texture::use() { glBindTexture(GL_TEXTURE_2D, id); }

void Texture::setData(float *raster) {
    use();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, (const void *)raster);
}

// void Texture::render() {
//     // glBindTexture(GL_TEXTURE_2D, reasterID1);
//     shader->use();
//     glActiveTexture(GL_TEXTURE0);
//     // glBindTexture(GL_TEXTURE_2D, rasterIDs[currentRasterIndex]);
//     glBindTexture(GL_TEXTURE_2D, texture);
//     // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, _width, _height, 0, GL_RGB, GL_FLOAT, (const void *)currentRaster);
//
//     glBindVertexArray(VAO);
//     glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
//     glBindVertexArray(0);
// }
