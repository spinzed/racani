#include "examples/example1.h"

#if false

// Local Headers
#include "models/Mesh.h"
#include "objects/BSpline.h"
#include "objects/MeshObject.h"
#include "objects/Sphere.h"
#include "renderer/Animation.h"
#include "renderer/Animator.h"
#include "renderer/Camera.h"
#include "renderer/Cubemap.h"
#include "renderer/Input.h"
#include "renderer/ParticleSystem.h"
#include "renderer/Renderer.h"
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

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <numbers>

int width = 500, height = 500;
float moveSensitivity = 3, sprintMultiplier = 5, mouseSensitivity = 0.15f;

Renderer *renderer;

Transform *movingObject;
Transform *movingObject2;
PolyLine *tangenta;

class MoveAnimation : public Animation {
  public:
    using Animation::Animation;
    ~MoveAnimation() {};

    Transform *tr;

    MoveAnimation(BSpline *c, Transform *tr, float duration) : Animation(c, duration) { this->tr = tr; }

    void onChange(float current, float total) override {
        BSpline *c = (BSpline *)curve;
        float t = current / total;
        glm::vec3 point = curve->evaluatePoint(t);
        glm::vec3 forward = c->evaluateTangent(t);

        tr->setPosition(point);
        tr->pointAtDirection(forward, TransformIdentity::up());

        tangenta->reset();
        tangenta->addPoint(point);
        tangenta->addPoint(point + forward);
        tangenta->commit();
    }
};

class PC5 : public ParticleCluster {
  public:
    float pc5hs = 0.1f;
    float pc5mvs = 0.1f;
    float pc5vs = pc5mvs;
    glm::vec3 pc5Middle = glm::vec3(0);
    PC5(glm::vec3 mid, bool oneTime) : ParticleCluster(1000) {
        getTransform()->translate(mid);
        setOnReset([&](ParticleCluster *pc, ParticleUpdate data) {
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

bool axis = false;

BSpline *cameraCurve = nullptr;
BSpline *helix = nullptr;

void KeyCallback(InputGlobalListenerData event) {
    if (event.action != GLFW_PRESS && event.action != GLFW_REPEAT)
        return;
    Camera *camera = renderer->GetCamera();

    if (event.key == GLFW_KEY_G) {
        // std::cout << "axis changed" << std::endl;
        axis = !axis;
    } else if (event.key == GLFW_KEY_1 && event.action == GLFW_PRESS) {
        renderer->setRenderingMethod(RenderingMethod::Rasterize);
        std::cout << "Set to rasterize" << std::endl;
    } else if (event.key == GLFW_KEY_2 && event.action == GLFW_PRESS) {
        renderer->setRenderingMethod(RenderingMethod::Raytrace);
        std::cout << "Set to raytrace" << std::endl;
    } else if (event.key == GLFW_KEY_3 && event.action == GLFW_PRESS) {
        renderer->setRenderingMethod(RenderingMethod::Pathtrace);
        std::cout << "Set to pathtrace" << std::endl;
    } else if (event.key == GLFW_KEY_E && event.action == GLFW_PRESS) {
        renderer->setIntegrationEnabled(!renderer->integrationEnabled());
        renderer->resetStats();
        std::string status = renderer->integrationEnabled() ? "on" : "off";
        std::cout << "Automatic RT rerendering with integration: " << status << std::endl;
        std::cout << "Stats have been reset" << std::endl;
        //} else if (event.key == GLFW_KEY_UP) {
        //    renderer->setDepth(renderer->getDepth() + 1);
        //    std::cout << "Set depth to " << renderer->getDepth() << std::endl;
        //} else if (event.key == GLFW_KEY_DOWN) {
        //    if (renderer->getDepth() > 1) {
        //        renderer->setDepth(renderer->getDepth() - 1);
        //        std::cout << "Set depth to " << renderer->getDepth() << std::endl;
        //    }
        //} else if (event.key == GLFW_KEY_LEFT) {
        //    renderer->setKSpecular(std::max(renderer->kSpecular() - 0.05f, 0.0f));
        //    std::cout << "Set reflection factor to " << renderer->kSpecular() << std::endl;
        //} else if (event.key == GLFW_KEY_RIGHT) {
        //    renderer->setKSpecular(std::min(renderer->kSpecular() + 0.05f, 1.0f));
        //    std::cout << "Set reflection factor to " << renderer->kSpecular() << std::endl;
    } else if (event.key == GLFW_KEY_PAGE_DOWN) {
        renderer->setKRougness(std::max(renderer->kRoughness() - 0.01f, 0.0f));
        std::cout << "Set roughness factor to " << renderer->kRoughness() << std::endl;
    } else if (event.key == GLFW_KEY_PAGE_UP) {
        renderer->setKRougness(std::min(renderer->kRoughness() + 0.01f, 1.0f));
        if (renderer->kRoughness() > 1)
            renderer->setKSpecular(1);
        std::cout << "Set roughness factor to " << renderer->kRoughness() << std::endl;
    } else if (event.key == GLFW_KEY_H) {
        std::cout << "##################" << std::endl;
        std::cout << "Forward:  " << glm::to_string(camera->forward()) << std::endl;
        std::cout << "Up:     " << glm::to_string(camera->up()) << std::endl;
        std::cout << "Right:  " << glm::to_string(camera->right()) << std::endl;
        std::cout << "Position: " << glm::to_string(camera->position()) << std::endl;
        std::cout << "#################" << std::endl;
    } else if (event.key == GLFW_KEY_C && event.action == GLFW_PRESS) {
        cameraCurve->addControlPoint(camera->position());
        cameraCurve->finish();
        std::cout << "Added control point" << std::endl;
    } else if (event.key == GLFW_KEY_P && event.action == GLFW_PRESS) {
        std::cout << "Started animation" << std::endl;
        Animation *a = new MoveAnimation(cameraCurve, movingObject, 3000.0f);
        Animation *b = new MoveAnimation(helix, movingObject2, 8000.0f);
        Animator::registerAnimation(a);
        Animator::registerAnimation(b);
    } else if (event.key == GLFW_KEY_N && event.action == GLFW_PRESS) {
        PC5 *pc = new PC5(glm::vec3(2.5, 1.6, -18), true); // leakage
        // std::unique_ptr<PC5> pc = std::make_unique<PC5>(glm::vec3(2.5, 1.6, -18), true);
        renderer->AddParticleCluster(pc);
    }
}

void cursorPositionCallback(WindowCursorEvent event) {
    if (!renderer->manager->focused)
        return;

    int dx = (float)width / 2 - event.xpos;
    int dy = (float)height / 2 - event.ypos;
    Camera *camera = renderer->GetCamera();

    camera->rotate(mouseSensitivity * dx, mouseSensitivity * dy);
    camera->recalculateMatrix();

    renderer->manager->CenterCursor();
}

int example1(std::string execDirectory) {
    renderer = new Renderer(width, height, execDirectory);

    renderer->input.addMouseListener([](MouseClickOptions _) {});
    renderer->input.addGlobalListener(KeyCallback);
    renderer->input.hideCursor();

    renderer->manager->setCursorCallback(cursorPositionCallback);
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

    // Mesh *glavaMesh = Mesh::Load("glava");
    // MeshObject *glava = new MeshObject("glavaRobota", glavaMesh, phongShader);

    // glm::vec3 min, max;
    // glavaMesh->getBoundingBox(min, max);
    // glava->getTransform()->normalize(min, max);
    // glava->getTransform()->translate(glm::vec3(-1, 0, 0));

    // renderer->AddObject(glava);

    Mesh *kockaMesh = Mesh::Load("kocka", glm::vec3(1, 0.2, 0.3));
    Mesh *kockaMesh2 = Mesh::Load("kocka", glm::vec3(0.2, 0.2, 0.8));
    Mesh *kockaMesh3 = Mesh::Load("kocka", glm::vec3(0.8, 0.8, 0.8));
    Mesh *kockaMesh4 = Mesh::Load("kocka", glm::vec3(1, 1, 1));
    Mesh *kockaMesh5 = Mesh::Load("kocka", glm::vec3(1, 0, 0));
    MeshObject *kocka = new MeshObject("kocka1", kockaMesh, phongShader);
    MeshObject *kocka2 = new MeshObject("kocka2", kockaMesh2, phongShader);
    MeshObject *floor = new MeshObject("pod", kockaMesh3, phongShader);
    MeshObject *zid1 = new MeshObject("zid1", kockaMesh4, phongShader);
    MeshObject *zid2 = new MeshObject("zid2", kockaMesh4, phongShader);
    MeshObject *strop = new MeshObject("strop", kockaMesh5, phongShader);
    kockaMesh2->material->colorTransmitive = glm::vec3(0.5);

    // BoundingBox box1 = kockaMesh->getBoundingBox();
    //  kocka->getTransform()->normalize(box1.minX, box1.minY, box1.minZ, box1.maxX, box1.maxY, box1.maxZ);
    //  kocka->getTransform()->translate(glm::vec3(2, 1, 0));
    kocka2->getTransform()->translate(glm::vec3(1.0f, 1.0f, 1.0f));
    kocka2->getTransform()->rotate(TransformIdentity::up(), 45.0f);

    floor->getTransform()->translate(glm::vec3(0.0f, -2.0f, 0.0f));
    // floor->getTransform()->rotate(TransformIdentity::up(), 45.0f);
    floor->getTransform()->scale(glm::vec3(300.0f, 0.1f, 300.0f));

    zid1->getTransform()->translate(glm::vec3(0.0f, 0, 7.0f));
    zid1->getTransform()->scale(glm::vec3(8.0f, 3.2f, 1.0f));

    zid2->getTransform()->translate(glm::vec3(-7.0f, 0, 0.0f));
    zid2->getTransform()->scale(glm::vec3(1.0f, 3.2f, 8.0f));

    strop->getTransform()->translate(glm::vec3(0.0f, 3.2, 0.0f));
    strop->getTransform()->scale(glm::vec3(8.0f, 0.05f, 8.0f));

    renderer->AddObject(kocka);
    renderer->AddObject(kocka2);
    renderer->AddObject(floor);
    renderer->AddObject(zid1);
    renderer->AddObject(zid2);
    renderer->AddObject(strop);

    Light *light = new PointLight(glm::vec3(-3, 3, 2), glm::vec3(1, 1, 1), glm::vec3(1, 1, 1));
    renderer->AddLight(light);
    movingObject = light->getTransform();

    // ###################
    // # DODATNI OBJEKTI #
    // ###################

    // Mesh *catMesh = Mesh::Load("cat");
    //  Object *cat = new Object(catMesh, phongShader);

    // struct BoundingBox box2 = catMesh->getBoundingBox();
    // cat->getTransform()->normalize(box2.minX, box2.minY, box2.minZ, box2.maxX, box2.maxY, box2.maxZ);
    // cat->getTransform()->setPosition(glm::vec3(-2, 1, -1));
    // cat->getTransform()->rotate(TransformIdentity::right(), -90.0f);

    // renderer->AddObject(cat);

    // Sphere *zutaKugla = new Sphere("zutaKugla", glm::vec3(-2, 2, -4), 2, glm::vec3(1, 1, 0));
    // zutaKugla->mesh->material->colorReflective = glm::vec3(0.5);
    // renderer->AddObject(zutaKugla);

    // Sphere *prozirnaKugla = new Sphere("zutaKugla", glm::vec3(5, 2, 4), 2, glm::vec3(0.3, 1, 0));
    // prozirnaKugla->mesh->material->colorReflective = glm::vec3(1);
    // renderer->AddObject(prozirnaKugla);

    cameraCurve = new BSpline();
    renderer->AddObject(cameraCurve);

    helix = new BSpline();
    helix->addControlPoint(glm::vec3(0, 0, 0));
    helix->addControlPoint(glm::vec3(0, 10, 5));
    helix->addControlPoint(glm::vec3(10, 10, 10));
    helix->addControlPoint(glm::vec3(10, 0, 15));
    helix->addControlPoint(glm::vec3(0, 0, 20));
    helix->addControlPoint(glm::vec3(0, 10, 25));
    helix->addControlPoint(glm::vec3(10, 10, 30));
    helix->addControlPoint(glm::vec3(10, 0, 35));
    helix->addControlPoint(glm::vec3(0, 0, 40));
    helix->addControlPoint(glm::vec3(0, 10, 45));
    helix->addControlPoint(glm::vec3(10, 10, 50));
    helix->addControlPoint(glm::vec3(10, 0, 55));
    helix->finish();
    renderer->AddObject(helix);

    Cubemap skybox = Cubemap::Load({
        "skybox/right.png",
        "skybox/left.png",
        "skybox/top.png",
        "skybox/bottom.png",
        "skybox/front.png",
        "skybox/back.png",
    });
    renderer->SetSkybox(&skybox);

    // Mesh *planeMesh = Mesh::Load("airplane");
    // MeshObject *plane = new MeshObject("avion", planeMesh, phongShader);
    // plane->getTransform()->rotate(plane->getTransform()->forward(), 90);
    // plane->getTransform()->rotate(plane->getTransform()->right(), 90);
    // plane->getTransform()->translate(glm::vec3(0, 3, 3));
    // plane->getTransform()->scale(0.5);
    // renderer->AddObject(plane);

    Mesh *f16Mesh = Mesh::Load("f16");
    MeshObject *f16 = new MeshObject("f16", f16Mesh, phongShader);
    f16->getTransform()->translate(glm::vec3(0, 10, 0));
    f16->getTransform()->scale(3);
    renderer->AddObject(f16);
    movingObject2 = f16->getTransform();

    tangenta = new PolyLine(glm::vec3(1, 0, 0));
    renderer->AddObject(tangenta);

    // Mesh *arapiMesh = Mesh::Load("ArabianCity");
    // MeshObject *arapi = new MeshObject("f16", arapiMesh, phongShader);
    // arapi->getTransform()->translate(glm::vec3(0, 1, 0));
    // arapi->getTransform()->scale(100);
    // arapi->getTransform()->translate(glm::vec3(1, 0, 1));
    // renderer->AddObject(arapi);

    ParticleCluster pc(1000);
    pc.setOnReset([](auto pc, ParticleUpdate _) {
        for (int i = 0; i < pc->n; ++i) {
            float d1 = i % 10;
            float d2 = (i / 10) % 10;
            float d3 = (i / 100) % 10;

            glm::vec3 val = glm::vec3(2.5, 1.6, -1.8) + glm::vec3(d1, d2, d3) * (4.0f / 100);
            pc->setPoint(i, val);
        }
    });
    pc.setOnChange([&](auto pc, ParticleUpdate data) {
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
        // sfera.getTransform()->setScale(1);
        // std::cout << glm::to_string(sfera.getMesh()->getVertex(i)) << std::endl;
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

    ParticleCluster pc3(1000);
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

    ParticleCluster pc4(1000);
    glm::vec3 pc4Middle = glm::vec3(0);
    std::vector<float> pc4Speeds(1000);
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
    movingObject = pc4.getTransform();

    PC5 pc5(glm::vec3(2.5, 1.6, -13), false);

    renderer->AddParticleCluster(&pc5);

    ParticleCluster pc6(1000);
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

    renderer->EnableVSync(controllerNum);

    renderer->Loop();

    return EXIT_SUCCESS;
}

#endif