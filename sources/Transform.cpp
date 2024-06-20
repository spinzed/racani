#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

#include "Transform.h"
#include "mtr.h"

#include <iostream>

glm::vec3 Transform::right() {
    //return glm::mat3(getMatrix()) * TransformIdentity::right();
    //return glm::transpose(matrix)[0];
    return matrix[0];
}

glm::vec3 Transform::up() {
    return matrix[1];
    //return glm::transpose(matrix)[1];
    //return glm::mat3(getMatrix()) * TransformIdentity::up();
}

glm::vec3 Transform::forward() {
    return -matrix[2];
    //return -glm::transpose(matrix)[2];
    //return glm::mat3(getMatrix()) * TransformIdentity::forward();
}

glm::vec3 Transform::position() {
    //return glm::transpose(matrix)[3];
    return matrix[3];
}

glm::vec3 Transform::vertical() {
    //return glm::inverse(glm::mat3(getMatrix())) * TransformIdentity::up();
    return glm::transpose(matrix)[1];
}

void Transform::translate(glm::vec3 position) {
    //matrix = glm::translate(matrix, position);
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

void Transform::scale(float sc) {
    return scale(glm::vec3(sc, sc, sc));
}

void Transform::lookAt(glm::vec3 point) {
    matrix = glm::lookAt(position(), point, up());
    //matrix = mtr::lookAtMatrix(position(), point, up());
}

void Transform::lookAt(glm::vec3 point, glm::vec3 up) {
    matrix = glm::lookAt(position(), point, up);
    //matrix = mtr::lookAtMatrix(position(), point, up);
}

void Transform::normalize(float minX, float minY, float minZ, float maxX, float maxY, float maxZ) {
    float x = (maxX + minX) / 2;
    float y = (maxY + minY) / 2;
    float z = (maxZ + minZ) / 2;

    float M = std::max(maxX - minX, maxY - minY);
    M = std::max(M, maxZ - minZ);

    translate(glm::vec3(-x, -y, -z));
    scale(2/M);
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

glm::mat4 Transform::getMatrix() {
    return matrix;
}
