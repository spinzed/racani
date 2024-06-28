#include "TextureObject.h"
#include "Shader.h"

#include <iostream>

TextureObject::TextureObject(std::string shaderName) {
    shader = Shader::load(shaderName);

    mesh = new Mesh();
    mesh->addVertex(-1, -1, 0, 0, 0, 1);
    mesh->addVertex(1, -1, 0, 1, 0, 1);
    mesh->addVertex(1, 1, 0, 1, 1, 1);
    mesh->addVertex(-1, 1, 0, 0, 1, 1);
    mesh->addIndices(0, 1, 2);
    mesh->addIndices(0, 2, 3);
    mesh->updateBufferData();
}

void TextureObject::setTexture(Texture *texture) { this->texture = texture; }

void TextureObject::render(Raster *raster) {
    if (!texture) {
        std::cerr << "No texture to render" << std::endl;
        return;
    }
    shader->use();
    shader->setUniform(SHADER_TEXTURE, 0);
    glActiveTexture(GL_TEXTURE0);
    texture->setData(raster->get());
    mesh->draw(GL_TRIANGLES);
}
