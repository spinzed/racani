#pragma once

#include "Mesh.h"
#include "Shader.h"
#include "Transform.h"

#include <memory>
#include <optional>
#include <string>

class Light {
  public:
    float position[3];
    float intensity[3];
    float color[3];

    Light(float x, float y, float z, float i1, float i2, float i3, float r, float g, float b) {
        position[0] = x;
        position[1] = y;
        position[2] = z;
        intensity[0] = i1;
        intensity[1] = i2;
        intensity[2] = i3;
        color[0] = r;
        color[1] = g;
        color[2] = b;
    }
};

class Object {
  protected:
    std::unique_ptr<Transform> transform;
    bool setViewMatrix = false;
    int primitiveType = GL_TRIANGLES;

  public:
    std::string name;
    std::string tag;
    unsigned char layerMask = 0b0000000;

    Shader *shader = nullptr;
    Mesh *mesh = nullptr;
    Material *material = nullptr; // this should be a vector

    Object(std::string name);
    virtual ~Object(){};

    virtual std::optional<Intersection> findIntersection(glm::vec3 origin, glm::vec3 direction) {
        return mesh->findIntersection(origin, direction, getModelMatrix());
    }

    virtual void getAABB(glm::vec3 &min, glm::vec3 &max) { return mesh->calculateAABB(getModelMatrix(), min, max); }

    virtual void render();
    virtual void render(Shader *s) = 0;

    glm::mat4 getModelMatrix() { return transform->getMatrix(); };
    Transform *getTransform() { return transform.get(); };
};