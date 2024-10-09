#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Transform {
  public:
    constexpr Transform() = default;
    Transform(glm::mat4 m) { matrix = m; };

    // glm::mat4 matrix = glm::mat4(1);
    glm::mat4 matrix = glm::mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);

    glm::vec3 right();
    glm::vec3 up();
    glm::vec3 forward();
    glm::vec3 position();

    glm::vec3 vertical();
    glm::vec3 direction();
    glm::vec3 getScale();

    void setRight(glm::vec3 right);
    void setUp(glm::vec3 up);
    void setForward(glm::vec3 forward);

    void translate(glm::vec3 orientation);
    void setPosition(glm::vec3 position);
    void rotate(glm::vec3 axis, float degrees);
    void scale(glm::vec3 scale);
    void scale(float scale);
    void pointAt(glm::vec3 point);
    void pointAt(glm::vec3 point, glm::vec3 up);
    void pointAtDirection(glm::vec3 direction);
    void pointAtDirection(glm::vec3 direction, glm::vec3 up);
    void normalize(glm::vec3 min, glm::vec3 max);

void moveAlongSpline(glm::vec3 position, glm::vec3 tangent) {
    glm::vec3 forward = glm::normalize(tangent);
    
    // Define the up vector (Y-up, or adjust depending on your coordinate system)
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

    // Compute the right and recompute up vectors for correct orientation
    glm::vec3 right = glm::normalize(glm::cross(up, forward));
    up = glm::cross(forward, right);

    // Create the rotation matrix from the right, up, and forward vectors
    glm::mat4 rotationMatrix = glm::mat4(1.0f);
    rotationMatrix[0] = glm::vec4(right, 0.0f);
    rotationMatrix[1] = glm::vec4(up, 0.0f);
    rotationMatrix[2] = glm::vec4(forward, 0.0f);

    // Set the object's transformation matrix (position and rotation)
    glm::mat4 transform = glm::mat4(1.0f);
    transform = glm::translate(transform, position); // Apply position
    transform *= rotationMatrix;                     // Apply rotation

    // Set this matrix as the object's final transformation
    matrix = transform;
}


    static glm::mat4 frustum(float l, float r, float b, float t, float n, float f);
    static glm::mat4 perspective(int width, int height, float nearp, float far, float angleDeg);

    glm::highp_mat4 getMatrix();
    void setMatrix(glm::mat4 m) { matrix = m; };

    glm::vec3 getEulerAngles(); // pitch, jaw, roll
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
