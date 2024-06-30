#pragma once

#include "MeshObject.h"
#include "Object.h"
#include "Sphere.h"
#include "Types.h"
#include <optional>
#include <vector>

#include <iostream>

class SphereObject : public MeshObject {
  public:
    SphereObject(std::string name, glm::vec3 center, float radius, glm::vec3 color);
    ~SphereObject();

    Mesh *getMesh() { return mesh; }

    virtual std::optional<Intersection> findIntersection(glm::vec3 origin, glm::vec3 direction);

  private:
    Sphere *sphere;
    glm::vec3 color;

    void generateSphere();
};
