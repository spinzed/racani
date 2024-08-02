#include "renderer/Texture.h"

#include <stb_image.h>

const int formatMap[] = {-1, GL_RED, -1, GL_RGB, GL_RGBA};

std::unordered_map<int, std::vector<int>> fullFormatMatrix = {
    {GL_UNSIGNED_BYTE, {-1, GL_RED, -1, GL_RGB, GL_RGBA}},
    {GL_FLOAT, {-1, GL_RED, -1, GL_RGB32F, GL_RGBA32F}},
};

Texture::Texture(int width, int height) {
    glGenTextures(1, &id);
    setSize(width, height);
}

void Texture::setSize(int width, int height) {
    this->width = width;
    this->height = height;
    glBindTexture(GL_TEXTURE_2D, id);
    float borderColor[] = {1.0, 1.0, 1.0, 1.0};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    // NULL na kraju jer nemamo default raster
    // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
    setData<float>(4, NULL);
    // glTexImage2D(GL_TEXTURE_2D, 0, GL_channels_COMPONENT, width, height, 0, GL_channels_COMPONENT, GL_FLOAT, NULL);
    glBindImageTexture(0, id, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F); // za compute shader sampler
    // glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture
}

void Texture::bind(int textureID) { glActiveTexture(GL_TEXTURE0 + textureID); }

void Texture::use() { glBindTexture(GL_TEXTURE_2D, id); }

// TODO: move to Importer.h (or at least move majority of logic there)
Texture TextureLoader::Load(std::string resourceName, std::string fileName) {
    int width, height, nrChannels;
    unsigned char *data = Importer::LoadTexture(resourceName, fileName, width, height, nrChannels);

    Texture tx(width, height);
    tx.setData(nrChannels, data);
    stbi_image_free(data);

    return tx;
}