#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Material {
    public:
        glm::vec3 colorAmbient;
        glm::vec3 colorDiffuse;
        glm::vec3 colorSpecular;
        float shininess;
        glm::vec3 colorReflective;
        glm::vec3 colorEmissive;
        int texture = -1;
};