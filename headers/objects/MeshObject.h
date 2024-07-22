#pragma once

#include "models/Mesh.h"
#include "objects/Object.h"
#include "renderables/MeshRenderer.h"
#include "renderer/Shader.h"

class MeshObject : public Object {
  public:
    MeshObject(std::string name, Mesh *mesh, Shader *shader) : Object(name) {
        this->mesh = mesh;
        this->renderable = new MeshRenderer(mesh);
        this->shader = shader;
    }
};
