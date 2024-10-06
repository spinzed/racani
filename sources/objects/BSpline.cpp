#include "objects/BSpline.h"

#define SUBDIVISIONS 100

BSpline::BSpline() { shader = Shader::Load("line"); }

void BSpline::addControlPoint(glm::vec3 point) { points.push_back(point); }

void BSpline::finish() {
    reset();
    constructCurve();
    commit();
}

glm::vec3 BSpline::evaluatePoint(float t) {
    int size = points.size();
    if (size < 4)
        return glm::vec3(0);

    if (t < 0)
        t = 0;
    if (t >= 1)
        t = 0.9999f;

    float tt = t * (size - 3);
    int i = (int)tt;

    float localT = tt - i;

    glm::mat4 A(-1, 3, -3, 1, 3, -6, 0, 4, -3, 3, 3, 1, 1, 0, 0, 0);
    glm::mat4 r(glm::vec4(points[i], 1), glm::vec4(points[i + 1], 1), glm::vec4(points[i + 2], 1),
                glm::vec4(points[i + 3], 1));
    glm::vec3 p =
        (1.0f / 6.0f) * glm::vec4(glm::pow(localT, 3), glm::pow(localT, 2), localT, 1) * A * glm::transpose(r);

    std::cout << "t: " << t << " tt: " << tt << " i: " << i << " size: " << size << std::endl;
    return p;
}

glm::vec3 BSpline::evaluateTangent(float t) {
    int size = points.size();
    if (size < 4)
        return glm::vec3(0);

    if (t < 0)
        t = 0;
    if (t >= 1)
        t = 0.9999f;

    float tt = t * (size - 3);
    int i = (int)tt;

    float localT = tt - i;

    glm::mat4x3 A(-1, 2, -1, 3, -4, 0, -3, 2, 1, 1, 0, 0);
    glm::mat4 r(glm::vec4(points[i], 1), glm::vec4(points[i + 1], 1), glm::vec4(points[i + 2], 1),
                glm::vec4(points[i + 3], 1));
    glm::vec3 p = 0.5f * glm::vec3(glm::pow(localT, 2), localT, 1) * A * glm::transpose(r);
    std::cout << "t: " << t << " tt: " << tt << " i: " << i << " size: " << size << std::endl;
    return p;
}

void BSpline::constructCurve() {
    for (int t = 0; t <= SUBDIVISIONS; t++) {
        glm::vec3 point = evaluatePoint((float)t / SUBDIVISIONS);
        // mesh->addVertexStrip(point, color);
        addPoint(point);
    }
}
