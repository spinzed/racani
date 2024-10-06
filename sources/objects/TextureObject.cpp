#include "objects/TextureObject.h"
#include "renderables/MeshRenderer.h"
#include "renderer/Shader.h"

#include <iostream>

TextureObject::TextureObject(std::string name, std::string shaderName) : Object(name) {
    shader = Shader::Load(shaderName);

    mesh = new Mesh(GL_TRIANGLES);
    mesh->addVertex(-1, -1, 0, 0, 0, 1);
    mesh->addVertex(1, -1, 0, 1, 0, 1);
    mesh->addVertex(1, 1, 0, 1, 1, 1);
    mesh->addVertex(-1, 1, 0, 0, 1, 1);
    mesh->addIndices(0, 1, 2);
    mesh->addIndices(0, 2, 3);
    mesh->commit();

    renderable = new MeshRenderer(mesh);
}

TextureObject::~TextureObject() { delete mesh; }

void TextureObject::setTexture(Texture *texture) {
    this->texture = texture;
    shader->use();
    shader->setTexture(SHADER_TEXTURE, 0, texture->id);
    // shader->setUniform(SHADER_TEXTURE, 0);
}
