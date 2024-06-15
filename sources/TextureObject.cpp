#include "TextureObject.h"
#include "Texture.h"
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

void TextureObject::render(float * currentRaster) {
    if (!texture) {
        std::cerr << "No texture to render" << std::endl;
        return;
    }
    shader->use();
    //shader->setUniform(SHADER_TEXTURE, 0);
    shader->setUniform(SHADER_TEXTURE, 0);
    glActiveTexture(GL_TEXTURE0);
    // glBindTexture(GL_TEXTURE_2D, rasterIDs[currentRasterIndex]);
    texture->setData(currentRaster);
    //glBindTexture(GL_TEXTURE_2D, texture->id);
    // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, _width, _height, 0, GL_RGB, GL_FLOAT, (const void *)currentRaster);
    mesh->draw(GL_TRIANGLE_STRIP);
}
