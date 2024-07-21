#include "SphereObject.h"

#include <iostream>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

SphereObject::SphereObject(std::string name, glm::vec3 center, float radius, glm::vec3 color) : MeshObject(name, new Mesh(), Shader::Load("phong")), color(color) {
    sphere = new Sphere(center, radius);
    generateSphere();
    mesh->commit();
}

SphereObject::~SphereObject() {
    delete mesh;
    delete sphere;
}

std::optional<Intersection> SphereObject::findIntersection(glm::vec3 origin, glm::vec3 direction) {
    float t1, t2;
    unsigned int koliko = mtr::intersectLineAndSphere(origin, direction, sphere->center, sphere->radius, t1, t2);
    if (koliko == 2 && t2 > 0 && t2 < t1) {
        glm::vec3 point = origin + t2 * direction;
        return Intersection(t2, point, color, glm::normalize(point - sphere->center));
    }
    if (koliko > 0 && t1 > 0) {
        glm::vec3 point = origin + t1 * direction;
        return Intersection(t1, point, color, glm::normalize(point - sphere->center));
    }
    return std::nullopt;
}

const int STACKS = 30;
const int SLICES = 30;

void SphereObject::generateSphere() {
    for (int i = 0; i <= STACKS; ++i) {
        float V = (float)i / (float)STACKS;
        float phi = V * M_PI;

        for (int j = 0; j <= SLICES; ++j) {
            float U = (float)j / (float)SLICES;
            float theta = U * (M_PI * 2);

            glm::vec3 unit(cosf(theta) * sinf(phi), cosf(phi), sinf(theta) * sinf(phi));

            mesh->addVertex(sphere->center + unit * sphere->radius, color);
            mesh->addNormal(unit);
        }
    }

    for (int i = 0; i < SLICES * STACKS + SLICES; ++i) {
        mesh->addIndices(i, i + SLICES + 1, i + SLICES);
        mesh->addIndices(i + SLICES + 1, i, i + 1);
    }
    mesh->addIndices(1, 2, 0);
}
