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

    glm::mat4 getModelMatrix();
    Transform *getTransform();

    void addBehavior(Behavior *b);
    void removeBehavior(Behavior *b);

    void addChild(Object *o);
    void removeChild(Object *o);

    // find child by name, non-recursive
    Object *GetChild(std::string name);

    void applyTransform();

    void commit(bool force = false);

    static Object *Load(std::string resourceName, std::string objectName);

    std::string name;
    std::string tag;
    unsigned char layerMask = 0b0000000;
    bool uncommited = false;

    Shader *shader = nullptr;
    Mesh *mesh = nullptr;
    Renderable *renderable = nullptr;
    Material *material = nullptr; // this should be a vector
    std::vector<Behavior *> behaviors;
    Object *parent = nullptr;
    Object *rootParent = nullptr;
    std::vector<Object *> children;

  protected:
    // std::unique_ptr<Transform> transform;
    Transform transform;
    bool setViewMatrix = false;
    int primitiveType = GL_TRIANGLES;

    inline static std::vector<Object *> objects;
};
