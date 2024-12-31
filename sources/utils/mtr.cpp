#include "utils/mtr.h"
#include "glm/common.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/random.hpp>
#include <glm/gtx/string_cast.hpp>

#include <cmath>

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

bool isPointInAABB2D(glm::vec2 point, glm::vec2 boxc1, glm::vec2 boxc2) {
    glm::vec2 boxMin = glm::min(boxc1, boxc2);
    glm::vec2 boxMax = glm::max(boxc1, boxc2);

    return point.x >= boxMin.x && point.x <= boxMax.x && point.y >= boxMin.y && point.y <= boxMax.y;
}

bool intersectAABB2D(glm::vec2 box1c1, glm::vec2 box1c2, glm::vec2 box2c1, glm::vec2 box2c2) {
    glm::vec2 box1Min = glm::min(box1c1, box1c2);
    glm::vec2 box1Max = glm::max(box1c1, box1c2);
    glm::vec2 box2Min = glm::min(box2c1, box2c2);
    glm::vec2 box2Max = glm::max(box2c1, box2c2);

    if (box1Max.x < box2Min.x || box1Min.x > box2Max.x)
        return false;

    if (box1Max.y < box2Min.y || box1Min.y > box2Max.y)
        return false;

    return true;
}

bool intersectAABB3D(glm::vec3 box1c1, glm::vec3 box1c2, glm::vec3 box2c1, glm::vec3 box2c2) {
    glm::vec3 box1Min = glm::min(box1c1, box1c2);
    glm::vec3 box1Max = glm::max(box1c1, box1c2);
    glm::vec3 box2Min = glm::min(box2c1, box2c2);
    glm::vec3 box2Max = glm::max(box2c1, box2c2);

    if (box1Max.x < box2Min.x || box1Min.x > box2Max.x)
        return false;

    if (box1Max.y < box2Min.y || box1Min.y > box2Max.y)
        return false;

    if (box1Max.z < box2Min.z || box1Min.z > box2Max.z)
        return false;

    return true;
}

bool intersectOBB2D(glm::vec2 box1c1, glm::vec2 box1c2, glm::vec2 box2c1, glm::vec2 box2c2) {
    // Calculate the centers of the boxes
    glm::vec2 center1 = (box1c1 + box1c2) / 2.0f;
    glm::vec2 center2 = (box2c1 + box2c2) / 2.0f;

    // Calculate the half extents
    glm::vec2 halfExtent1 = glm::abs(box1c2 - box1c1) / 2.0f;
    glm::vec2 halfExtent2 = glm::abs(box2c2 - box2c1) / 2.0f;

    // Calculate the axes of the first box
    glm::vec2 axis1 = glm::normalize(box1c2 - box1c1);
    glm::vec2 axis2 = glm::vec2(-axis1.y, axis1.x); // Perpendicular axis

    // Calculate the axes of the second box
    glm::vec2 axis3 = glm::normalize(box2c2 - box2c1);
    glm::vec2 axis4 = glm::vec2(-axis3.y, axis3.x); // Perpendicular axis

    // All potential separating axes
    glm::vec2 axes[] = {axis1, axis2, axis3, axis4};

    // Check for overlap on each axis
    for (const auto &axis : axes) {
        // Project the centers onto the axis
        float projection1 = glm::dot(center1, axis);
        float projection2 = glm::dot(center2, axis);

        // Project the half extents onto the axis
        float radius1 = glm::dot(halfExtent1, glm::abs(axis));
        float radius2 = glm::dot(halfExtent2, glm::abs(axis));

        // Check for a gap
        if (std::abs(projection1 - projection2) > (radius1 + radius2)) {
            return false; // Separating axis found
        }
    }

    return true; // No separating axis found, the boxes intersect
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

unsigned int intersectLineAndSphere(glm::vec3 origin, glm::vec3 direction, glm::vec3 center, float radius, float &t1,
                                    float &t2) {
    // glm::vec3 dir = glm::normalize(direction);
    glm::vec3 dir = direction;

    glm::vec3 oc = origin - center;
    float a = glm::dot(dir, dir); // 1 if dir is normalized
    float b = 2.0f * glm::dot(oc, dir);
    float c = glm::dot(oc, oc) - radius * radius;

    float discriminant = b * b - 4 * a * c;

    if (discriminant < 0) {
        return 0;
    }

    // Calculate the intersection points
    float sqrt_discriminant = std::sqrt(discriminant);
    t1 = (-b - sqrt_discriminant) / (2.0f * a);
    t2 = (-b + sqrt_discriminant) / (2.0f * a);

    return discriminant == 0 ? 1 : 2;
}

glm::vec3 linearRandVec3(float v1, float v2) {
    glm::vec3 v(0);
    for (int i = 0; i < 3; i++) {
        v[i] = glm::linearRand(v1, v2);
    }
    return v;
}
}; // namespace mtr