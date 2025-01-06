#include "examples/exampleRT.h"

#if false

// Local Headers
#include "models/Mesh.h"
#include "objects/Plane.h"
#include "objects/Sphere.h"
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
float moveSensitivity = 3, sprintMultiplier = 5, mouseSensitivity = 0.15f;

Renderer *renderer;

void KeyCallback(InputGlobalListenerData event) {
    if (event.action != GLFW_PRESS && event.action != GLFW_REPEAT)
        return;
    Camera *camera = renderer->GetCamera();

    if (event.key == GLFW_KEY_1 && event.action == GLFW_PRESS) {
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
    } else if (event.key == GLFW_KEY_UP) {
        renderer->setDepth(renderer->getDepth() + 1);
        std::cout << "Set depth to " << renderer->getDepth() << std::endl;
    } else if (event.key == GLFW_KEY_DOWN) {
        if (renderer->getDepth() > 1) {
            renderer->setDepth(renderer->getDepth() - 1);
            std::cout << "Set depth to " << renderer->getDepth() << std::endl;
        }
    } else if (event.key == GLFW_KEY_LEFT) {
        renderer->setKSpecular(std::max(renderer->kSpecular() - 0.05f, 0.0f));
        std::cout << "Set reflection factor to " << renderer->kSpecular() << std::endl;
    } else if (event.key == GLFW_KEY_RIGHT) {
        renderer->setKSpecular(std::min(renderer->kSpecular() + 0.05f, 1.0f));
        std::cout << "Set reflection factor to " << renderer->kSpecular() << std::endl;
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

int exampleRT(std::string execDirectory) {
    renderer = new Renderer(width, height, execDirectory);

    renderer->input.addMouseListener([](MouseClickOptions _) {});
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
    Camera *camera = renderer->GetCamera();
    camera->translate(glm::vec3(3.0f, 3.0f, -3.0f));
    camera->rotate(145, -30);
    camera->recalculateMatrix();

    Shader *phongShader = Shader::Load("phong");
    Shader *fullbrightShader = Shader::Load("fullbright");

    Plane floor("pod", 10, 10, glm::vec3(0.5, 0.5, 1));

    floor.getTransform()->rotate(TransformIdentity::right(), 90);
    floor.applyTransform();
    floor.mesh->commit();

    renderer->AddObject(&floor);

    Light *light = new PointLight(glm::vec3(3, 1, 3), glm::vec3(1, 1, 1), glm::vec3(1, 1, 1), 10);
    renderer->AddLight(light);

    Sphere *zutaKugla = new Sphere("zutaKugla", glm::vec3(0, 2, 0), 2, glm::vec3(1, 1, 0));
    zutaKugla->mesh->material->colorReflective = glm::vec3(0.5);
    renderer->AddObject(zutaKugla);

    // Sphere *prozirnaKugla = new Sphere("zutaKugla", glm::vec3(5, 2, 4), 2, glm::vec3(0.3, 1, 0));
    // prozirnaKugla->mesh->material->colorReflective = glm::vec3(1);
    // renderer->AddObject(prozirnaKugla);

    Cubemap skybox = Cubemap::Load({
        "skybox/right.png",
        "skybox/left.png",
        "skybox/top.png",
        "skybox/bottom.png",
        "skybox/front.png",
        "skybox/back.png",
    });
    renderer->SetSkybox(&skybox);

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