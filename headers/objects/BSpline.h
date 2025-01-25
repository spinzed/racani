#pragma once

#include <models/Curve.h>
#include <objects/MeshObject.h>
#include <objects/Object.h>
#include <objects/PolyLine.h>

#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <glm/glm.hpp>

#include <vector>

class BSpline : public PolyLine, public Curve {
  public:
    BSpline();

    void addControlPoint(glm::vec3 point);
    void finish(); // tell mesh to transfer to gpu

    int degree() { return points.size() - 1; };

    glm::vec3 evaluatePoint(float t);
    glm::vec3 evaluateTangent(float t);

  private:
    void constructCurve();

    std::vector<glm::vec3> points;
    glm::mat4 aInterp;
};
