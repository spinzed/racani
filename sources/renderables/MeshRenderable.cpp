#include "renderables/MeshRenderer.h"

MeshRenderer::MeshRenderer(Mesh *mesh) {
    this->mesh = mesh;
    mesh->addChangeListener(std::bind(&MeshRenderer::updateBufferData, this));
    generateBuffers();
    updateBufferData();
}

MeshRenderer::~MeshRenderer() {
    glDeleteBuffers(4, VBO);
    glDeleteBuffers(2, EBO);
    glDeleteVertexArrays(1, &VAO);
}

void MeshRenderer::generateBuffers() {
    GLCheckError();
    glGenVertexArrays(1, &VAO);
    glGenBuffers(4, VBO); // za poziciju (vrhove) i boju
    glGenBuffers(2, EBO);
    GLCheckError();
}

typedef struct {
    std::vector<float> *data;
    GLuint size;
} BufferInput;

void MeshRenderer::updateBufferData() {
    std::vector<unsigned int> &indices = mesh->indeksi;

    if (indices.empty())
        return;

    std::vector<BufferInput> allData = {
        BufferInput{&(mesh->vrhovi), 3},
        BufferInput{&(mesh->boje), 3},
        BufferInput{&(mesh->normals), 3},
        BufferInput{&(mesh->textureCoords), 2},
    };

    assert(allData.size() == VBO_NUM);

    glBindVertexArray(VAO);

    for (size_t i = 0; i < allData.size(); i++) {
        const auto bufferInput = allData[i];
        const auto data = bufferInput.data;
        const auto size = data->size();

        if (size == 0)
            continue;

        glBindBuffer(GL_ARRAY_BUFFER, VBO[i]);
        glBufferData(GL_ARRAY_BUFFER, size * sizeof(float), &data->at(0), GL_STATIC_DRAW);
        glVertexAttribPointer(i, allData[i].size, GL_FLOAT, GL_FALSE, allData[i].size * sizeof(float), (void *)0);
        glEnableVertexAttribArray(i);
        GLCheckError();
    }

    // buffer za indekse, moze biti samo jedan GL_ELEMENT_ARRAY_BUFFER po VAO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[0]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(int), &indices[0], GL_STATIC_DRAW);
    GLCheckError();

    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[1]);
    // glBufferData(GL_ELEMENT_ARRAY_BUFFER, textureIndices.size() * sizeof(int), &textureIndices[0], GL_STATIC_DRAW);

    glBindVertexArray(0);
    GLCheckError();
}

// assumes that shader has been set prior
void MeshRenderer::draw() {
    std::vector<unsigned int> &indices = mesh->indeksi;
    GLuint primitiveType = mesh->getPrimitiveType();

    GLCheckError();
    glBindVertexArray(VAO);
    GLCheckError();
#if DEBUG_WIREFRAME
    glDrawElements(GL_LINES, indeksi.size(), GL_UNSIGNED_INT, 0);
#else
    glDrawElements(primitiveType, indices.size(), GL_UNSIGNED_INT, 0);
    GLCheckError();
#endif
    glBindVertexArray(0);
    GLCheckError();
}

void MeshRenderer::commit() { updateBufferData(); }
