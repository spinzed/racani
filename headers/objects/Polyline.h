#pragma once

#include <objects/MeshObject.h>
#include <objects/Object.h>

#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <glm/glm.hpp>

#include <iostream>
#include <vector>

class Polyline : public MeshObject {
  public:
    Polyline() : MeshObject("curve", new Mesh(GL_LINE_STRIP), Shader::Load("line")) {};
    Polyline(int renderMode) : MeshObject("curve", new Mesh(renderMode), Shader::Load("line")) {};

    void addPoint(glm::vec3 point, glm::vec3 color = glm::vec3(1, 0, 0)) {
        points.push_back(point);
        mesh->addVertex(point, color);
        if (pointNumber() > 1) {
            mesh->addIndex(pointNumber() - 1);
        }
    }
    int pointNumber() { return points.size(); };
    void reset() {
        mesh->removeAllVertices();
        points.clear();
    }

    void commit() { mesh->commit(); };

  private:
    std::vector<glm::vec3> points;
};
