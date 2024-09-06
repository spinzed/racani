#pragma once

#include "models/Raster.h"
#include "objects/Object.h"
#include "renderer/Importer.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include <unordered_map>
#include <vector>

template <typename T> struct GLtype;

template <> struct GLtype<float> {
    static constexpr int value = GL_FLOAT;
};

template <> struct GLtype<unsigned char> {
    static constexpr int value = GL_UNSIGNED_BYTE;
};

// channels -> format
extern const int formatMap[];

// type, channels -> format
extern std::unordered_map<int, std::vector<int>> fullFormatMatrix;

class Texture {
  public:
    GLuint id;

    Texture(int glType, int width, int height);
    void use(int textureID);
    void setSize(int width, int height);
    template <typename T> void setData(Raster<T> *raster);
    template <typename T> void setData(int channels, T *data);

    static Texture Load(std::string resourceName, std::string fileName);
    static Texture Load(std::string resourcePath);

    int width;
    int height;

  protected:
    template <typename T> void setTextureData(int glTextureType, int channels, T *data);

  private:
    int glType;
};

template <typename T> void Texture::setData(Raster<T> *raster) { setData(raster->channels, raster->get()); }

template <typename T> void Texture::setData(int channels, T *data) {
    setTextureData(GL_TEXTURE_2D, channels, data);
}

template <typename T> void Texture::setTextureData(int glTextureType, int channels, T *data) {
    assert(channels == 1 || channels == 3 || channels == 4);

    use(0);
    int glType = GLtype<T>::value;
    int pictureFormat = formatMap[channels];
    int fullPictureFormat = fullFormatMatrix[glType][channels];

    glTexImage2D(glTextureType, 0, fullPictureFormat, width, height, 0, pictureFormat, glType, (void *)data);
    glGenerateMipmap(glTextureType); // triba maknit ka opciju
}
