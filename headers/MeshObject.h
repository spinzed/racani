#pragma once

#include "Mesh.h"
#include "Object.h"
#include "Shader.h"

class MeshObject : public Object {
  public:
    MeshObject(std::string name, Mesh *mesh, Shader *shader) : Object(name) {
        this->mesh = mesh;
        this->shader = shader;
    }

    void render(Shader *s) override {
        s->use();
        mesh->draw();
    }
};
