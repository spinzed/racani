#pragma once

#include "models/Mesh.h"
#include "renderables/Renderable.h"

#define VBO_NUM 4

class MeshRenderer : public Renderable {
  public:
    MeshRenderer(Mesh *mesh);
    ~MeshRenderer();

    void generateBuffers();
    void updateBufferData();
    void draw();
    void render() { draw(); };
    void commit();

    Mesh *getMesh() { return mesh; }

  private:
    Mesh *mesh;
    bool buffersSet = false;

    GLuint VAO;
    GLuint VBO[VBO_NUM];
    GLuint EBO[2];
};
