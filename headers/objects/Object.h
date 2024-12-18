#pragma once

#include "models/Mesh.h"
#include "renderables/Renderable.h"
#include "renderer/Shader.h"
#include "renderer/Transform.h"

#include <optional>
#include <string>

class Object : public Renderable {
  public:
    Object(std::string name);
    virtual ~Object() {};

    virtual std::optional<Intersection> findIntersection(glm::vec3 origin, glm::vec3 direction) {
        return mesh->findIntersection(origin, direction, getModelMatrix());
    }

    virtual void getAABB(glm::vec3 &min, glm::vec3 &max) { return mesh->calculateAABB(getModelMatrix(), min, max); }

    virtual void render();
    virtual void render(Shader *s);

    glm::mat4 getModelMatrix() { return transform.getMatrix(); };
    Transform *getTransform() { return &transform; };

    std::string name;
    std::string tag;
    unsigned char layerMask = 0b0000000;

    Shader *shader = nullptr;
    Mesh *mesh = nullptr;
    Renderable *renderable = nullptr;
    Material *material = nullptr; // this should be a vector

  protected:
    //std::unique_ptr<Transform> transform;
    Transform transform;
    bool setViewMatrix = false;
    int primitiveType = GL_TRIANGLES;
};
