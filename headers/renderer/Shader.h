#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

const std::vector<std::string> SHADER_UNIFORMS = {
    "mMatrix",      "pvMatrix",     "cameraPos",     "lightNum",  "lightPosition",   "lightColor",     "lightIntensity",
    "colorAmbient", "colorDiffuse", "colorSpecular", "shininess", "colorReflective", "colorEmissive",  "texture1",
    "hasTextures",  "shadowMap",    "hasShadowMap",  "hasSkybox", "skybox",          "pvCenterMatrix",
};

#define SHADER_MMATRIX 0
#define SHADER_PVMATRIX 1
#define SHADER_CAMERA 2
#define SHADER_LIGHT_NUM 3
#define SHADER_LIGHT_POSITION 4
#define SHADER_LIGHT_COLOR 5
#define SHADER_LIGHT_INTENSITY 6
#define SHADER_MATERIAL_COLOR_AMBIENT 7
#define SHADER_MATERIAL_COLOR_DIFFUSE 8
#define SHADER_MATERIAL_COLOR_SPECULAR 9
#define SHADER_MATERIAL_SHININESS 10
#define SHADER_MATERIAL_COLOR_REFLECTIVE 11
#define SHADER_MATERIAL_COLOR_EMISSIVE 12
#define SHADER_TEXTURE 13
#define SHADER_HAS_TEXTURES 14
#define SHADER_SHADOWMAP 15
#define SHADER_HAS_SHADOWMAP 16
#define SHADER_HAS_SKYBOX 17
#define SHADER_SKYBOX 18
#define SHADER_PVCENTERMATRIX 19

#define SHADER_UNIFORM_SIZE 20

typedef struct {
    GLuint type;
    std::string path;
    bool optional;
} ShaderData;

class Shader {
  private:
    void checkCompilerErrors(unsigned int shader, std::string type);

    GLint uniformPositions[SHADER_UNIFORM_SIZE];

    static std::string _baseDir;
    static std::unordered_map<std::string, Shader *> loadedShaders;
    static std::unordered_map<std::string, Shader *> loadedCompute;

  public:
    unsigned int ID;

    // de facto methods for shader loading. Caches already loaded shaders.
    static Shader *Load(std::string naziv);
    static Shader *LoadCompute(std::string naziv);

    Shader(std::vector<ShaderData> shaders);
    ~Shader();

    void use();

    GLint getUniformLocation(const std::string &name);
    void setUniformByLocation(int location, int size, int *val) const;

    void setUniform(int index, int value) const;
    void setUniform(int index, float value) const;
    void setUniform(int index, int size, float array[3]) const;
    void setUniform(int index, int size, glm::vec3 vector) const;
    void setUniform(int index, int size, std::vector<float> vector) const;
    void setUniform(int index, int size, glm::mat4 matrix) const;

    void setTexture(int uniform, int textureNum, int textureID);

    void compute(int width, int height);

    static void setBaseDirectory(std::string dir);
};
