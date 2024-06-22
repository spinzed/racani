#pragma once

#include "Object.h"

class Sphere: public Object {
    public:
        Sphere(glm::vec3 center, float radius);
        virtual ~Sphere() {};

        glm::vec3 center;
        float radius;
};