#include "objects/Object.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

// Object::Object(std::string name) : name(name) { transform = std::make_unique<Transform>(); }
Object::Object(std::string name) : name(name) {}

void Object::render() {
    if (shader && renderable) {
        shader->use();
        renderable->render();
    }
}

void Object::render(Shader *s) {
    Shader *oldShader = shader;
    shader = s;
    render();
    shader = oldShader;
}
