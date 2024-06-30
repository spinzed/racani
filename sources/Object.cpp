#include "Object.h"
#include "Mesh.h"
#include "Shader.h"
#include "Transform.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include <iostream>

Object::Object(std::string name) : name(name) { transform = std::make_unique<Transform>(); }

void Object::render() { render(shader); }
