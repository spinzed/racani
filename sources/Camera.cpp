#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Camera.h"

Camera::Camera(int width, int height) {
    setSize(width, height);
    recalculateMatrix();
}

void Camera::setSize(int width, int height) {
    constraints.near = 0.1f;
    constraints.far = 100.0f;
    constraints.angle = 70.0f;
    float w = constraints.near * tan(glm::radians(constraints.angle / 2));
    float h = (float)height / width * w;
    constraints.left = -w;
    constraints.right = w;
    constraints.bottom = -h;
    constraints.top = h;
    projectionMatrix = Transform::frustum(constraints.left, constraints.right, constraints.bottom, constraints.top,
                                          constraints.near, constraints.far);
    recalculateMatrix();
}

glm::mat4 Camera::getViewMatrix() { return glm::inverse(getMatrix()); }

glm::mat4 Camera::getProjectionMatrix() { return projectionMatrix; }

glm::mat4 Camera::getProjectionViewMatrix() { return projectionViewMatrix; }

void Camera::recalculateMatrix() {
    projectionViewMatrix = projectionMatrix * getViewMatrix();
    notify();
}
