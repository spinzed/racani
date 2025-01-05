#include "examples/exampleGame.h"

#if true

// Local Headers
#include "models/Mesh.h"
#include "objects/BSpline.h"
#include "objects/MeshObject.h"
#include "objects/PointCloud.h"
#include "renderer/Animation.h"
#include "renderer/Animator.h"
#include "renderer/Behavior.h"
#include "renderer/Camera.h"
#include "renderer/Cubemap.h"
#include "renderer/InputSystem.h"
#include "renderer/ParticleSystem.h"
#include "renderer/Renderer.h"
#include "renderer/Shader.h"
#include "renderer/Transform.h"
#include "renderer/WindowManager.h"
#include "utils/PerlinNoise.h"

// System Headers
#include "glm/common.hpp"
#include "utils/mtr.h"
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

bool axis = false;

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

int exampleGame(std::string execDirectory) {
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
    Shader *phongShader = Shader::Load("phong");
    Shader *fullbrightShader = Shader::Load("fullbright");

    Mesh *kockaMesh3 = Mesh::Load("kocka", glm::vec3(0.8, 0.8, 0.8));
    MeshObject *floor = new MeshObject("pod", kockaMesh3, phongShader);

    floor->getTransform()->translate(glm::vec3(0.0f, -2.0f, 0.0f));
    floor->getTransform()->scale(glm::vec3(300.0f, 0.1f, 300.0f));

    renderer->AddObject(floor);

    Camera *camera = renderer->GetCamera();
    camera->translate(glm::vec3(3.0f, 3.0f, -3.0f));
    camera->rotate(145, -30);
    camera->recalculateMatrix();

    Light *light = new PointLight(glm::vec3(-3, 3, 2), glm::vec3(1, 1, 1), glm::vec3(1, 1, 1));
    renderer->AddLight(light);

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

        if (InputSystem::checkKeyEvent(GLFW_KEY_LEFT_CONTROL, GLFW_PRESS)) {
            multiplier *= sprintMultiplier;
        }
        if (InputSystem::checkKeyEvent(GLFW_KEY_W, GLFW_PRESS)) {
            // camera->translate(multiplier * Transform::Identity().forward());
            camera->setPosition(camera->position() + multiplier * camera->forward());
            camera->recalculateMatrix();
            // camera->translate(multiplier * camera->forward());
        }
        if (InputSystem::checkKeyEvent(GLFW_KEY_A, GLFW_PRESS)) {
            camera->translate(multiplier * -TransformIdentity::right());
            camera->recalculateMatrix();
        }
        if (InputSystem::checkKeyEvent(GLFW_KEY_S, GLFW_PRESS)) {
            camera->translate(multiplier * -TransformIdentity::forward());
            camera->recalculateMatrix();
        }
        if (InputSystem::checkKeyEvent(GLFW_KEY_D, GLFW_PRESS)) {
            camera->translate(multiplier * TransformIdentity::right());
            camera->recalculateMatrix();
        }
        if (InputSystem::checkKeyEvent(GLFW_KEY_SPACE, GLFW_PRESS)) {
            camera->translate(multiplier * camera->vertical());
            camera->recalculateMatrix();
        }
        if (InputSystem::checkKeyEvent(GLFW_KEY_LEFT_SHIFT, GLFW_PRESS)) {
            camera->translate(-multiplier * camera->vertical());
            camera->recalculateMatrix();
        }
        if (InputSystem::checkKeyEvent(GLFW_KEY_ESCAPE, GLFW_PRESS)) {
            renderer->SetShouldClose();
        }
    });

    PerlinNoise perlin;
    PointCloud perlinCloud;
    perlinCloud.setPointSize(2.0f);
    perlinCloud.getTransform()->translate(glm::vec3(10, 0, 0));
    float perlinMin = 0;
    float perlinMax = 5;
    float perlinD = 0.01;
    float perlinScale = 5;

    for (float y = perlinMin; y < perlinMax; y += perlinD) {
        for (float x = perlinMin; x < perlinMax; x += perlinD) {
            float value = perlin.noise(x, y);
            // perlinCloud.addPoint(glm::vec3(perlinScale * x, 0, perlinScale * y),
            //                      value > 0.2 ? glm::vec3(0, 1, 0) : glm::vec3(1, 0, 0));

            if (value <= 0.2)
                continue;

            bool found = false;
            for (float dy = -perlinD; dy <= perlinD; dy += perlinD) {
                for (float dx = -perlinD; dx <= perlinD; dx += perlinD) {
                    if (dx == 0 && dy == 0)
                        continue;
                    float neighborValue = perlin.noise(x + dx, y + dy);
                    if (neighborValue <= 0.2) {
                        found = true;
                        break;
                    }
                }
                if (found)
                    break;
            }

            if (found) {
                perlinCloud.addPoint(glm::vec3(perlinScale * x, 0, perlinScale * y), glm::vec3(1, 1, 0));
            }
        }
    }
    perlinCloud.applyTransform();
    perlinCloud.commit();
    renderer->AddObject(&perlinCloud);

    // Mesh clifMesh;
    // MeshObject clifs("clifs", &clifMesh, phongShader);

    // player
    Mesh *arwing = Mesh::Load("arwing");
    MeshObject player("player", arwing, fullbrightShader);
    // player.setPointSize(5.0f);
    // player.addPoint(glm::vec3(0, 0, 0), glm::vec3(1, 0, 0));
    // player.commit();

    FunctionalBehavior playerBehavior;

    player.getTransform()->translate(perlinCloud.getTransform()->position());
    float maxSpeed = 8.0f;
    glm::vec2 speed(0);
    float accel = maxSpeed / 0.250f; // get to max speed in 250 ms
    playerBehavior.onUpdate = [&](Object *o, float deltaTime) {
        PointCloud *player = (PointCloud *)o;

        glm::vec2 dir(0);
        glm::vec2 isAutoDeccel(0);
        if (InputSystem::checkKeyEvent(GLFW_KEY_UP, GLFW_PRESS)) {
            dir.x = 1;
        }
        if (InputSystem::checkKeyEvent(GLFW_KEY_DOWN, GLFW_PRESS)) {
            dir.x = -1;
        }
        if (InputSystem::checkKeyEvent(GLFW_KEY_LEFT, GLFW_PRESS)) {
            dir.y = -1;
        }
        if (InputSystem::checkKeyEvent(GLFW_KEY_RIGHT, GLFW_PRESS)) {
            dir.y = 1;
        }

        // decelerate if button not pressed
        if (dir.x == 0 && speed.x != 0) {
            dir.x = -glm::sign(speed.x);
            isAutoDeccel.x = 1;
        }
        if (dir.y == 0 && speed.y != 0) {
            dir.y = -glm::sign(speed.y);
            isAutoDeccel.y = 1;
        }

        speed += dir * accel * deltaTime;

        // 0 clamp
        if (isAutoDeccel.x && speed.x * dir.x > 0) {
            speed.x = 0;
        }
        if (isAutoDeccel.y && speed.y * dir.y > 0) {
            speed.y = 0;
        }
        // extremes clamp
        speed = glm::clamp(speed, glm::vec2(-maxSpeed), glm::vec2(maxSpeed));

        // set new pos
        glm::vec3 oldPlayerPos = player->getTransform()->position();
        player->getTransform()->translate(glm::vec3(speed.x * deltaTime, 0, speed.y * deltaTime));

        // check for collisions with the perlin walls
        glm::vec3 newPlayerPos = player->getTransform()->position();
        glm::vec2 pointPlayer(newPlayerPos.x, newPlayerPos.z);
        glm::vec2 playerSize(0.1);

        bool found = false;
        for (int i = 0; i < perlinCloud.pointNumber(); i++) {
            glm::vec3 point = perlinCloud.getPoint(i);
            glm::vec2 point2d(point.x, point.z);
            found = mtr::isPointInAABB2D(point2d, pointPlayer - playerSize, pointPlayer + playerSize);
            if (found)
                break;
        }

        // if collision, revert to the old position
        if (found) {
            player->getTransform()->setPosition(oldPlayerPos);
        }
        player->setPointColor(0, found ? glm::vec3(0, 1, 0) : glm::vec3(1, 0, 0));
        player->commit();
    };
    player.addBehavior(&playerBehavior);
    renderer->AddObject(&player);

    renderer->EnableVSync();

    renderer->Loop();

    return EXIT_SUCCESS;
}
#endif