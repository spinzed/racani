#include "TextureObject.h"
#include "Shader.h"

#include <iostream>

TextureObject::TextureObject(std::string name, std::string shaderName): Object(name) {
    shader = Shader::Load(shaderName);

    mesh = new Mesh();
    mesh->addVertex(-1, -1, 0, 0, 0, 1);
    mesh->addVertex(1, -1, 0, 1, 0, 1);
    mesh->addVertex(1, 1, 0, 1, 1, 1);
    mesh->addVertex(-1, 1, 0, 0, 1, 1);
    mesh->addIndices(0, 1, 2);
    mesh->addIndices(0, 2, 3);
    mesh->updateBufferData();
}

TextureObject::~TextureObject() {
    delete mesh;
}

void TextureObject::setTexture(Texture *texture) {
    this->texture = texture;
    shader->use();
    shader->setTexture(0, texture->id);
    //shader->setUniform(SHADER_TEXTURE, 0);
}

void TextureObject::loadRaster(Raster *raster) {
    assert(texture);
    shader->use();
    texture->setData(0, raster->get());
}

void TextureObject::render(Shader *shader) {
    assert(texture);
    shader->use();
    mesh->draw();
}
