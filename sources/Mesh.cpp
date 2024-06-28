// Local Headers
#include "Mesh.h"
#include "Importer.h"
#include "Shader.h"
#include "mtr.h"

// System Headers
#include <GLFW/glfw3.h>
#include <glad/glad.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include <assimp/scene.h>

#include <array>
#include <assert.h>
#include <cstdlib>
#include <iostream>
#include <vector>

#define DEFAULT_COLOR glm::vec3(0.5, 0.5, 1)

#define DEBUG_WIREFRAME 0
#define DEBUG_NOCOLOR 0
#define DEBUG_PRINT_OPENGL_ERRORS 0

#define GLCheckError() (glCheckError_(__FILE__, __LINE__))

void glCheckError_(const char *file, int line) {
    GLenum err(glGetError());
    while (err != GL_NO_ERROR) {
        std::string error;
        switch (err) {
        case GL_INVALID_OPERATION:
            error = "INVALID_OPERATION";
            break;
        case GL_INVALID_ENUM:
            error = "INVALID_ENUM";
            break;
        case GL_INVALID_VALUE:
            error = "INVALID_VALUE";
            break;
        case GL_OUT_OF_MEMORY:
            error = "OUT_OF_MEMORY";
            break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            error = "INVALID_FRAMEBUFFER_OPERATION";
            break;
        }
#if DEBUG_PRINT_OPENGL_ERRORS
        std::cerr << "GL_" << error.c_str() << " - " << file << ":" << line << std::endl;
#endif
        err = glGetError();
    }
}

Mesh::Mesh() {
    generateBuffers();
    _defaultColor = DEFAULT_COLOR;
}

Mesh::Mesh(float *vrhovi, float *boje, int brojVrhova) {
    generateBuffers();
    getDataFromArrays(vrhovi, boje, brojVrhova);
    updateBufferData();
}

Mesh *Mesh::Load(std::string ime) { return Mesh::Load(ime, DEFAULT_COLOR); }

Mesh *Mesh::Load(std::string name, glm::vec3 defaultColor) {
    Mesh *mesh = new Mesh();
    mesh->_defaultColor = defaultColor;
    Importer::loadResource(name, (ResourceProcessor *)mesh);
    mesh->updateBufferData();
    return mesh;
}

Mesh::~Mesh() {
    glDeleteBuffers(4, VBO);
    glDeleteBuffers(2, EBO);
    glDeleteVertexArrays(1, &VAO);
}

void Mesh::getDataFromArrays(float *vrhovi, float *boje, int brojVrhova) {
    for (int i = 0; i < brojVrhova; i++) {
        float *vrh = &vrhovi[3 * i];
        float *boja = &boje[3 * i];

        addVertex(vrh, boja);

        if (i > 2) {
            addIndices(i - 2, i - 1, i);
        }
    }
}

void copyColor(aiColor3D color, float *array) {
    array[0] = color.r;
    array[1] = color.g;
    array[2] = color.b;
}

// sugavi assimp
std::string fixPath(const std::string &path) {
    std::string linuxPath = path;
    std::replace(linuxPath.begin(), linuxPath.end(), '\\', '/');
    return linuxPath;
}

void Mesh::processResource(std::string name, const aiScene *scene) {
    if (!scene->HasMeshes())
        return;

    aiMesh *mesh = scene->mMeshes[0];
    if (scene->mNumMeshes > 1) {
        std::cout << "Detected " << scene->mNumMeshes << " meshes, but only one is read" << std::endl;
    }

    if (!mesh->HasFaces())
        return;

    // vertices
    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        float vertex[] = {mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z};

        if (mesh->HasVertexColors(i)) {
            float color[] = {mesh->mColors[i]->r, mesh->mColors[i]->g, mesh->mColors[i]->b, mesh->mColors[i]->a};
            addVertex(vertex, color);
        } else {
            addVertex(vertex, glm::value_ptr(_defaultColor));
        }
    }

    // indices
    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        assert(mesh->mFaces[i].mNumIndices == 3);
        addIndices(mesh->mFaces[i].mIndices);
    }

    // normals
    if (mesh->HasNormals()) {
        for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
            normals.insert(normals.end(), {mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z});
        }
    }

    // texturesCoords
    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        if (mesh->HasTextureCoords(0)) {
            textureCoords.push_back(mesh->mTextureCoords[0][i].x);
            textureCoords.push_back(mesh->mTextureCoords[0][i].y);
        } else {
            textureCoords.push_back(0.0f);
            textureCoords.push_back(0.0f);
        }
    }

    // textureIndices
    for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; ++j) {
            textureIndices.push_back(face.mIndices[j]);
        }
    }

    // materials
    for (unsigned int i = 0; i < scene->mNumMaterials; i++) {
        Materials material;
        aiColor3D ambientK, diffuseK, specularK, reflectiveK, emissiveK;
        float shininessK;
        scene->mMaterials[i]->Get(AI_MATKEY_COLOR_AMBIENT, ambientK);
        scene->mMaterials[i]->Get(AI_MATKEY_COLOR_DIFFUSE, diffuseK);
        scene->mMaterials[i]->Get(AI_MATKEY_COLOR_SPECULAR, specularK);
        scene->mMaterials[i]->Get(AI_MATKEY_SHININESS, shininessK);
        scene->mMaterials[i]->Get(AI_MATKEY_COLOR_REFLECTIVE, reflectiveK);
        scene->mMaterials[i]->Get(AI_MATKEY_COLOR_EMISSIVE, emissiveK);
        copyColor(ambientK, material.colorAmbient);
        copyColor(diffuseK, material.colorDiffuse);
        copyColor(specularK, material.colorSpecular);
        copyColor(reflectiveK, material.colorReflective);
        copyColor(emissiveK, material.colorEmissive);

        aiMaterial *mat = scene->mMaterials[i];
        aiString materialName;
        aiReturn ret;

        ret = mat->Get(AI_MATKEY_NAME, materialName);
        if (ret != AI_SUCCESS)
            materialName = "";

        // Diffuse maps
        int numTextures = mat->GetTextureCount(aiTextureType_DIFFUSE); // Amount of diffuse textures
        aiString textureName; // Filename of the texture using the aiString assimp structure

        if (numTextures > 0 && AI_SUCCESS == mat->Get(AI_MATKEY_TEXTURE(aiTextureType_DIFFUSE, 0), textureName)) {
            // why the hell are there windows delimiters???
            material.texture = Importer::loadTexture(name, fixPath(textureName.data));
        }

        if (numTextures > 1) {
            std::cout << "warning: Detected " << numTextures << " textures, but only one is read" << std::endl;
        }
        materials.push_back(material);
    }
}

BoundingBox Mesh::getBoundingBox() { return bb; }

void Mesh::generateBuffers() {
    GLCheckError();
    glGenVertexArrays(1, &VAO);
    glGenBuffers(4, VBO); // za poziciju (vrhove) i boju
    glGenBuffers(2, EBO);
    GLCheckError();
}

// there should be another class that should bind mesh and shader together (DrawStrategy perhaps?)
// and this should be in that class. TODO I guess
void Mesh::updateBufferData() {
    glBindVertexArray(VAO);

    // vrhovi
    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
    glBufferData(GL_ARRAY_BUFFER, vrhovi.size() * sizeof(float), &vrhovi[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    GLCheckError();

    // boje
    glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
    glBufferData(GL_ARRAY_BUFFER, boje.size() * sizeof(float), &boje[0], GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(1);
    GLCheckError();

    // normale
    if (normals.size() > 0) {
        glBindBuffer(GL_ARRAY_BUFFER, VBO[2]);
        glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(float), &normals[0], GL_STATIC_DRAW);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(2);
        GLCheckError();
    }

    // tekstura
    if (textureCoords.size() > 0) {
        glBindBuffer(GL_ARRAY_BUFFER, VBO[3]);
        glBufferData(GL_ARRAY_BUFFER, textureCoords.size() * sizeof(float), &textureCoords[0], GL_STATIC_DRAW);
        glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(3);
        GLCheckError();
    }

    // buffer za indekse, moze biti samo jedan GL_ELEMENT_ARRAY_BUFFER po VAO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[0]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indeksi.size() * sizeof(int), &indeksi[0], GL_STATIC_DRAW);
    GLCheckError();

    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[1]);
    // glBufferData(GL_ELEMENT_ARRAY_BUFFER, textureIndices.size() * sizeof(int), &textureIndices[0], GL_STATIC_DRAW);

    glBindVertexArray(0);
    GLCheckError();
}

// Doesn't refresh update the buffer data and doesn't add new indices
void Mesh::addVertex(float vrh[3], float boja[3]) { addVertex(vrh[0], vrh[1], vrh[2], boja[0], boja[1], boja[2]); }

// Doesn't refresh update the buffer data and doesn't add new indices
void Mesh::addVertex(glm::vec3 vrh, glm::vec3 boja) { addVertex(vrh[0], vrh[1], vrh[2], boja[0], boja[1], boja[2]); }

// Doesn't refresh update the buffer data and doesn't add new indices
// every vertex-adding must ultimately call this function
void Mesh::addVertex(float x, float y, float z, float a, float b, float c) {
    vrhovi.insert(vrhovi.end(), {x, y, z});

    // a = (float) std::rand() / RAND_MAX, b = (float) std::rand() / RAND_MAX, c = (float) std::rand() / RAND_MAX;
#if DEBUG_NOCOLOR
    boje.insert(boje.end(), {1, 1, 1});
#else
    boje.insert(boje.end(), {a, b, c});
#endif

    if (bb.min.x > x)
        bb.min.x = x;
    if (bb.min.y > y)
        bb.min.y = y;
    if (bb.min.z > z)
        bb.min.z = z;
    if (bb.max.x < x)
        bb.max.x = x;
    if (bb.max.y < y)
        bb.max.y = y;
    if (bb.max.z < z)
        bb.max.z = z;
}

void Mesh::addVertexStrip(glm::vec3 vrh, glm::vec3 boja) {
    addVertex(vrh, boja);
    if (numberOfVertices() > 2) {
        addIndices(numberOfVertices() - 3, numberOfVertices() - 2, numberOfVertices() - 1);
    }
}

// every index-adding must ultimately call this function
void Mesh::addIndices(unsigned int i1, unsigned int i2, unsigned int i3) {
    indeksi.insert(indeksi.end(), {i1, i2, i3});
}

// every index-adding must ultimately call this function
void Mesh::addIndices(unsigned int i1, unsigned int i2) { indeksi.insert(indeksi.end(), {i1, i2}); }

// template <typename... Args> void Mesh::addIndices(Args... args) { (indeksi.push_back(args), ...); }

// void Mesh::addIndices(unsigned int i...) {
//     indeksi.insert(indeksi.end(), i);
// }

void Mesh::addIndices(unsigned int i[]) { addIndices(i[0], i[1], i[2]); }

std::optional<Intersection> Mesh::findIntersection(glm::vec3 origin, glm::vec3 direction, glm::mat4 matrix) {
    Intersection p;

    float t, u, v;
    bool intersected;

    for (unsigned int i = 0; i < indeksi.size() / 3; ++i) {
        glm::mat3 vrhovi = getTriangle(i);

        // TODO: matrix calculation here can be avoided
        glm::vec3 vrh0 = matrix * glm::vec4(vrhovi[0], 1);
        glm::vec3 vrh1 = matrix * glm::vec4(vrhovi[1], 1);
        glm::vec3 vrh2 = matrix * glm::vec4(vrhovi[2], 1);

        intersected = mtr::intersectLineAndTriangle(origin, direction, vrh0, vrh1, vrh2, &t, &u, &v);

        if (intersected) {
            p.t = t;
            // p.u = u;
            // p.v = v;
            // p.vertices[0] = vrh0;
            // p.vertices[1] = vrh1;
            // p.vertices[2] = vrh2;
            p.point = origin + t * direction;
            glm::vec3 indices = getIndices(i);
            // p.colors[0] = getColor(indices[0]);
            // p.colors[1] = getColor(indices[1]);
            // p.colors[2] = getColor(indices[2]);
            //p.color = (getColor(indices[0]) + getColor(indices[1]) + getColor(indices[2])) * (1.0f / 3);
            p.color = getColor(indices[0]);
            //p.color = glm::vec3(0.3, 0.2, 0.1);
            // p.normals[0] = getNormal(indices[0]);
            // p.normals[1] = getNormal(indices[1]);
            // p.normals[2] = getNormal(indices[2]);
            p.normal = glm::normalize(glm::cross(vrh1 - vrh0, vrh2 - vrh0));
            return p;
        }
    }
    return std::nullopt;
}

glm::vec3 Mesh::getIndices(int indeks) {
    assert((unsigned int) indeks < indeksi.size() / 3);

    int start = 3 * indeks;
    int i0 = indeksi[start];
    int i1 = indeksi[start + 1];
    int i2 = indeksi[start + 2];

    return glm::vec3(i0, i1, i2);
}

glm::mat3 Mesh::getTriangle(int indeks) {
    glm::vec3 indices = getIndices(indeks);
    return glm::mat3(getVertex(indices[0]), getVertex(indices[1]), getVertex(indices[2]));
}

glm::vec3 Mesh::getVertex(int indeks) {
    assert((unsigned int) indeks < vrhovi.size() / 3);

    int start = 3 * indeks;
    glm::vec3 a = glm::vec3(vrhovi[start], vrhovi[start + 1], vrhovi[start + 2]);

    return a;
}

glm::vec3 Mesh::getColor(int indeks) {
    assert((unsigned int) indeks < boje.size() / 3);

    int start = 3 * indeks;
    glm::vec3 a = glm::vec3(boje[start], boje[start + 1], boje[start + 2]);

    return a;
}

glm::vec3 Mesh::getNormal(int indeks) {
    assert((unsigned int) indeks < vrhovi.size() / 3);

    int start = 3 * indeks;
    float v0 = normals[start];
    float v1 = normals[start + 1];
    float v2 = normals[start + 2];

    return glm::vec3(v0, v1, v2);
}

// assumes that shader has been set prior
void Mesh::draw(int primitiveType) {
    // assert(indeksi.size() % 3 == 0);
    GLCheckError();
    glBindVertexArray(VAO);
    GLCheckError();
#if DEBUG_WIREFRAME
    glDrawElements(GL_LINES, indeksi.size(), GL_UNSIGNED_INT, 0);
#else
    glDrawElements(primitiveType, indeksi.size(), GL_UNSIGNED_INT, 0);
    GLCheckError();
#endif
    glBindVertexArray(0);
    GLCheckError();
}

void Mesh::removeAllVertices() {
    vrhovi.clear();
    indeksi.clear();
    normals.clear();
    textureCoords.clear();
    textureIndices.clear();
    bb = DEFAULT_BOUNDING_BOX;
}
