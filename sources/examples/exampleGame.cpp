#include "examples/exampleGame.h"

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
#include "renderer/UI.h"
#include "renderer/WindowManager.h"
#include "utils/PerlinNoise.h"
#include "utils/ThreadPool.h"

// System Headers
#include "glm/common.hpp"
#include "imgui.h"
#include "utils/mtr.h"
#include <GLFW/glfw3.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <mutex>

int width = 500, height = 500;
float moveSensitivity = 3, sprintMultiplier = 5, mouseSensitivity = 0.15f;

Renderer *renderer;

PerlinNoise perlin;

class Field : public Behavior {
  public:
    glm::vec2 _min = glm::vec2(0);
    glm::vec2 _max = glm::vec2(25);
    glm::vec2 scale = glm::vec2(20, 3);
    float perlinD = 0.01;
    float threshold = 0.3f;

    virtual void Init(Object *object) {
        PointCloud *perlinCloud = dynamic_cast<PointCloud *>(object);
        perlinCloud->setPointSize(2.0f);
    }

    virtual void generate(float min_, float max_, int stripEdgeMask = 0) {
        assert(object != nullptr);
        _min.x = min_;
        _max.x = max_;

        PointCloud *perlinCloud = dynamic_cast<PointCloud *>(object);

        perlinCloud->reset();

        for (float y = _min.y / scale.y; y < _max.y / scale.y; y += perlinD) {
            for (float x = _min.x / scale.x; x < _max.x / scale.x; x += perlinD) {
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
                    perlinCloud->addPoint(glm::vec3(scale.x * x, 0, scale.y * y), glm::vec3(1, 1, 0));
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

    // player
    Mesh *arwingMesh = Mesh::Load("arwing");
    Mesh *glava = Mesh::Load("glava");
    MeshObject arwing("playerModel", arwingMesh, fullbrightShader);
    MeshObject arwing2("a2", glava, fullbrightShader);
    arwing2.getTransform()->translate(glm::vec3(-2.5, 1, 1.5));

    glm::vec3 initialPlayerPosition = glm::vec3(-10.0f, -0.3f, 12.5f);
    Object player("player");
    player.getTransform()->translate(glm::vec3(0.0f, -0.3f, 5.0f));
    arwing.getTransform()->scale(0.2f);
    arwing.getTransform()->rotate(TransformIdentity::up(), 90.0f);
    player.getTransform()->translate(initialPlayerPosition);
    player.addChild(&arwing);

    FunctionalBehavior playerBehavior;

    glm::vec3 initialSpeed(8.0f, 0.0f, 0.0f);
    glm::vec3 maxSpeed(999, 8.0f, 8.0f); // goes in both directions
    glm::vec3 speed(initialSpeed);
    glm::vec3 boundsMin(-INFINITY, -size / 2, 0);
    glm::vec3 boundsMax(INFINITY, size / 2, size);
    // 1 point every 4 secs, get to max speed in 250 ms
    glm::vec3 accel(1.0f / 5, maxSpeed.y / 0.250f, maxSpeed.z / 0.250f);
    bool invertZ = false;
    glm::vec3 currentCameraOffset;
    bool cameraOffsetSet = false;
    bool gamePaused = false;

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

    auto setGamePaused = [&](bool paused) {
        gamePaused = paused;
    };

    auto stopGame = [&]() {
        gamePaused = true;
        float score = std::round(player.getTransform()->position().x * 100) / 100;
        std::cout << "Score: " << score << std::endl;
    };

    auto resetGame = [&]() {
        gamePaused = false;
        perlin.randomizeSeed();
        player.getTransform()->setPosition(initialPlayerPosition);
        speed = initialSpeed;
        currentPoljeStart = -50;
        pool.enqueue([&]() { generateNextField(); });
    };

    playerBehavior.onUpdate = [&](Object *player, float deltaTime) {
        if (gamePaused)
            return;

        // switch fields
        if (player->getTransform()->position().x > currentPoljeStart + size && !fieldBusy) {
            pool.enqueue([&]() { generateNextField(); });
        }

        glm::vec3 dir(0);
        glm::vec3 isAutoDeccel(0);

        // keyboard
        if (Input::checkKeyEvent(GLFW_KEY_UP, GLFW_PRESS)) {
            dir.y = invertZ * -1;
        }
        if (Input::checkKeyEvent(GLFW_KEY_DOWN, GLFW_PRESS)) {
            dir.y = invertZ * 1;
        }
        if (Input::checkKeyEvent(GLFW_KEY_LEFT, GLFW_PRESS)) {
            dir.z = -1;
        }
        if (Input::checkKeyEvent(GLFW_KEY_RIGHT, GLFW_PRESS)) {
            dir.z = 1;
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
        if (dir.z == 0 && speed.z != 0) {
            dir.z = -glm::sign(speed.z);
            isAutoDeccel.z = 1;
        }

        // instead of x speed being controlled by keyboard, it starts at a constant and constantly increases
        // speed.x += dir.x * accel * deltaTime;
        speed.x += accel.x * deltaTime;

        // if controller is connected, calculate the speed using the analog stick
        if (Input::ControllerConnected()) {
            float valY = mtr::deadzone(Input::GetAnalog(XboxOneAnalog::LEFT_Y), 0.5f, 0, 1);
            float valZ = mtr::deadzone(Input::GetAnalog(XboxOneAnalog::LEFT_X), 0.1f, 0, 1);
            speed.y = valY * maxSpeed.y;
            speed.z = valZ * maxSpeed.z;
        } else {
            speed.y += dir.y * accel.y * deltaTime;
            speed.z += dir.z * accel.z * deltaTime;
        }

        // 0 clamp
        if (isAutoDeccel.x && speed.x * dir.x > 0) {
            speed.x = 0;
        }
        if (isAutoDeccel.y && speed.y * dir.y > 0) {
            speed.y = 0;
        }
        if (isAutoDeccel.z && speed.z * dir.z > 0) {
            speed.z = 0;
        }

        // extremes clamp
        speed = glm::clamp(speed, -maxSpeed, maxSpeed);

        // apply final temporary (non linear) transformations to speed
        // make the y & z speed move on a sphere rather than a line. This will effectively
        // make the aircraft change its direction uniformally rather than doing it faster
        // when the speed is small rather than large.
        glm::vec3 transformedSpeed(                                                 //
            speed.x,                                                                //
            maxSpeed.y * glm::sin((speed.y / maxSpeed.y) * (glm::pi<float>() / 2)), //
            maxSpeed.z * glm::sin((speed.z / maxSpeed.z) * (glm::pi<float>() / 2))  //
        );

        // add some speed forward if the boost button is pressed
        if (Input::checkKeyEvent(GLFW_KEY_LEFT_SHIFT, GLFW_PRESS) || Input::GetAnalog(XboxOneAnalog::RT) > 0) {
            transformedSpeed.x += initialSpeed.x * 0.5f;
        }

        glm::vec3 oldPlayerPos = player->getTransform()->position();
        glm::vec3 newPlayerPos = oldPlayerPos + transformedSpeed * deltaTime;

        // position clamp
        newPlayerPos = glm::clamp(newPlayerPos, boundsMin, boundsMax);

        // set new pos
        player->getTransform()->setPosition(newPlayerPos);

        // check for collisions with the perlin walls
        glm::vec2 pointPlayer(newPlayerPos.x, newPlayerPos.z);
        glm::vec2 playerSize(0.1);

        bool hit = false;
        for (int i = 0; i < currentPolje->pointNumber(); i++) {
            glm::vec3 point = currentPolje->getPoint(i);
            glm::vec2 point2d(point.x, point.z);
            hit = mtr::isPointInAABB2D(point2d, pointPlayer - playerSize, pointPlayer + playerSize);
            if (hit)
                break;
        }

        // if collision, revert to the old position
        if (hit) {
            player->getTransform()->setPosition(oldPlayerPos);
            stopGame();
        }

        // snap the inner model
        if (transformedSpeed != glm::vec3(0)) {
            Object *model = player->GetChild("playerModel");
            assert(model != nullptr);
            model->getTransform()->pointAtDirection(-transformedSpeed, TransformIdentity::up());
        }

        // snap camera - calculate interpolated speed between current position of the camera the position
        // of where the camera is supposed to be in
        glm::vec3 newCameraOffset(-3 - (transformedSpeed.x - speed.x) / speed.x, 1.5 - speed.y / maxSpeed.y,
                                  -speed.z / maxSpeed.z);
        glm::vec3 interpolatedCameraOffset;
        if (!cameraOffsetSet) {
            interpolatedCameraOffset = newCameraOffset;
            cameraOffsetSet = true;
        } else {
            interpolatedCameraOffset = glm::mix(currentCameraOffset, newCameraOffset, 0.95f * deltaTime * 5);
        }
        Transform *t = renderer->GetCamera();
        t->setPosition(player->getTransform()->position() + interpolatedCameraOffset);
        t->pointAt(player->getTransform()->position(), TransformIdentity::up());

        currentCameraOffset = interpolatedCameraOffset;

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
            Input::ControllerButtonPressed(XboxOneButtons::START)) {
            renderer->SetShouldClose();
        }
        if (Input::ControllerButtonPressed(XboxOneButtons::Y)) {
            resetGame();
        }
        if (Input::ControllerButtonPressed(XboxOneButtons::PAUSE)) {
            setGamePaused(!gamePaused);
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