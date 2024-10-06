#pragma once

#include <glm/glm.hpp>

class Curve {
    public:
        virtual glm::vec3 evaluatePoint(float f) = 0;
};
