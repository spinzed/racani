#include "examples/exampleGame.h"
#include "imgui.h"
#include "renderer/UI.h"
#include "utils/GLDebug.h"
#include "utils/ThreadPool.h"
#include <GL/gl.h>

#if true

// Local Headers
#include "models/Mesh.h"
#include "objects/MeshObject.h"
#include "objects/PointCloud.h"
#include "renderer/Behavior.h"
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
#include <mutex>

int width = 500, height = 500;
float moveSensitivity = 3, sprintMultiplier = 5, mouseSensitivity = 0.15f;

Renderer *renderer;

class Field : public Behavior {
  public:
    float minX = 0;
    float maxX = 25;
    float minY = 0;
    float maxY = 25;
    float perlinD = 0.01;
    float scale = 3;
    float threshold = 0.5f;

    PerlinNoise perlin;

    virtual void Init(Object *object) {
        PointCloud *perlinCloud = dynamic_cast<PointCloud *>(object);
        perlinCloud->setPointSize(2.0f);
    }

    virtual void generate(float min_, float max_) {
        assert(object != nullptr);
        minX = min_;
        maxX = max_;

        PointCloud *perlinCloud = dynamic_cast<PointCloud *>(object);

        perlinCloud->reset();

        for (float y = minY / scale; y < maxY / scale; y += perlinD) {
            for (float x = minX / scale; x < maxX / scale; x += perlinD) {
                float value = perlin.noise(x, y);

                if (value <= threshold)
                    continue;

                bool found = false;
                for (float dy = -perlinD; dy <= perlinD; dy += perlinD) {
                    for (float dx = -perlinD; dx <= perlinD; dx += perlinD) {
                        if (dx == 0 && dy == 0)
                            continue;
                        float neighborValue = perlin.noise(x + dx, y + dy);
                        if (neighborValue <= threshold) {
                            found = true;
                            break;
                        }
                    }
                    if (found)
                        break;
                }

                if (found) {
                    perlinCloud->addPoint(glm::vec3(scale * x, 0, scale * y), glm::vec3(1, 1, 0));
                }
            }
        }
        perlinCloud->commit();
    }
};

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

    int dx = (float)renderer->manager->width / 2 - event.xpos;
    int dy = (float)renderer->manager->height / 2 - event.ypos;
    Camera *camera = renderer->GetCamera();

    camera->rotate(mouseSensitivity * dx, mouseSensitivity * dy);
    camera->recalculateMatrix();

    renderer->manager->CenterCursor();
}

int exampleGame(std::string execDirectory) {
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
    Shader *fullbrightShader = Shader::Load("fullbright");

    ThreadPool pool(1);

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

    // Cubemap skybox = Cubemap::Load("skybox");
    Cubemap skybox = Cubemap::Load({
        "kurt/space_rt.png",
        "kurt/space_lf.png",
        "kurt/space_up.png",
        "kurt/space_dn.png",
        "kurt/space_ft.png",
        "kurt/space_bk.png",
    });
    renderer->SetSkybox(&skybox);

    float size = 25;
    float currentPoljeStart = 0;

    PointCloud polje1;
    PointCloud polje2;
    Field poljeBehavior1;
    Field poljeBehavior2;
    polje1.addBehavior(&poljeBehavior1);
    polje2.addBehavior(&poljeBehavior2);
    poljeBehavior1.generate(0, size);
    poljeBehavior2.generate(size, 2 * size);
    renderer->AddObject(&polje1);
    renderer->AddObject(&polje2);
    PointCloud *currentPolje = &polje1;
    PointCloud *nextPolje = &polje2;

    // Mesh clifMesh;
    // MeshObject clifs("clifs", &clifMesh, phongShader);

    // player
    Mesh *arwingMesh = Mesh::Load("arwing");
    Mesh *glava = Mesh::Load("glava");
    MeshObject arwing("playerModel", arwingMesh, fullbrightShader);
    MeshObject arwing2("a2", glava, fullbrightShader);
    arwing2.getTransform()->translate(glm::vec3(-2.5, 1, 1.5));

    glm::vec3 initialPlayerPosition = glm::vec3(-5.0f, -0.3f, 12.5f);
    Object player("player");
    player.getTransform()->translate(glm::vec3(0.0f, -0.3f, 5.0f));
    arwing.getTransform()->scale(0.2f);
    arwing.getTransform()->rotate(TransformIdentity::up(), 90.0f);
    player.getTransform()->translate(initialPlayerPosition);
    player.addChild(&arwing);

    FunctionalBehavior playerBehavior;

    glm::vec2 initialSpeed(8.0f, 0.0f);
    glm::vec2 maxSpeed(999, 8.0f);
    glm::vec2 speed(initialSpeed);
    // 1 point every 4 secs, get to max speed in 250 ms
    glm::vec2 accel(1.0f / 4, maxSpeed.y / 0.250f);

    std::mutex generateMutex;
    bool fieldBusy = false;

    auto generateNextField = [&]() {
        generateMutex.lock();
        fieldBusy = true;

        PointCloud *o = currentPolje;
        currentPolje = nextPolje;
        nextPolje = o;

        Field *b = (Field *)nextPolje->behaviors[0];
        b->generate(currentPoljeStart + 2 * size, currentPoljeStart + 3 * size);

        currentPoljeStart += size;

        fieldBusy = false;
        generateMutex.unlock();
    };

    playerBehavior.onUpdate = [&](Object *player, float deltaTime) {
        // switch fields
        if (player->getTransform()->position().x > currentPoljeStart + size && !fieldBusy) {
            pool.enqueue([&]() { generateNextField(); });
        }

        glm::vec2 dir(0);
        glm::vec2 isAutoDeccel(0);

        // keyboard
        if (Input::checkKeyEvent(GLFW_KEY_UP, GLFW_PRESS)) {
            dir.x = 1;
        }
        if (Input::checkKeyEvent(GLFW_KEY_DOWN, GLFW_PRESS)) {
            dir.x = -1;
        }
        if (Input::checkKeyEvent(GLFW_KEY_LEFT, GLFW_PRESS)) {
            dir.y = -1;
        }
        if (Input::checkKeyEvent(GLFW_KEY_RIGHT, GLFW_PRESS)) {
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

        // instead of x speed being controlled by keyboard, it starts at a constant and constantly increases
        // speed.x += dir.x * accel * deltaTime;
        speed.x += accel.x * deltaTime;

        // if controller is connected, calculate the speed using the analog stick
        if (Input::ControllerConnected()) {
            float deadzone = 0.1f;
            float val = Input::GetAnalog(XboxOneAnalog::LEFT_X);
            if (std::abs(val) < deadzone) {
                val = 0;
            } else {
                val = glm::sign(val) * (mtr::map(glm::abs(val), deadzone, 1, 0, 1));
            }
            speed.y = val * maxSpeed.y;
        } else {
            speed.y += dir.y * accel.y * deltaTime;
        }

        // 0 clamp
        if (isAutoDeccel.x && speed.x * dir.x > 0) {
            speed.x = 0;
        }
        if (isAutoDeccel.y && speed.y * dir.y > 0) {
            speed.y = 0;
        }
        // extremes clamp
        speed = glm::clamp(speed, -maxSpeed, maxSpeed);

        // apply final temporary (non linear) transformations to speed
        // make the y speed move on a sphere rather than a line. This will effectively
        // make the aircraft change its direction uniformally rather than doing it faster
        // when the speed is small rather than large.
        glm::vec2 transformedSpeed(speed.x, maxSpeed.y * glm::sin((speed.y / maxSpeed.y) * (glm::pi<float>() / 2)));

        // add some speed if the boost button is pressed
        if (Input::checkKeyEvent(GLFW_KEY_LEFT_SHIFT, GLFW_PRESS) || Input::GetAnalog(XboxOneAnalog::RT) > 0) {
            transformedSpeed.x += initialSpeed.x * 0.5f;
        }

        // set new pos
        glm::vec3 oldPlayerPos = player->getTransform()->position();
        player->getTransform()->translate(glm::vec3(transformedSpeed.x * deltaTime, 0, transformedSpeed.y * deltaTime));

        // check for collisions with the perlin walls
        glm::vec3 newPlayerPos = player->getTransform()->position();
        glm::vec2 pointPlayer(newPlayerPos.x, newPlayerPos.z);
        glm::vec2 playerSize(0.1);

        bool found = false;
        for (int i = 0; i < currentPolje->pointNumber(); i++) {
            glm::vec3 point = currentPolje->getPoint(i);
            glm::vec2 point2d(point.x, point.z);
            found = mtr::isPointInAABB2D(point2d, pointPlayer - playerSize, pointPlayer + playerSize);
            if (found)
                break;
        }

        // if collision, revert to the old position
        if (found) {
            player->getTransform()->setPosition(oldPlayerPos);
        }

        // snap the inner model
        if (transformedSpeed != glm::vec2(0)) {
            Object *model = player->GetChild("playerModel");
            assert(model != nullptr);
            model->getTransform()->pointAtDirection(glm::vec3(-transformedSpeed.x, 0, -transformedSpeed.y),
                                                    TransformIdentity::up());
        }

        // snap camera
        Transform *t = renderer->GetCamera();
        t->setPosition(player->getTransform()->position() + glm::vec3(-3, 1.5, 0));
        t->pointAt(player->getTransform()->position(), TransformIdentity::up());

        UI::Build([speed, transformedSpeed]() {
            bool c = Input::ControllerConnected();
            ImGui::Text("Controller connected: %s", c ? Input::GetControllerName() : "No");
            ImGui::Text("Saved Speed: %s", glm::to_string(speed).c_str());
            ImGui::Text("Applied Speed: %s", glm::to_string(transformedSpeed).c_str());
        });
    };
    player.addBehavior(&playerBehavior);
    renderer->AddObject(&player);
    renderer->AddObject(&arwing2);

    auto resetGame = [&]() {
        player.getTransform()->setPosition(initialPlayerPosition);
        speed = initialSpeed;
        currentPoljeStart = -50;
        pool.enqueue([&]() { generateNextField(); });
    };

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
        if (Input::checkKeyEvent(GLFW_KEY_ESCAPE, GLFW_PRESS) ||
            Input::ControllerButtonPressed(XboxOneButtons::PAUSE)) {
            renderer->SetShouldClose();
        }
        if (Input::ControllerButtonPressed(XboxOneButtons::PAUSE)) {
            resetGame();
        }
    });

    renderer->input.addKeyEventListener([&](InputGlobalListenerData event) {
        if (event.action != GLFW_PRESS)
            return;
        if (event.key == GLFW_KEY_R) {
            resetGame();
        }
    });

    renderer->EnableVSync();

    renderer->Loop();

    return EXIT_SUCCESS;
}
#endif