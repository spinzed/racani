#include "Object.h"
#include "Mesh.h"
#include "Shader.h"
#include "Transform.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include <iostream>

Object::Object() : Object(nullptr, nullptr) {}

Object::Object(Mesh *mesh, Shader *shader) {
    transform = new Transform();
    this->mesh = mesh;
    this->shader = shader;
}

Object::~Object() { delete transform; }

void Object::render() {
    shader->use();
    mesh->draw(primitiveType);
}

void Object::render(Shader *s) {
    s->use();
    mesh->draw(primitiveType);
}