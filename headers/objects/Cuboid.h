#pragma once

#include "objects/MeshObject.h"

class Cuboid : public MeshObject {
  public:
    Cuboid(std::string name, glm::vec3 center, glm::vec3 sides, glm::vec3 color)
        : MeshObject(name, new Mesh(GL_TRIANGLES), Shader::Load("phong")) {
        init(name, center, sides, color);
    }
    Cuboid(std::string name, glm::vec3 center, glm::vec3 sides)
        : MeshObject(name, new Mesh(GL_TRIANGLES), Shader::Load("phong")) {
        init(name, center, sides, glm::vec3(1));
    }
    Cuboid(std::string name, glm::vec3 center, float side, glm::vec3 color)
        : MeshObject(name, new Mesh(GL_TRIANGLES), Shader::Load("phong")) {
        init(name, center, glm::vec3(side), color);
    }
    Cuboid(std::string name, glm::vec3 center, float side)
        : MeshObject(name, new Mesh(GL_TRIANGLES), Shader::Load("phong")) {
        init(name, center, glm::vec3(side), glm::vec3(1));
    }
    ~Cuboid();

  private:
    glm::vec3 color;
    glm::vec3 center;
    glm::vec2 sides;

    void init(std::string name, glm::vec3 center, glm::vec3 sides, glm::vec3 color) {
        this->name = name;
        this->center = center;
        this->sides = sides;
        this->color = color;

        glm::vec3 bottomLeft = center - 0.5f * sides;
        mesh->addVertex(bottomLeft, color);
        mesh->addVertex(bottomLeft + sides * glm::vec3(0, 0, 1), color);
        mesh->addVertex(bottomLeft + sides * glm::vec3(0, 1, 0), color);
        mesh->addVertex(bottomLeft + sides * glm::vec3(0, 1, 1), color);
        mesh->addVertex(bottomLeft + sides * glm::vec3(0, 0, 0), color);
        mesh->addVertex(bottomLeft + sides * glm::vec3(0, 0, 1), color);
        mesh->addVertex(bottomLeft + sides * glm::vec3(0, 1, 0), color);
        mesh->addVertex(bottomLeft + sides * glm::vec3(0, 1, 1), color);
        mesh->addVertex(bottomLeft + sides * glm::vec3(0, 0, 1), color);
        mesh->addVertex(bottomLeft + sides * glm::vec3(1, 0, 0), color);
        mesh->addVertex(bottomLeft + sides * glm::vec3(1, 1, 0), color);
        mesh->addVertex(bottomLeft + sides * glm::vec3(1, 1, 1), color);
        mesh->addVertex(bottomLeft + sides * glm::vec3(1, 0, 0), color);
        mesh->addVertex(bottomLeft + sides * glm::vec3(1, 0, 1), color);
        mesh->addVertex(bottomLeft + sides * glm::vec3(1, 1, 0), color);
        mesh->addVertex(bottomLeft + sides * glm::vec3(1, 1, 1), color);

        mesh->addIndices(0, 1, 2);
        mesh->addIndices(1, 3, 2);
        mesh->addIndices(4, 5, 6);
        mesh->addIndices(5, 7, 6);
        mesh->addIndices(0, 2, 4);
        mesh->addIndices(2, 6, 4);
        mesh->addIndices(1, 5, 3);
        mesh->addIndices(5, 7, 3);
        mesh->addIndices(2, 3, 6);
        mesh->addIndices(3, 7, 6);
        mesh->addIndices(0, 1, 4);
        mesh->addIndices(1, 5, 4);

        mesh->addNormal(glm::vec3(0, 0, 1)); 
        mesh->addNormal(glm::vec3(0, 0, 1)); 
        mesh->addNormal(glm::vec3(0, 0, 1)); 
        mesh->addNormal(glm::vec3(0, 0, 1)); 
        mesh->addNormal(glm::vec3(0, 0, -1)); 
        mesh->addNormal(glm::vec3(0, 0, -1)); 
        mesh->addNormal(glm::vec3(0, 0, -1)); 
        mesh->addNormal(glm::vec3(0, 0, -1)); 
        mesh->addNormal(glm::vec3(-1, 0, 0)); 
        mesh->addNormal(glm::vec3(-1, 0, 0)); 
        mesh->addNormal(glm::vec3(-1, 0, 0)); 
        mesh->addNormal(glm::vec3(-1, 0, 0)); 
        mesh->addNormal(glm::vec3(1, 0, 0)); 
        mesh->addNormal(glm::vec3(1, 0, 0)); 
        mesh->addNormal(glm::vec3(1, 0, 0)); 
        mesh->addNormal(glm::vec3(1, 0, 0)); 
        mesh->addNormal(glm::vec3(0, 1, 0)); 
        mesh->addNormal(glm::vec3(0, 1, 0)); 
        mesh->addNormal(glm::vec3(0, 1, 0)); 
        mesh->addNormal(glm::vec3(0, 1, 0)); 
        mesh->addNormal(glm::vec3(0, -1, 0)); 
        mesh->addNormal(glm::vec3(0, -1, 0)); 
        mesh->addNormal(glm::vec3(0, -1, 0)); 
        mesh->addNormal(glm::vec3(0, -1, 0)); 

        mesh->commit();
    }
};
