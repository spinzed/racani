#pragma once

#include <GLFW/glfw3.h>
#include <Object.h>
#include <MeshObject.h>
#include <glad/glad.h>
#include <glm/glm.hpp>

#include <iostream>
#include <vector>

class Curve : public MeshObject {
  public:
    Curve();
    ~Curve();

    void addControlPoint(glm::vec3 point);
    void finish();
    void render() override;

    int degree() { return points.size() - 1; };

    // void render();
    glm::vec3 evaluatePoint(float t);
    glm::vec3 evaluateInterpolationPoint(float t);

  private:
    void constructInterpolationCurve();
    std::vector<glm::vec3> points;
    Object *controlPolygon;
    Object *interpolationLine;
    glm::mat4 aInterp;
};
