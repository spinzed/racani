#pragma once

#include "Importer.h"
#include "Material.h"
#include "Shader.h"
#include "Types.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <assimp/scene.h>

#include <array>
#include <iostream>
#include <optional>
#include <vector>

#define GLCheckError() (glCheckError_(__FILE__, __LINE__))

void glCheckError_(const char *file, int line);

#define DEFAULT_BOUNDING_BOX                                                                                           \
    { glm::vec3(FLT_MAX), glm::vec3(-FLT_MAX) }

typedef struct {
    glm::vec3 min;
    glm::vec3 max;
} BoundingBox;

class Mesh : public ResourceProcessor {
  public:
    Material *material = nullptr;
    glm::vec3 _defaultColor;

    Mesh();
    Mesh(float *vrhovi, float *boje, int brojVrhova);

    virtual ~Mesh();

    static Mesh *Load(std::string ime);
    static Mesh *Load(std::string ime, glm::vec3 defaultColor);

    void addVertex(float x, float y, float z, float r, float g, float b);
    void addVertex(glm::vec3 vrh, glm::vec3 boja);
    void addVertex(float vrh[3], float boja[3]);
    void addVertexStrip(glm::vec3 vrh, glm::vec3 boja); // also adds indices if possible

    void addIndices(unsigned int i1, unsigned int i2, unsigned int i3);
    void addIndices(unsigned int i1, unsigned int i2);
    // template <typename... Args> void addIndices(Args... args);
    // void addIndices(unsigned int i...);
    void addIndices(unsigned int i[]);

    void addNormal(float i1, float i2, float i3) { normals.insert(normals.end(), {i1, i2, i3}); };
    void addNormal(glm::vec3 norm) { addNormal(norm.x, norm.y, norm.z); };

    void updateBufferData();
    void draw();

    void calculateAABB(const glm::mat4 &modelMatrix, glm::vec3 &min, glm::vec3 &max);
    void getBoundingBox(glm::vec3 &min, glm::vec3 &max);
    void setPrimitiveType(int type) { primitiveType = type; }

    std::optional<Intersection> findIntersection(glm::vec3 origin, glm::vec3 direction, glm::mat4 matrix);

    glm::vec3 getIndices(int indeks);
    glm::vec3 getVertex(int redniBroj);
    glm::vec3 getColor(int redniBroj);
    glm::mat3 getTriangle(int indeks);
    glm::vec3 getNormal(int redniBroj);

    int numberOfVertices() { return vrhovi.size() / 3; };
    int numberOfIndices() { return indeksi.size() / 3; };

    void removeAllVertices();

  private:
    BoundingBox bb = DEFAULT_BOUNDING_BOX;

    std::vector<float> vrhovi; // has to be divisible by 3
    std::vector<float> boje;
    std::vector<float> normals;
    std::vector<unsigned int> indeksi;
    std::vector<float> textureCoords;
    std::vector<unsigned int> textureIndices;

    GLuint VAO;
    GLuint VBO[4];
    GLuint EBO[2];
    GLint primitiveType = GL_TRIANGLES;

    void processResource(std::string name, const aiScene *scene);
    void getDataFromArrays(float *vrhovi, float *boje, int brojVrhova);
    void generateBuffers();
};
