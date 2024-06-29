#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

#include "Transform.h"
#include "mtr.h"

#include <iostream>

glm::vec3 Transform::right() { return matrix[0]; }

glm::vec3 Transform::up() { return matrix[1]; }

glm::vec3 Transform::forward() { return -matrix[2]; }

glm::vec3 Transform::position() { return matrix[3]; }

glm::vec3 Transform::vertical() { return glm::transpose(matrix)[1]; } // something's fishy here

glm::vec3 Transform::direction() { return glm::cross(TransformIdentity::up(), right()); }

void Transform::setRight(glm::vec3 right) { matrix[0] = glm::vec4(right, 0); }
void Transform::setUp(glm::vec3 up) { matrix[1] = glm::vec4(up, 0); }
void Transform::setForward(glm::vec3 forward) { matrix[2] = glm::vec4(-forward, 0); }

void Transform::translate(glm::vec3 position) {
    matrix = glm::translate(matrix, position);
    // matrix = matrix * mtr::translate3D(position);
}

void Transform::setPosition(glm::vec3 position) {
    matrix[3][0] = position[0];
    matrix[3][1] = position[1];
    matrix[3][2] = position[2];
    // matrix = matrix * mtr::translate3D(position);
}

void Transform::rotate(glm::vec3 axis, float degrees) {
    matrix = glm::rotate(matrix, glm::radians(degrees), axis);
    // matrix = mtr::rotate3D(axis, glm::radians(degrees)) * matrix;
}

void Transform::scale(glm::vec3 scale) {
    matrix = glm::scale(matrix, scale);
    // matrix = matrix * mtr::scale3D(scale);
}

void Transform::scale(float sc) { return scale(glm::vec3(sc, sc, sc)); }

void Transform::lookAt(glm::vec3 point) {
    matrix = glm::lookAt(position(), point, up());
    // matrix = mtr::lookAtMatrix(position(), point, up());
}

void Transform::lookAt(glm::vec3 point, glm::vec3 up) {
    matrix = glm::lookAt(position(), point, up);
    // matrix = mtr::lookAtMatrix(position(), point, up);
}

void Transform::normalize(glm::vec3 min, glm::vec3 max) {
    float x = (max.x + min.x) / 2;
    float y = (max.y + min.y) / 2;
    float z = (max.z + min.z) / 2;

    float M = std::max(max.x - min.x, max.y - min.y);
    M = std::max(M, max.z - min.z);

    translate(glm::vec3(-x, -y, -z));
    scale(2 / M);
}

glm::mat4 Transform::frustum(float left, float right, float bottom, float top, float nearp, float far) {
    return mtr::frustum(left, right, bottom, top, nearp, far);
}

glm::mat4 Transform::perspective(int width, int height, float nearp, float far, float angleDeg) {
    // matrix = glm::perspective(glm::radians(70.0f), (float)width/(float)height, 0.1f, 100.0f);
    float w = nearp * tan(glm::radians(angleDeg / 2));
    float h = (float)height / width * w;
    return Transform::frustum(-w, w, -h, h, nearp, far);
}

glm::mat4 Transform::getMatrix() { return matrix; }

glm::vec3 Transform::getEulerAngles() {
    glm::vec3 angles(0);
    float dotProduct = glm::dot(forward(), direction());
    dotProduct = glm::clamp(dotProduct, -1.0f, 1.0f);
    float isvert = glm::dot(forward(), TransformIdentity::up());
    angles.x = std::acos(dotProduct) * glm::sign(isvert);

    dotProduct = glm::dot(forward(), TransformIdentity::forward());
    dotProduct = glm::clamp(dotProduct, -1.0f, 1.0f);
    angles.y = std::acos(dotProduct);

    return angles * (180.0f / glm::pi<float>());
}
