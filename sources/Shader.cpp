#include "Shader.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <vector>

std::string Shader::_baseDir;
std::unordered_map<std::string, Shader*> Shader::loadedShaders;

void Shader::setBaseDirectory(std::string baseDir) { _baseDir = baseDir; };

void Shader::checkCompilerErrors(unsigned int shader, std::string type) {
    int success;
    char infolog[1024];
    if (type != "PROGRAM") {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

        if (!success) {
            glGetShaderInfoLog(shader, 1024, nullptr, infolog);
            fprintf(stderr,
                    "ERROR::SHADER_COMPILATION_ERROR of type: "
                    "%s\n%s\n-----------------------------------------------------\n",
                    type.c_str(), infolog);
        }
    } else {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shader, 1024, nullptr, infolog);
            fprintf(stderr,
                    "ERROR::PROGRAM_LINKING_ERROR of type: "
                    "%s\n%s\n-------------------------------------------------------\n",
                    type.c_str(), infolog);
        }
    }
}

Shader::Shader(std::string vertexPath, std::string fragmentPath, std::string geometryPath) {
    // std::cout << vertexPath << std::endl;
    std::string vertexCode;
    std::string fragmentCode;
    std::string geometryCode;
    std::ifstream vShaderFile;
    std::ifstream fShaderFile;
    std::ifstream gShaderFile;

    //std::cout << vertexPath << std::endl;

    // ensure ifstream objects can throw exceptions:
    vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    try {
        // open files
        vShaderFile.open(vertexPath);
        fShaderFile.open(fragmentPath);
        std::stringstream vShaderStream, fShaderStream;
        // read file's buffer contents into streams
        vShaderStream << vShaderFile.rdbuf();
        fShaderStream << fShaderFile.rdbuf();
        // close file handlers
        vShaderFile.close();
        fShaderFile.close();
        // convert stream into string
        vertexCode = vShaderStream.str();
        fragmentCode = fShaderStream.str();

        gShaderFile.open(geometryPath);
        if (!gShaderFile.fail()) {
            // std::cout << "nasa geom " << geometryPath << std::endl;
            std::stringstream gShaderStream;
            gShaderStream << gShaderFile.rdbuf();
            gShaderFile.close();
            geometryCode = gShaderStream.str();
        }
        // else {
        //     std::cout << "nisan nasa geom " << geometryPath << std::endl;
        // }
    } catch (std::ifstream::failure &e) {
        fprintf(stderr, "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ\n");
    }

    const char *vShaderCode = vertexCode.c_str();
    const char *fShaderCode = fragmentCode.c_str();

    // 2. compile shaders
    unsigned int vertex, fragment, geometry;

    // vertex shader
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, NULL);
    glCompileShader(vertex);
    checkCompilerErrors(vertex, vertexPath);

    // fragment Shader
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, NULL);
    glCompileShader(fragment);
    checkCompilerErrors(fragment, fragmentPath);

    // shader Program
    ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);

    if (!geometryCode.empty()) {
        const char *gShaderCode = geometryCode.c_str();

        geometry = glCreateShader(GL_GEOMETRY_SHADER);
        glShaderSource(geometry, 1, &gShaderCode, NULL);
        glCompileShader(geometry);
        checkCompilerErrors(geometry, geometryPath);

        glAttachShader(ID, geometry);
    }

    glLinkProgram(ID);
    checkCompilerErrors(ID, "PROGRAM");

    glDeleteShader(vertex);
    glDeleteShader(fragment);

    if (!geometryCode.empty()) {
        glDeleteShader(geometry);
    }

    // cache uniform variable positions
    int i = 0;
    for (std::string uniformName : SHADER_UNIFORMS) {
        uniformPositions[i++] = getUniformLocation(uniformName);
    }
}



Shader *Shader::load(std::string naziv) {
    if (loadedShaders.find(naziv) != loadedShaders.end()) {
        return loadedShaders[naziv];
    }

    std::string base = Shader::_baseDir + "/" + naziv;
    Shader *s = new Shader(base + ".vert", base + ".frag", base + ".geom");
    loadedShaders[naziv] = s;
    return s;
}

Shader::~Shader() { glDeleteProgram(ID); }

void Shader::use() { glUseProgram(ID); }

GLint Shader::getUniformLocation(const std::string &name) { return glGetUniformLocation(ID, name.c_str()); }

void Shader::setUniformByLocation(int location, int size, int *val) const { glUniform1iv(location, size, val); }

void Shader::setUniform(int index, int val) const { glUniform1i(uniformPositions[index], val); }

void Shader::setUniform(int index, float val) const {
    glUniform1f(uniformPositions[index], val); }

void Shader::setUniform(int index, int size, float array[3]) const {
    glUniform3fv(uniformPositions[index], size, array);
}

void Shader::setUniform(int index, int size, glm::vec3 vector) const {
    glUniform3fv(uniformPositions[index], size, glm::value_ptr(vector));
}

void Shader::setUniform(int index, int size, std::vector<float> vector) const {
    glUniform3fv(uniformPositions[index], size, &vector[0]);
}

void Shader::setUniform(int index, int size, glm::mat4 matrix) const {
    glUniformMatrix4fv(uniformPositions[index], size, GL_FALSE, glm::value_ptr(matrix));
}
