#pragma once

#include "Mesh.h"
#include "Object.h"
#include "Shader.h"
#include "renderables/MeshRenderer.h"

class MeshObject : public Object {
  public:
    MeshObject(std::string name, Mesh *mesh, Shader *shader) : Object(name) {
        this->mesh = mesh;
        this->renderable = new MeshRenderer(mesh);
        this->shader = shader;
    }
};
