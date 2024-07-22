#include "objects/Object.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include <iostream>

Object::Object(std::string name) : name(name) { transform = std::make_unique<Transform>(); }

void Object::render() {
    assert(shader != nullptr);
    if (renderable == nullptr) {
        std::cout << "sfg" << std::endl;
    }
    assert(renderable != nullptr);
    shader->use();
    renderable->render();
}

void Object::render(Shader *s) {
    Shader *oldShader = shader;
    shader = s;
    render();
    shader = oldShader;
}
