#include "Object.h"
#include "Mesh.h"
#include "Shader.h"
#include "Transform.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

#include <iostream>

Object::Object() : Object(nullptr, nullptr) {}

Object::Object(Mesh *m, Shader *s) {
    transform = new Transform();
    mesh = m;
    shader = s;
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