#pragma once

#include "Object.h"
#include "Types.h"
#include "mtr.h"

#include <vector>
#include <optional>

class Sphere {
    public:
        Sphere(glm::vec3 center, float radius);
        virtual ~Sphere() {};

        glm::vec3 center;
        float radius;
};