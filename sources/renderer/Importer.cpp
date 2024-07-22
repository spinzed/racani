#include "renderer/Importer.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <functional>
#include <iostream>

std::string Importer::_path = "";

void Importer::setPath(std::string path) { _path = path; };

unsigned int Importer::loadTexture(std::string resourceName, std::string fileName) {
    unsigned int textureID;
    glGenTextures(1, &textureID);

    std::string finalPath = _path + "/" + resourceName + "/" + fileName;
    int width, height, nrChannels;
    unsigned char *data = stbi_load(finalPath.c_str(), &width, &height, &nrChannels, 0);

    if (data) {
        GLenum format;
        if (nrChannels == 1)
            format = GL_RED;
        else if (nrChannels == 3) // ovo bi tribalo uvik bit
            format = GL_RGB;
        else if (nrChannels == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    } else {
        std::cerr << "Failed to load texture: " << finalPath << std::endl;
        assert(false);
    }
    stbi_image_free(data);
    return textureID;
}

std::string Importer::getFilePath(std::string name) { return _path + "/" + name; }

bool Importer::loadResource(std::string name, ResourceProcessor *processor) {
    std::string objPath = getFilePath(name + "/" + name + ".obj");

    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(
        objPath.c_str(), aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_JoinIdenticalVertices |
                             aiProcess_SortByPType | aiProcess_FlipUVs | aiProcess_GenNormals);

    if (!scene) {
        return false;
    }

    processor->processResource(name, scene);

    return true;
}