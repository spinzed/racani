#pragma once

#include "objects/Object.h"
#include "utils/Types.h"
#include "utils/mtr.h"

#include <vector>
#include <optional>

class Sphere {
    public:
        Sphere(glm::vec3 center, float radius);
        virtual ~Sphere() {};

        glm::vec3 center;
        float radius;
};