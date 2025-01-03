#pragma once

#include "models/Mesh.h"
#include "renderables/Renderable.h"
#include "renderer/Behavior.h"
#include "renderer/Shader.h"
#include "renderer/Transform.h"

#include <optional>
#include <string>
#include <vector>

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

    glm::mat4 getModelMatrix() {
        return parent != nullptr ? parent->getModelMatrix() * transform.getMatrix() : transform.getMatrix();
    }
    Transform *getTransform() { return &transform; };

    void addBehavior(Behavior *b) { behaviors.emplace_back(b); }

    void removeBehavior(Behavior *b) {
        behaviors.erase(std::remove(behaviors.begin(), behaviors.end(), b), behaviors.end());
    }

    void addChild(Object *o) {
        children.emplace_back(o);
        o->parent = this;
    }

    void removeChild(Object *o) {
        children.erase(std::remove(children.begin(), children.end(), o), children.end());
        o->parent = nullptr;
    }

    void applyTransform() {
        if (mesh) {
            mesh->applyTransform(transform.getMatrix());
            transform.setMatrix(glm::mat4(1));
        }
    }

    std::string name;
    std::string tag;
    unsigned char layerMask = 0b0000000;

    Shader *shader = nullptr;
    Mesh *mesh = nullptr;
    Renderable *renderable = nullptr;
    Material *material = nullptr; // this should be a vector
    std::vector<Behavior *> behaviors;
    Object *parent = nullptr;
    std::vector<Object *> children;

  protected:
    // std::unique_ptr<Transform> transform;
    Transform transform;
    bool setViewMatrix = false;
    int primitiveType = GL_TRIANGLES;
};
