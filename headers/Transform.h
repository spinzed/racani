#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Transform {
  public:
    // glm::mat4 matrix = glm::mat4(1);
    glm::mat4 matrix = glm::mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);

    glm::vec3 right();
    glm::vec3 forward();
    glm::vec3 up();
    glm::vec3 position();

    glm::vec3 vertical();

    void translate(glm::vec3 orientation);
    void setPosition(glm::vec3 position);
    void rotate(glm::vec3 axis, float degrees);
    void scale(glm::vec3 scale);
    void scale(float scale);
    void lookAt(glm::vec3 point);
    void lookAt(glm::vec3 point, glm::vec3 up);
    void normalize(float minX, float minY, float minZ, float maxX, float maxY, float maxZ);

    static glm::mat4 frustum(float l, float r, float b, float t, float n, float f);
    static glm::mat4 perspective(int width, int height, float nearp, float far, float angleDeg);

    //static constexpr Transform Identity() { return Transform(); };

    glm::highp_mat4 getMatrix();
    void setMatrix(glm::mat4 m) { matrix = m; };
};

class TransformIdentity : public Transform {
  private:
    static constexpr Transform singleton() { return Transform(); };

    TransformIdentity() {};
  public:
    static glm::vec3 right() { return glm::transpose(singleton().matrix)[0]; }
    static glm::vec3 up() { return glm::transpose(singleton().matrix)[1]; }
    static glm::vec3 forward() { return -glm::transpose(singleton().matrix)[2]; }
    static glm::vec3 position() { return singleton().matrix[3]; }
    static glm::vec3 vertical() { return singleton().vertical(); }
    static glm::mat4 getMatrix() { return singleton().getMatrix(); }
};
