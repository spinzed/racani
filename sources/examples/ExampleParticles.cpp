#include "examples/ExampleParticles.h"

// Local Headers
#include "models/Mesh.h"
#include "objects/MeshObject.h"
#include "objects/Sphere.h"
#include "renderer/Camera.h"
#include "renderer/Cubemap.h"
#include "renderer/Input.h"
#include "renderer/ParticleSystem.h"
#include "renderer/Shader.h"
#include "renderer/Transform.h"
#include "renderer/WindowManager.h"
#include "utils/PerlinNoise.h"

// System Headers
#include "glm/common.hpp"
#include "glm/geometric.hpp"
#include "glm/gtc/random.hpp"
#include <GLFW/glfw3.h>
#include <glad/glad.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

#include <cstdio>
#include <cstdlib>
#include <numbers>

class PC5 : public ParticleCluster {
  public:
    float pc5hs = 0.1f;
    float pc5mvs = 0.1f;
    float pc5vs = pc5mvs;
    glm::vec3 pc5Middle = glm::vec3(0);

    PC5(glm::vec3 mid, bool oneTime, int howMany) : ParticleCluster(howMany) {
        getTransform()->translate(mid);
        setOnReset([&](ParticleCluster *pc, ParticleUpdate _) {
            for (int i = 0; i < pc->n; ++i) {
                pc5vs = pc5mvs;
                glm::vec3 val = pc5Middle + glm::sphericalRand(0.5f);
                glm::vec3 color = i >= pc->n * 0.2 ? glm::vec3(0.5, 1, 0.5) : glm::vec3(0.5, 0.5, 1);
                pc->setPoint(i, val, color);
            }
        });
        setOnChange([&](ParticleCluster *pc, ParticleUpdate data) {
            float t = data.t;
            pc->setPointSize(3 - 3 * t);
            for (int i = 0; i < pc->mesh->numberOfVertices(); ++i) {
                glm::vec3 norm = pc->mesh->getVertex(i) + pc5Middle;
                glm::vec3 right = glm::cross(norm, TransformIdentity::up());
                glm::vec3 forward = pc5hs * glm::normalize(glm::cross(TransformIdentity::up(), right));
                glm::vec3 total = forward * pc5hs + TransformIdentity::up() * pc5vs;
                pc->mesh->setVertex(i, pc->mesh->getVertex(i) + total);
            }
            pc5vs -= data.deltaTime * 0.15;
        });
        setDuration(2.0f);
        setBehavior(oneTime ? ParticleClusterBehavior::ONE_TIME_THEN_DESTROY : ParticleClusterBehavior::REPEAT);
        setPointSize(5);
    }
};

void ExampleParticles::KeyCallback(InputGlobalListenerData event) {
    if (event.action != GLFW_PRESS && event.action != GLFW_REPEAT)
        return;

    if (event.key == GLFW_KEY_N && event.action == GLFW_PRESS) {
        PC5 *pc = new PC5(glm::vec3(2.5, 1.6, -18), true, howMany); // leakage
        // std::unique_ptr<PC5> pc = std::make_unique<PC5>(glm::vec3(2.5, 1.6, -18), true);
        renderer->AddParticleCluster(pc);
    }
    if (Input::checkKeyEvent(GLFW_KEY_V, GLFW_PRESS)) {
        renderer->vsync ? renderer->DisableVSync() : renderer->EnableVSync();
        std::cout << "Vsync toggled" << std::endl;
    }
}

void ExampleParticles::cursorPositionCallback(WindowCursorEvent event) {
    if (!renderer->manager->focused)
        return;

    int dx = (float)width / 2 - event.xpos;
    int dy = (float)height / 2 - event.ypos;
    Camera *camera = renderer->GetCamera();

    camera->rotate(mouseSensitivity * dx, mouseSensitivity * dy);
    camera->recalculateMatrix();

    renderer->manager->CenterCursor();
}

int ExampleParticles::run(std::string execDirectory) {
    renderer = new Renderer(width, height, execDirectory);

    renderer->input.addMouseListener([](MouseClickOptions _) {});
    renderer->input.addKeyEventListener([&](auto a) { KeyCallback(a); });
    renderer->manager->SetCursorHidden(true);

    renderer->manager->setCursorCallback([&](auto a) { cursorPositionCallback(a); });
    renderer->manager->setWindowFocusCallback([&](auto data) {
        // FIXME: when clicking on window focused goes to 1, but it doesn't change the cursor pos
        if (data.focused) {
            renderer->manager->CenterCursor();
        }
    });

    /*********************************************************************************************/
    Camera *camera = renderer->GetCamera();
    camera->translate(glm::vec3(3.0f, 3.0f, -3.0f));
    camera->rotate(145, -30);
    camera->recalculateMatrix();

    Shader *phongShader = Shader::Load("phong");

    Mesh *kockaMesh2 = Mesh::Load("kocka", glm::vec3(0.2, 0.2, 0.8));
    Mesh *kockaMesh3 = Mesh::Load("kocka", glm::vec3(0.8, 0.8, 0.8));
    MeshObject *floor = new MeshObject("pod", kockaMesh3, phongShader);
    kockaMesh2->material->colorTransmitive = glm::vec3(0.5);

    floor->getTransform()->translate(glm::vec3(0.0f, -2.0f, 0.0f));
    floor->getTransform()->scale(glm::vec3(300.0f, 0.1f, 300.0f));
    renderer->AddObject(floor);

    Light *light = new PointLight(glm::vec3(-3, 3, 2), glm::vec3(1, 1, 1), glm::vec3(1, 1, 1));
    renderer->AddLight(light);

    cameraCurve = new BSpline();
    renderer->AddObject(cameraCurve);

    Cubemap skybox = Cubemap::Load({
        "skybox/right.png",
        "skybox/left.png",
        "skybox/top.png",
        "skybox/bottom.png",
        "skybox/front.png",
        "skybox/back.png",
    });
    renderer->SetSkybox(&skybox);

    ParticleCluster pc(howMany);
    pc.setOnReset([](auto pc, ParticleUpdate _) {
        for (int i = 0; i < pc->n; ++i) {
            float d1 = i % 10;
            float d2 = (i / 10) % 10;
            float d3 = (i / 100) % 10;

            glm::vec3 val = glm::vec3(2.5, 1.6, -1.8) + glm::vec3(d1, d2, d3) * (4.0f / 100);
            pc->setPoint(i, val);
        }
    });
    pc.setOnChange([&](ParticleCluster *pc, ParticleUpdate data) {
        for (int i = 0; i < pc->mesh->numberOfVertices(); ++i) {
            auto vertex = pc->mesh->getVertex(i) + 0.5f * data.deltaTime * glm::vec3(1);
            pc->mesh->setVertex(i, vertex);
        }
    });
    pc.setDuration(2.0);
    pc.setBehavior(ParticleClusterBehavior::REPEAT);
    renderer->AddParticleCluster(&pc);

    Sphere sfera("sfera", glm::vec3(0), 0.1f);
    ParticleCluster pc2(sfera.getMesh()->numberOfVertices());
    pc2.getTransform()->translate(glm::vec3(2.5, 1.6, -4));
    pc2.setOnInit([&sfera](auto pc, ParticleUpdate _) {
        for (int i = 0; i < pc->n; ++i) {
            glm::vec3 val = sfera.getMesh()->getVertex(i);
            pc->addPoint(val);
        }
    });
    pc2.setOnChange([&](auto pc, ParticleUpdate data) {
        float t = data.t;
        pc->getTransform()->setScale(0.001f + 10 * t);
        for (int i = 0; i < pc->mesh->numberOfVertices(); ++i) {
            float a = (float)i / pc->mesh->numberOfVertices();
            pc->mesh->setColor(i, glm::vec3(1, 0, a) * t + glm::vec3(0, 1, a) * (1.0f - t));
        }
    });
    pc2.setDuration(2.0);
    pc2.setBehavior(ParticleClusterBehavior::REPEAT);
    pc2.setPointSize(3);
    renderer->AddParticleCluster(&pc2);

    ParticleCluster pc3(howMany);
    pc3.getTransform()->translate(glm::vec3(2.5, 1.6, -10));
    glm::vec3 pc3Middle = glm::vec3(0);
    pc3.setOnInit([&pc3Middle](auto pc, ParticleUpdate _) {
        // return glm::linearRand(pc3Middle - glm::vec3(0.5), pc3Middle + glm::vec3(0.5));
        for (int i = 0; i < pc->n; ++i) {
            glm::vec3 val = pc3Middle + glm::sphericalRand(0.5f);
            pc->addPoint(val);
        }
    });
    pc3.setOnChange([&](auto pc, ParticleUpdate data) {
        float t = data.t;
        pc->setPointSize(3 - 3 * t);
        for (int i = 0; i < pc->mesh->numberOfVertices(); ++i) {
            glm::vec3 norm = glm::normalize(pc->mesh->getVertex(i) - pc3Middle);
            pc->mesh->setVertex(i, pc3Middle + norm * (0.01f + t));
        }
    });
    pc3.setDuration(2.0f);
    pc3.setBehavior(ParticleClusterBehavior::REPEAT_ALTERNATE);
    pc3.setPointSize(3);
    renderer->AddParticleCluster(&pc3);

    ParticleCluster pc4(howMany);
    glm::vec3 pc4Middle = glm::vec3(0);
    std::vector<float> pc4Speeds(howMany);
    pc4.getTransform()->translate(glm::vec3(2.5, 1.6, -7));
    pc4.setOnReset([&pc4Middle, &pc4Speeds](ParticleCluster *pc, auto _) {
        for (int i = 0; i < pc->n; ++i) {
            pc4Speeds[i] = 1e-3f * static_cast<float>(rand()) / RAND_MAX;
            glm::vec3 val = 1e-3f * glm::normalize(glm::sphericalRand(0.5f) - pc4Middle);
            pc->setPoint(i, val);
        }
        for (int i = 0; i < pc->mesh->numberOfVertices(); i++) {
            pc->mesh->setColor(i, i >= pc->n * 0.2 ? glm::vec3(0.8, 0.8, 1) : glm::vec3(1, 0.8, 0.8));
        }
    });
    // pc4.setOnReset([](auto pc, ParticleUpdate data) {});
    pc4.setOnChange([&](auto pc, ParticleUpdate data) {
        float t = data.t;
        pc->setPointSize(3 - 3 * t);
        for (int i = 0; i < pc->mesh->numberOfVertices(); ++i) {
            glm::vec3 norm = glm::normalize(pc->mesh->getVertex(i) - pc4Middle);
            pc->mesh->setVertex(i, pc->mesh->getVertex(i) + norm * data.deltaTime * 1000.0f * pc4Speeds[i]);
        }
    });
    pc4.setDuration(2.0f);
    pc4.setBehavior(ParticleClusterBehavior::REPEAT);
    pc4.setPointSize(3);
    renderer->AddParticleCluster(&pc4);

    PC5 pc5(glm::vec3(2.5, 1.6, -13), false, howMany);

    renderer->AddParticleCluster(&pc5);

    ParticleCluster pc6(howMany);
    pc6.getTransform()->translate(glm::vec3(2.5, 1.6, -15));
    glm::vec3 pc6Middle = glm::vec3(0);
    std::vector<glm::vec3> pc5InitialPos;
    float pc6vs = 0.1f;
    pc6.setOnInit([pc6Middle, &pc5InitialPos](ParticleCluster *pc, ParticleUpdate _) {
        for (int i = 0; i < pc->n; ++i) {
            glm::vec3 val = pc6Middle + glm::sphericalRand(0.5f);
            pc->setPoint(i, val, glm::vec3(0.8, 0.3, 0.3));
            pc5InitialPos.push_back(val);
        }
    });
    pc6.setOnReset([pc6Middle, &pc5InitialPos](ParticleCluster *pc, auto _) {
        for (int i = 0; i < pc->n; ++i) {
            glm::vec3 val = pc6Middle + pc5InitialPos[i];
            pc->setPoint(i, val, glm::vec3(0.8, 0.3, 0.3));
        }
    });
    pc6.setOnChange([&](auto pc, ParticleUpdate data) {
        float t = data.t * pc->direction;
        float sint = glm::sin(t * 2 * std::numbers::pi) * data.deltaTime;
        for (int i = 0; i < pc->mesh->numberOfVertices(); ++i) {
            float index = (float)i / pc->mesh->numberOfVertices();
            float t2 = glm::clamp(2.0f * data.t - 1.0f * index, 0.0f, 1.0f) * pc->direction;
            float sint2 = -glm::sin(t2 * std::numbers::pi) * data.deltaTime * 2;
            pc->mesh->setVertex(i, pc->mesh->getVertex(i) + glm::vec3(0, sint, sint2));
        }
        pc6vs -= data.deltaTime * 0.15;
    });
    pc6.setDuration(2.0f);
    pc6.setBehavior(ParticleClusterBehavior::REPEAT_ALTERNATE);
    pc6.setPointSize(5);
    renderer->AddParticleCluster(&pc6);

    renderer->input.addPerFrameListener([&](auto a) {
        float deltaTime = a.deltaTime;

        float multiplier = moveSensitivity * deltaTime;
        // camera->getTransform()->translate(camera->getTransform()->forward() * 0.01f);
        // camera->recalculateMatrix();

        if (Input::checkKeyEvent(GLFW_KEY_LEFT_CONTROL, GLFW_PRESS)) {
            multiplier *= sprintMultiplier;
        }
        if (Input::checkKeyEvent(GLFW_KEY_W, GLFW_PRESS)) {
            // camera->translate(multiplier * Transform::Identity().forward());
            camera->setPosition(camera->position() + multiplier * camera->forward());
            camera->recalculateMatrix();
            // camera->translate(multiplier * camera->forward());
        }
        if (Input::checkKeyEvent(GLFW_KEY_A, GLFW_PRESS)) {
            camera->translate(multiplier * -TransformIdentity::right());
            camera->recalculateMatrix();
        }
        if (Input::checkKeyEvent(GLFW_KEY_S, GLFW_PRESS)) {
            camera->translate(multiplier * -TransformIdentity::forward());
            camera->recalculateMatrix();
        }
        if (Input::checkKeyEvent(GLFW_KEY_D, GLFW_PRESS)) {
            camera->translate(multiplier * TransformIdentity::right());
            camera->recalculateMatrix();
        }
        if (Input::checkKeyEvent(GLFW_KEY_SPACE, GLFW_PRESS)) {
            camera->translate(multiplier * camera->vertical());
            camera->recalculateMatrix();
        }
        if (Input::checkKeyEvent(GLFW_KEY_LEFT_SHIFT, GLFW_PRESS)) {
            camera->translate(-multiplier * camera->vertical());
            camera->recalculateMatrix();
        }
        if (Input::checkKeyEvent(GLFW_KEY_ESCAPE, GLFW_PRESS)) {
            renderer->SetShouldClose();
        }
    });

    renderer->Loop();

    return EXIT_SUCCESS;
}
