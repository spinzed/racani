#include "renderables/GenericRenderer.h"
#include "utils/GLDebug.h"

#include <stdlib.h>

GenericRenderer::GenericRenderer() {}

GenericRenderer::~GenericRenderer() {
    assert(indices != nullptr);
    assert(data != nullptr);

    free(VBO);
    glDeleteBuffers(data->size(), VBO);
    glDeleteBuffers(2, EBO);
    glDeleteVertexArrays(1, &VAO);
}

void GenericRenderer::setData(std::vector<BufferInput> *data, std::vector<unsigned int> *indices) {
    this->data = data;
    this->indices = indices;
}

void GenericRenderer::generateBuffers() {
    assert(indices != nullptr);
    assert(data != nullptr);

    GLCheckError();
    glGenVertexArrays(1, &VAO);
    VBO = (GLuint *)malloc(data->size() * sizeof(GLuint));
    glGenBuffers(data->size(), VBO);
    glGenBuffers(2, EBO);
    GLCheckError();
}

void GenericRenderer::updateBufferData() {
    assert(indices != nullptr);
    assert(data != nullptr);

    buffersSet = true;

    if (indices->empty())
        return;

    glBindVertexArray(VAO);

    for (size_t i = 0; i < data->size(); i++) {
        const auto bufferInput = data->at(i);
        const auto actualData = bufferInput.data;
        const auto size = actualData->size();

        if (size == 0)
            continue;

        glBindBuffer(GL_ARRAY_BUFFER, VBO[i]);
        glBufferData(GL_ARRAY_BUFFER, size * sizeof(float), &actualData->at(0), GL_STATIC_DRAW);
        glVertexAttribPointer(i, bufferInput.size, GL_FLOAT, GL_FALSE, bufferInput.size * sizeof(float), (void *)0);
        glEnableVertexAttribArray(i);
        GLCheckError();
    }

    // buffer za indekse, moze biti samo jedan GL_ELEMENT_ARRAY_BUFFER po VAO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[0]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices->size() * sizeof(int), &indices->at(0), GL_STATIC_DRAW);
    GLCheckError();

    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[1]);
    // glBufferData(GL_ELEMENT_ARRAY_BUFFER, textureIndices.size() * sizeof(int), &textureIndices[0], GL_STATIC_DRAW);

    glBindVertexArray(0);
    GLCheckError();
}

// assumes that shader has been set prior
void GenericRenderer::draw() {
    assert(buffersSet);
    assert(indices != nullptr);
    assert(primitiveType != -1);

    GLCheckError();
    glBindVertexArray(VAO);
    GLCheckError();

#if DEBUG_WIREFRAME
    glDrawElements(GL_LINE_STRIP, indices->size(), GL_UNSIGNED_INT, 0);
#else
    glDrawElements(primitiveType, indices->size(), GL_UNSIGNED_INT, 0);
    GLCheckError();
#endif
    glBindVertexArray(0);
    GLCheckError();
}

void GenericRenderer::commit() { updateBufferData(); }