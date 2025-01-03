#pragma once

#include "glm/geometric.hpp"
#include "objects/MeshObject.h"

class Plane : public MeshObject {
  public:
    Plane(std::string name, float width, float height, glm::vec3 color)
        : MeshObject(name, new Mesh(GL_TRIANGLES), Shader::Load("phong")) {
        init(name, width, height, color);
    }
    Plane(std::string name, float width, float height)
        : MeshObject(name, new Mesh(GL_TRIANGLES), Shader::Load("phong")) {
        init(name, width, height, glm::vec3(1));
    }
    ~Plane() {};

  private:
    glm::vec3 color;

    void init(std::string name, float width, float height, glm::vec3 color) {
        this->name = name;
        this->color = color;

        mesh->addVertex(glm::vec3(-width, -height, 0), color);
        mesh->addVertex(glm::vec3(-width, height, 0), color);
        mesh->addVertex(glm::vec3(width, -height, 0), color);
        mesh->addVertex(glm::vec3(width, height, 0), color);

        mesh->addIndices(0, 1, 2);
        mesh->addIndices(1, 3, 2);

        mesh->addNormal(glm::vec3(0, 0, -1));
        mesh->addNormal(glm::vec3(0, 0, -1));
        mesh->addNormal(glm::vec3(0, 0, -1));
        mesh->addNormal(glm::vec3(0, 0, -1));

        mesh->commit();
    }
};
