#include "SphereObject.h"

#include <iostream>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

SphereObject::SphereObject(glm::vec3 center, float radius, glm::vec3 color): color(color) {
    sphere = new Sphere(center, radius);
    shader = Shader::load("phong");
    mesh = new Mesh();
    generateSphere();
    mesh->updateBufferData();
}

SphereObject::~SphereObject() {
    delete mesh;
    delete sphere;
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

            float x = sphere->center.x + cosf(theta) * sinf(phi) * sphere->radius;
            float y = sphere->center.y + cosf(phi) * sphere->radius;
            float z = sphere->center.z + sinf(theta) * sinf(phi) * sphere->radius;

            mesh->addVertex(x, y, z, color.r, color.g, color.b);
        }
    }

    for (int i = 0; i < SLICES * STACKS + SLICES; ++i) {
        mesh->addIndices(i, i + SLICES + 1, i + SLICES);
        mesh->addIndices(i + SLICES + 1, i, i + 1);
    }
    mesh->addIndices(1, 2, 0);
    mesh->updateBufferData();
}
