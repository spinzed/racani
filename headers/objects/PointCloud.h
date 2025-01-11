#pragma once

#include <objects/MeshObject.h>
#include <objects/Object.h>

#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <glm/glm.hpp>

#include <vector>

class PointCloud : public MeshObject {
  public:
    float pointSize = 1.0f;

    PointCloud() : MeshObject("pointCloud", new Mesh(GL_POINTS), Shader::Load("pointCloud")) {};
    PointCloud(glm::vec3 defaultColor) : MeshObject("pointCloud", new Mesh(GL_POINTS), Shader::Load("pointCloud")) {
        this->defaultColor = defaultColor;
    };

    glm::vec3 getPoint(int i) { return mesh->getVertex(i); }

    void setPoint(int i, glm::vec3 point) {
        i >= mesh->numberOfVertices() ? addPoint(point) : mesh->setVertex(i, point);
    }
    void setPoint(int i, glm::vec3 point, glm::vec3 color) {
        if (i >= mesh->numberOfVertices()) {
            addPoint(point, color);
        } else {
            mesh->setVertex(i, point);
            mesh->setColor(i, color);
        }
    }
    void setPointColor(int i, glm::vec3 color) {
        if (i < mesh->numberOfVertices())
            mesh->setColor(i, color);
    }
    void addPoint(glm::vec3 point) { addPoint(point, defaultColor); }
    void addPoint(glm::vec3 point, glm::vec3 color) {
        points.push_back(point);
        mesh->addVertex(point, color);
        mesh->addIndex(pointNumber() - 1);

        // these are added to prevent OpenGL errors.
        // TODO: make point renderer not have normal and UV buffer objects
        mesh->addNormal(glm::vec3(0, 0, 0));
        mesh->addUV(0, 0);
    }
    int pointNumber() { return points.size(); };
    void reset() {
        mesh->removeAllVertices();
        points.clear();
    }

    void setPointSize(float size) { pointSize = size; }

    virtual void render() {
        if (shader) {
            shader->setFloat("pointSize", pointSize);
        }
        MeshObject::render();
    }

  private:
    std::vector<glm::vec3> points;
    glm::vec3 defaultColor = glm::vec3(1, 1, 1);
};
