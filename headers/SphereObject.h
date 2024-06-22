#pragma once

#include "Object.h"
#include "Sphere.h"

#include <iostream>

class SphereObject : public Object {
  public:
    SphereObject(glm::vec3 center, float radius, glm::vec3 color);
    ~SphereObject();

    Mesh* getMesh()  {
      return mesh;
    }

  private:
    Sphere *sphere;
    glm::vec3 color;

    void generateSphere();
};
