#pragma once

#include "Object.h"
#include "Sphere.h"
#include "Types.h"
#include <vector>
#include <optional>

#include <iostream>

class SphereObject : public Object {
  public:
    SphereObject(glm::vec3 center, float radius, glm::vec3 color);
    ~SphereObject();

    Mesh *getMesh() { return mesh; }

    virtual std::optional<Intersection> findIntersection(glm::vec3 origin, glm::vec3 direction);

  private:
    Sphere *sphere;
    glm::vec3 color;

    void generateSphere();
};
