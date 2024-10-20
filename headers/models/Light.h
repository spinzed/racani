#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "renderer/Cubemap.h"

#include <vector>

class Light {
  public:
    glm::vec3 position;
    glm::vec3 intensity;
    glm::vec3 color;

    Light(glm::vec3 position, glm::vec3 intensity, glm::vec3 color) {
        this->position = position;
        this->intensity = intensity;
        this->color = color;
    }
};

class PointLight : public Light {
  public:
    using Light::Light;

    Cubemap cb;
    std::vector<glm::mat4> transforms;

    PointLight(glm::vec3 position, glm::vec3 intensity, glm::vec3 color, float range = 100.0f)
        : Light(position, intensity, color), cb(1024, 1024, true) {
        glm::mat4 projection = glm::perspective(glm::radians(90.0f), 1.0f, 0.2f, 100.0f);

        glm::mat4 lookAtPosX =
            projection * glm::lookAt(position, position + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
        glm::mat4 lookAtNegX =
            projection * glm::lookAt(position, position + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
        glm::mat4 lookAtPosY =
            projection * glm::lookAt(position, position + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        glm::mat4 lookAtNegY =
            projection * glm::lookAt(position, position + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f));
        glm::mat4 lookAtPosZ =
            projection * glm::lookAt(position, position + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
        glm::mat4 lookAtNegZ =
            projection * glm::lookAt(position, position + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
        transforms = {lookAtPosX, lookAtNegX, lookAtPosY, lookAtNegY, lookAtPosZ, lookAtNegZ};
    }
};
