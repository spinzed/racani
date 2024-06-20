#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

#include <iostream>
#include <glm/gtc/random.hpp>

namespace mtr {
glm::mat4 translate3D(glm::vec3 translateVector) {
    glm::mat4 m = glm::mat4(1);
    m[3][0] = translateVector[0];
    m[3][1] = translateVector[1];
    m[3][2] = translateVector[2];
    return m;
}

glm::mat4 scale3D(glm::vec3 scaleVector) {
    glm::mat4 m = glm::mat4(1);
    m[0][0] = scaleVector[0];
    m[1][1] = scaleVector[1];
    m[2][2] = scaleVector[2];
    return m;
}

glm::mat4 scale3D(float factor) { return scale3D(glm::vec3(factor, factor, factor)); }

glm::mat4 rotate3D(glm::vec3 axis, float angle) {
    glm::vec3 norm = glm::normalize(axis);
    float c = glm::cos(angle);
    float s = glm::sin(angle);
    float t = 1 - c;

    float x = norm[0];
    float y = norm[1];
    float z = norm[2];

    // Build the rotation matrix
    return glm::mat4(x * x + (1 - x * x) * c, t * x * y + s * z, t * x * z - s * y, 0.0f, // stupac
                     t * x * y - s * z, y * y + (1 - y * y) * c, t * y * z + s * x, 0.0f, t * x * z + s * y,
                     t * y * z - s * x, z * z + (1 - z * z) * c, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
}

glm::mat4 lookAtMatrix(glm::vec3 eye, glm::vec3 center, glm::vec3 viewUp) {
    glm::vec3 front = glm::normalize(eye - center);
    glm::vec3 right = glm::normalize(glm::cross(viewUp, front));
    glm::vec3 up = glm::normalize(glm::cross(front, right));

    glm::mat4 m(glm::vec4(right, -glm::dot(eye, right)), glm::vec4(up, -glm::dot(eye, up)),
                glm::vec4(front, -glm::dot(eye, front)), glm::vec4(0, 0, 0, 1));

    return glm::transpose(m);
}

glm::mat4 frustum(float left, float right, float bottom, float top, float nearp, float far) {
    float w = right - left;
    float h = top - bottom;
    float d = far - nearp;

    glm::mat4 m(2 * nearp / w, 0, (right + left) / w, 0, // stupac
                0, 2 * nearp / h, (top + bottom) / h, 0, 0, 0, -(far + nearp) / d, -1, 0, 0, -2 * far * nearp / d, 0);

    return m;
}

bool intersectLineAndTriangle(glm::vec3 origin, glm::vec3 direction, glm::vec3 vrh0, glm::vec3 vrh1, glm::vec3 vrh2,
                              float *t, float *u, float *v) {
    glm::vec3 brid1 = vrh1 - vrh0;
    glm::vec3 brid2 = vrh2 - vrh0;

    // glm::vec3 N = getNormal(i);
    glm::vec3 N = glm::cross(brid1, brid2);
    float det = -glm::dot(direction, N);

    if (det < 1e-6)
        return false;

    float invdet = 1.0f / det;

    glm::vec3 AO = origin - vrh0;
    glm::vec3 DAO = glm::cross(AO, direction);

    *u = glm::dot(brid2, DAO) * invdet;
    *v = -glm::dot(brid1, DAO) * invdet;
    *t = glm::dot(AO, N) * invdet;
    // std::cout << "----------" << std::endl;
    // std::cout << glm::to_string(origin) << glm::to_string(direction) << std::endl;
    // std::cout << glm::to_string(vrh0) << glm::to_string(vrh1) << glm::to_string(vrh2) << std::endl;
    // std::cout << glm::to_string(brid1) << glm::to_string(brid2)<< glm::to_string(N) << std::endl;
    // std::cout << det << " " << t  << " " << u << " " << v << std::endl;
    return *t >= 0.0f && *u >= 0.0f && *v >= 0.0f && (*u + *v) <= 1.0f;
    // return origin + t * direction;
}

// axis alignment bounding box
bool intersectLineAndAABB(glm::vec3 origin, glm::vec3 direction, glm::vec3 min, glm::vec3 max, glm::vec3 &point) {
    glm::vec3 minT = (min - origin) / direction;
    glm::vec3 maxT = (max - origin) / direction;

    glm::vec3 tMin = glm::min(minT, maxT);
    glm::vec3 tMax = glm::max(minT, maxT);

    float tEnter = glm::max(glm::max(tMin.x, tMin.y), tMin.z);
    float tExit = glm::min(glm::min(tMax.x, tMax.y), tMax.z);

    return tEnter <= tExit && tExit >= 0.0f && tEnter <= 1.0f;
}

bool intersetLineAndOBB(const glm::vec3 &origin, const glm::vec3 &direction, const glm::vec3 &boxCenter,
                        const glm::vec3 &boxHalfExtents, const glm::mat3 &boxOrientation) {
    glm::vec3 invDir = 1.0f / direction;

    glm::vec3 tMin = (boxCenter - origin) * invDir;
    glm::vec3 tMax = (boxCenter + boxHalfExtents - origin) * invDir;

    glm::vec3 t1 = glm::min(tMin, tMax);
    glm::vec3 t2 = glm::max(tMin, tMax);

    float tNear = glm::max(glm::max(t1.x, t1.y), t1.z);
    float tFar = glm::min(glm::min(t2.x, t2.y), t2.z);

    if (tNear > tFar || tFar < 0.0f) {
        return false;
    }

    glm::vec3 intersectionPoint = origin + direction * tNear;

    glm::vec3 localIntersection = glm::inverse(boxOrientation) * (intersectionPoint - boxCenter);

    if (glm::abs(localIntersection.x) > boxHalfExtents.x || glm::abs(localIntersection.y) > boxHalfExtents.y ||
        glm::abs(localIntersection.z) > boxHalfExtents.z) {
        return false;
    }

    return true;
}

glm::vec3 linearRandVec3(float v1, float v2) {
    glm::vec3 v(0);
    for (int i = 0; i < 3; i++) {
        v[i] = glm::linearRand(v1, v2);
    }
    return v;
}
}; // namespace mtr