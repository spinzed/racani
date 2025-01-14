#include "examples/exampleSponza.h"

#if false

// Local Headers
#include "models/Mesh.h"
#include "objects/MeshObject.h"
#include "objects/PointCloud.h"
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

int width = 500, height = 500;
float moveSensitivity = 10, sprintMultiplier = 5, mouseSensitivity = 0.15f;

Renderer *renderer;

void KeyCallback(InputGlobalListenerData event) {
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
    } else if (event.key == GLFW_KEY_ESCAPE) {
        renderer->SetShouldClose();
    }
}

void cursorPositionCallback(WindowCursorEvent event) {
    if (!renderer->manager->focused)
        return;

    int dx = (float)renderer->manager->width / 2 - event.xpos;
    int dy = (float)renderer->manager->height / 2 - event.ypos;
    Camera *camera = renderer->GetCamera();

    camera->rotate(mouseSensitivity * dx, mouseSensitivity * dy);
    camera->recalculateMatrix();

    renderer->manager->CenterCursor();
}

int exampleSponza(std::string execDirectory) {
    renderer = new Renderer(width, height, execDirectory);

    // renderer->input.addMouseListener([](MouseClickOptions _) {});
    renderer->input.addKeyEventListener(KeyCallback);
    renderer->manager->SetCursorHidden(true);

    renderer->manager->setCursorCallback(cursorPositionCallback);
    renderer->manager->setWindowFocusCallback([&](auto data) {
        // FIXME: when clicking on window focused goes to 1, but it doesn't change the cursor pos
        if (data.focused) {
            renderer->manager->CenterCursor();
        }
    });

    /*********************************************************************************************/
    Shader *phongShader = Shader::Load("phong");

    Mesh *kockaMesh3 = Mesh::Load("kocka", glm::vec3(0.8, 0.8, 0.8));
    MeshObject *floor = new MeshObject("pod", kockaMesh3, phongShader);

    floor->getTransform()->translate(glm::vec3(15000.0f, -2.0f, 0.0f));
    floor->getTransform()->scale(glm::vec3(30000.0f, 0.1f, 100.0f));

    // renderer->AddObject(floor);

    Camera *camera = renderer->GetCamera();
    camera->translate(glm::vec3(3.0f, 3.0f, -3.0f));
    camera->rotate(145, -30);
    camera->recalculateMatrix();

    Light *light = new PointLight(glm::vec3(-3, 3, 2), glm::vec3(1, 1, 1), glm::vec3(1, 1, 1));
    PointCloud lightPoint;
    lightPoint.addPoint(light->getTransform()->position(), light->color);
    lightPoint.setPointSize(5);
    lightPoint.commit();
    renderer->AddLight(light);
    renderer->AddObject(&lightPoint);

    Cubemap skybox = Cubemap::Load("skybox");
    renderer->SetSkybox(&skybox);

    Mesh *sponzaMesh = Mesh::Load("sponza");
    MeshObject sponza("sponza", sponzaMesh, phongShader);
    sponza.getTransform()->scale(0.1f);
    renderer->AddObject(&sponza);

    renderer->input.addPerFrameListener([&](auto a) {
        float deltaTime = a.deltaTime;

        float multiplier = moveSensitivity * deltaTime;

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
    });

    renderer->input.addKeyEventListener([&](InputGlobalListenerData event) {
        if (event.action != GLFW_PRESS)
            return;
    });

    renderer->EnableVSync();

    renderer->Loop();

    return EXIT_SUCCESS;
}
#endif