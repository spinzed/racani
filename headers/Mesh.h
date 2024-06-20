#pragma once

#include "Importer.h"
#include "Shader.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <assimp/scene.h>

#include <array>
#include <vector>

#define DEFAULT_BOUNDING_BOX {glm::vec3(FLT_MAX, FLT_MAX, FLT_MAX), glm::vec3(-FLT_MAX, -FLT_MAX, -FLT_MAX)}

typedef struct {
    glm::vec3 min;
    glm::vec3 max;
} BoundingBox;

typedef struct {
    bool intersected;
    float t;
    float u;
    float v;
    glm::vec3 point;
    glm::vec3 vertices[3];
    glm::vec3 normals[3];
    glm::vec3 colors[3];
} IntersectPoint;

typedef struct {
    float colorAmbient[3];
    float colorDiffuse[3];
    float colorSpecular[3];
    float shininess;
    float colorReflective[3];
    float colorEmissive[3];
    int texture = -1;
} Materials;

class Mesh : public ResourceProcessor {
  public:
    std::vector<Materials> materials;
    glm::vec3 _defaultColor;

    Mesh();
    Mesh(float *vrhovi, float *boje, int brojVrhova);
    // Mesh(std::string name, glm::vec3 defaultColor);

    ~Mesh();

    static Mesh *Load(std::string ime);
    static Mesh *Load(std::string ime, glm::vec3 defaultColor);

    void addVertex(float x, float y, float z, float r, float g, float b);
    void addVertex(glm::vec3 vrh, glm::vec3 boja);
    void addVertex(float vrh[3], float boja[3]);
    void addVertexStrip(glm::vec3 vrh, glm::vec3 boja); // also adds indices if possible

    void addIndices(unsigned int i1, unsigned int i2, unsigned int i3);
    void addIndices(unsigned int i1, unsigned int i2);
    //template <typename... Args> void addIndices(Args... args);
    //void addIndices(unsigned int i...);
    void addIndices(unsigned int i[]);

    IntersectPoint intersectPoint(glm::vec3 origin, glm::vec3 direction, glm::mat4 matrix);

    void updateBufferData();
    void draw(int primitiveType);

    BoundingBox getBoundingBox();

    glm::vec3 getIndices(int indeks);
    glm::vec3 getVertex(int redniBroj);
    glm::vec3 getColor(int redniBroj);
    glm::mat3 getTriangle(int indeks);
    glm::vec3 getNormal(int redniBroj);

    int numberOfVertices() { return vrhovi.size() / 3; };

    void removeAllVertices();

  private:
    BoundingBox bb = DEFAULT_BOUNDING_BOX;

    std::vector<float> vrhovi; // has to be divisible by 3
    std::vector<float> boje;
    std::vector<float> normals;
    std::vector<unsigned int> indeksi;
    std::vector<float> textureCoords;
    std::vector<unsigned int> textureIndices;
    // Materials *materials = nullptr;

    GLuint VAO;
    GLuint VBO[4];
    GLuint EBO[2];

    void processResource(std::string name, const aiScene *scene);
    void getDataFromArrays(float *vrhovi, float *boje, int brojVrhova);
    void generateBuffers();
};
