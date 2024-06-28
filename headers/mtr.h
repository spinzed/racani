#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

namespace mtr {
glm::mat4 translate3D(glm::vec3 translateVector);
glm::mat4 scale3D(glm::vec3 scaleVector);
glm::mat4 scale3D(float factor);
glm::mat4 rotate3D(glm::vec3 axis, float angle);

glm::mat4 lookAtMatrix(glm::vec3 eye, glm::vec3 center, glm::vec3 viewUp);
glm::mat4 frustum(float l, float r, float b, float t, float n, float f);
bool intersectLineAndTriangle(glm::vec3 origin, glm::vec3 vrh0, glm::vec3 vrh1, glm::vec3 vrh2, glm::vec3 direction,
                              float *t, float *u, float *v);
bool intersectLineAndAABB(glm::vec3 origin, glm::vec3 direction, glm::vec3 min, glm::vec3 max, glm::vec3 &point);
unsigned int  intersectLineAndSphere(glm::vec3 origin, glm::vec3 direction, glm::vec3 center, float radius, float &t1, float &t2);

glm::vec3 linearRandVec3(float v1, float v2);

}; // namespace mtr
