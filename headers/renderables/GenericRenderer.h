#pragma once

#include "models/Mesh.h"
#include "renderables/Renderable.h"

typedef struct {
    std::vector<float> *data;
    GLuint size;
} BufferInput;

class GenericRenderer : public Renderable {
  public:
    GenericRenderer();
    ~GenericRenderer();

    void init(std::vector<BufferInput> *data, std::vector<unsigned int> *indices, int type) {
        setData(data, indices);
        setPrimitiveType(type);
        generateBuffers();
    }

    void updateBufferData();
    void draw();
    void render() { draw(); };
    void commit();

  private:
    bool buffersSet = false;
    int primitiveType = -1;
    std::vector<BufferInput> *data = nullptr;
    std::vector<unsigned int> *indices = nullptr;

    void generateBuffers();
    void setData(std::vector<BufferInput> *data, std::vector<unsigned int> *indices);
    void setPrimitiveType(int type) { primitiveType = type; }

    GLuint VAO;
    GLuint *VBO;
    GLuint EBO[2];
};
