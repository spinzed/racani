#include "examples/ExampleBSpline.h"

// Local Headers
#include "models/Mesh.h"
#include "objects/MeshObject.h"
#include "renderer/Animation.h"
#include "renderer/Animator.h"
#include "renderer/Camera.h"
#include "renderer/Cubemap.h"
#include "renderer/Input.h"
#include "renderer/Shader.h"
#include "renderer/Transform.h"
#include "renderer/WindowManager.h"
#include "utils/PerlinNoise.h"

// System Headers
#include <GLFW/glfw3.h>
#include <glad/glad.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

#include <cstdio>
#include <cstdlib>
#include <iostream>

class MoveAnimation : public Animation {
  public:
    using Animation::Animation;
    ~MoveAnimation() {};

    Transform *tr;
    PolyLine *tangenta;

    MoveAnimation(BSpline *c, Transform *tr, float duration, PolyLine *t) : Animation(c, duration) {
        this->tr = tr;
        tangenta = t;
    }

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

void ExampleBSpline::KeyCallback(InputGlobalListenerData event) {
    if (event.action != GLFW_PRESS && event.action != GLFW_REPEAT)
        return;
    Camera *camera = renderer->GetCamera();

    if (event.key == GLFW_KEY_H) {
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
        Animation *a = new MoveAnimation(cameraCurve, renderer->GetCamera(), 3.0f, tangenta);
        Animation *b = new MoveAnimation(helix, movingObject2, 8.0f, tangenta);
        if (cameraCurve->degree() >= 3) {
            Animator::registerAnimation(a);
        }
        Animator::registerAnimation(b);
    }
}

void ExampleBSpline::cursorPositionCallback(WindowCursorEvent event) {
    if (!renderer->manager->focused)
        return;

    int dx = (float)width / 2 - event.xpos;
    int dy = (float)height / 2 - event.ypos;
    Camera *camera = renderer->GetCamera();

    camera->rotate(mouseSensitivity * dx, mouseSensitivity * dy);
    camera->recalculateMatrix();

    renderer->manager->CenterCursor();
}

int ExampleBSpline::run(std::string execDirectory) {
    renderer = new Renderer(width, height, execDirectory);

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
    movingObject = camera;

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

    Mesh *f16Mesh = Mesh::Load("f16");
    MeshObject *f16 = new MeshObject("f16", f16Mesh, phongShader);
    f16->getTransform()->rotate(TransformIdentity::up(), 180.0f);
    f16->shader = Shader::Load("fullbright");

    Object avion("avion");
    avion.getTransform()->translate(glm::vec3(0, 10, 0));
    avion.getTransform()->scale(3);
    avion.addChild(f16);

    renderer->AddObject(&avion);
    movingObject2 = avion.getTransform();

    tangenta = new PolyLine(glm::vec3(1, 0, 0));
    renderer->AddObject(tangenta);

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

    renderer->EnableVSync();

    renderer->Loop();

    return EXIT_SUCCESS;
}
