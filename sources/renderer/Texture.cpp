#include "renderer/Texture.h"

#include "utils/GLDebug.h"
#include "renderer/Importer.h"

const int formatMap[] = {-1, GL_RED, -1, GL_RGB, GL_RGBA};

std::unordered_map<int, std::vector<int>> fullFormatMatrix = {
    {GL_UNSIGNED_BYTE, {-1, GL_RED, -1, GL_RGB, GL_RGBA}},
    {GL_FLOAT, {-1, GL_RED, -1, GL_RGB32F, GL_RGBA32F}},
};

Texture::Texture(int glType, int width, int height, bool isDepth) {
    this->glType = glType;
    this->isDepth = isDepth;

    GLCheckError();
    glGenTextures(1, &id);
    // glActiveTexture(GL_TEXTURE0 + 0);
    glBindTexture(glType, id);
    GLCheckError();

    float borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
    glTexParameterfv(glType, GL_TEXTURE_BORDER_COLOR, borderColor);
    glTexParameteri(glType, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(glType, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(glType, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(glType, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    GLCheckError();

    setSize(width, height);
    GLCheckError();
}

void Texture::setSize(int width, int height) {
    this->width = width;
    this->height = height;
    //setData<float>(4, NULL);                                              // not needed, might disable
    GLCheckError();
    glBindImageTexture(0, id, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F); // za compute shader sampler
    GLCheckError();
    // glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture
}

void Texture::use(int textureID) {
    glActiveTexture(GL_TEXTURE0 + textureID);
    glBindTexture(glType, id);
}

Texture *Texture::Load(std::string resourcePath) {
    int width, height, nrChannels;
    unsigned char *data = Importer::LoadTexture(resourcePath, width, height, nrChannels);

    Texture *tx = new Texture(GL_TEXTURE_2D, width, height);
    assert(tx != nullptr);

    tx->setData(nrChannels, data);
    Importer::freeResource(data);

    return tx;
}

// TODO: move to Importer.h (or at least move majority of logic there)
Texture *Texture::Load(std::string resourceName, std::string fileName) { return Load(resourceName + "/" + fileName); }
