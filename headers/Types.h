#pragma once

#include <glm/glm.hpp>

class Intersection {
  public:
    Intersection(){};
    Intersection(float t, glm::vec3 point, glm::vec3 color, glm::vec3 normal)
        : t(t), point(point), color(color), normal(normal) {}

    float t;
    glm::vec3 point;
    glm::vec3 color;
    glm::vec3 normal;
};
