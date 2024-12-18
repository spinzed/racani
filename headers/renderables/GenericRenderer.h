#pragma once

#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include "renderables/Renderable.h"

#include <vector>

typedef struct {
    std::vector<float> *data;
    GLuint size;
} BufferInput;

// one per object
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
  
  protected:
    void setPrimitiveType(int type) { primitiveType = type; }

  private:
    bool buffersSet = false;
    int primitiveType = -1;
    std::vector<BufferInput> *data = nullptr;
    std::vector<unsigned int> *indices = nullptr;

    void generateBuffers();
    void setData(std::vector<BufferInput> *data, std::vector<unsigned int> *indices);

    GLuint VAO;
    GLuint *VBO;
    GLuint EBO[2];
};
