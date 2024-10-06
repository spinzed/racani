#pragma once

#include <models/Curve.h>
#include <objects/Polyline.h>
#include <objects/MeshObject.h>
#include <objects/Object.h>

#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <glm/glm.hpp>

#include <iostream>
#include <vector>

class Bezier : public Object, public Curve {
  public:
    Bezier();

    void addControlPoint(glm::vec3 point);
    void finish();
    void render() override;

    int degree() { return points.size() - 1; };

    // void render();
    glm::vec3 evaluateApproxPoint(float t);
    glm::vec3 evaluateInterpPoint(float t);

    glm::vec3 evaluatePoint(float t) { return evaluateApproxPoint(t); };

  private:
    void constructApproxCurve();
    void constructInterpCurve();

    std::vector<glm::vec3> points;
    std::shared_ptr<Polyline> approxCurve;
    std::shared_ptr<Polyline> interpCurve;
    std::shared_ptr<Mesh> controlMesh;
    std::shared_ptr<MeshObject> controlPolygon;
    glm::mat4 aInterp;
};
