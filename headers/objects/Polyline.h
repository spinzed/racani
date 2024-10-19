#pragma once

#include <objects/MeshObject.h>
#include <objects/Object.h>

#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <glm/glm.hpp>

#include <iostream>
#include <vector>

class PolyLine : public MeshObject {
  public:
    PolyLine() : MeshObject("curve", new Mesh(GL_LINE_STRIP), Shader::Load("line")) {};
    PolyLine(int renderMode) : MeshObject("curve", new Mesh(renderMode), Shader::Load("line")) {};
    PolyLine(glm::vec3 defaultColor) : MeshObject("curve", new Mesh(GL_LINE_STRIP), Shader::Load("line")) {
        this->defaultColor = defaultColor;
    };

    void addPoint(glm::vec3 point) { addPoint(point, defaultColor); }
    void addPoint(glm::vec3 point, glm::vec3 color) {
        points.push_back(point);
        mesh->addVertex(point, color);
        mesh->addIndex(pointNumber() - 1);
    }
    int pointNumber() { return points.size(); };
    void reset() {
        mesh->removeAllVertices();
        points.clear();
    }

    void commit() { mesh->commit(); };

  private:
    std::vector<glm::vec3> points;
    glm::vec3 defaultColor = glm::vec3(1, 1, 1);
};
