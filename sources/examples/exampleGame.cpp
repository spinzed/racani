#include "examples/exampleGame.h"
#include "renderer/Texture.h"

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

int width = 1000, height = 1000;
float moveSensitivity = 3, sprintMultiplier = 5, mouseSensitivity = 0.15f;

Renderer *renderer;

PerlinNoise perlin;

Shader *genShader;
Texture *genIn;
Texture *genOut;

struct Point {
    glm::vec2 location;
    float value;
    int type; // 1 out, 2 edge, 3 in
    bool checked;
    bool stripped;
};

bool showCulled = false;

class Field : public Behavior {
  public:
    glm::vec2 _min = glm::vec2(0);
    glm::vec2 _max = glm::vec2(25);
    glm::vec2 scale = glm::vec2(20, 3);
    glm::vec2 resolution = glm::vec2(1000, 1000);
    std::vector<std::vector<Point>> points;
    float threshold = 0.3f;
    bool suppressGeneration = false;
    bool useCPU = true;
    int stripEdgeMask = 0;

    virtual void Init(Object *object) {
        PointCloud *perlinCloud = dynamic_cast<PointCloud *>(object);
        perlinCloud->setPointSize(2.0f);
    }

    virtual void reset() {
        assert(object != nullptr);
        PointCloud *perlinCloud = dynamic_cast<PointCloud *>(object);

        perlinCloud->reset();
    }

    virtual void generate(float min_, float max_) {
        Timer t = Timer::start();
        if (useCPU) {
            generateCPU(min_, max_);
        } else {
            generateGPU(min_, max_);
        }
        t.printElapsed("Generating done in: $");
    }

    virtual void generateGPU(float min_, float max_0) { // 0bXXXX - x-, x+, y-, y+
        if (!genOut) {
            genShader = Shader::LoadCompute("generator");
            genOut = new Texture(GL_TEXTURE_2D, resolution.x, resolution.y, 0, 4);
            genOut->setStorage(0x01);
        }

        genShader->use();
        genShader->compute(resolution.x, resolution.y);

        std::vector<float> out;
        genOut->getData(out);

        std::cout << out[0] << std::endl;
    }

    virtual void generateCPU(float min_, float max_) {
        assert(object != nullptr);
        if (suppressGeneration) {
            return;
        }
        _min.x = min_;
        _max.x = max_;

        PointCloud *perlinCloud = dynamic_cast<PointCloud *>(object);

        perlinCloud->reset();
        points.clear();
        points.resize(resolution.x);

        glm::vec2 diff = _max / scale - _min / scale;

        // generate values
        for (int x = 0; x < resolution.x; x++) {
            points[x].resize(resolution.y);
            float xCoord = _min.x / scale.x + diff.x / resolution.x * x;

            for (int y = 0; y < resolution.y; y++) {
                float yCoord = _min.y / scale.y + diff.y / resolution.y * y;
                points[x][y] = {{xCoord, yCoord}, perlin.noise(xCoord, yCoord), 1, 0, 0};
            }
        }

        // generate the edges
        for (int x = 0; x < resolution.x; x++) {
            for (int y = 0; y < resolution.y; y++) {
                Point &p = points[x][y];
                p.type = (p.value > threshold && findOutOfRangeNeighbors(x, y)) ? 2 : 1;
            }
        }

        // clamp the unfinished edges
        for (int x = 0; x < resolution.x; x++) {
            for (int y = 0; y < resolution.y; y++) {
                Point &p = points[x][y];
                p.checked = true;

                // skip if not applicable
                if (p.value < threshold || p.type != 2)
                    continue;

                if (((stripEdgeMask & 0b1000) && (x == 0)) || ((stripEdgeMask & 0b0100) && (x == resolution.x - 1)) ||
                    ((stripEdgeMask & 0b0010) && (y == 0)) || ((stripEdgeMask & 0b0001) && (y == resolution.y - 1))) {
                    markStripped(x, y);
                }
            }
        }

        // generate the renderable data
        for (int x = 0; x < resolution.x; x++) {
            for (int y = 0; y < resolution.y; y++) {
                Point &p = points[x][y];
                if (p.value > threshold && p.type == 2 && !p.stripped) {
                    glm::vec2 &loc = points[x][y].location;
                    perlinCloud->addPoint(glm::vec3(scale.x * loc.x, 0, scale.y * loc.y), glm::vec3(1, 1, 0));
                }
                if (showCulled && p.stripped) {
                    glm::vec2 &loc = points[x][y].location;
                    perlinCloud->addPoint(glm::vec3(scale.x * loc.x, 0, scale.y * loc.y), glm::vec3(1, 0, 0));
                }
            }
        }
        perlinCloud->commit();
    }

    void markStripped(int x, int y, bool first = true) {
        // skip borders and stripped
        if (points[x][y].type != 2 || points[x][y].stripped)
            return;

        points[x][y].stripped = true;

        int has = 0;

        for (const auto &vec : findNeighbors(x, y)) {
            // std::cout << x << " " << y << " stripped" << std::endl;
            markStripped(vec.x, vec.y, false);
            has = 1;
        }

        if (first && !has) {
            std::cout << "this first has" << std::endl;
        }
    }

    // find neighbors that are above the treshold
    std::vector<glm::vec2> findNeighbors(int x, int y) {
        std::vector<glm::vec2> n;

        for (int dx = x - 1; dx <= x + 1; dx++) {
            for (int dy = y - 1; dy <= y + 1; dy++) {
                if (dx == x && dy == x)
                    continue;
                if (dx < 0 || dy < 0 || dx >= resolution.x || dy >= resolution.y) {
                    continue;
                }

                if (points[dx][dy].value > threshold) {
                    n.emplace_back(glm::vec2(dx, dy));
                }
            }
        }
        return n;
    }

    bool findOutOfRangeNeighbors(int x, int y) {
        for (int dx = x - 1; dx <= x + 1; dx++) {
            for (int dy = y - 1; dy <= y + 1; dy++) {
                if (dx == x && dy == x)
                    continue;
                if (dx < 0 || dy < 0 || dx >= resolution.x || dy >= resolution.y) {
                    continue;
                }

                if (points[dx][dy].value <= threshold) {
                    return true;
                }
            }
        }
        return false;
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

    int segments = 5;

    std::vector<PointCloud *> polja(segments);
    std::vector<Field> ponasanja(segments);

    for (int i = 0; i < segments; i++) {
        polja[i] = new PointCloud();
        polja[i]->addBehavior(&ponasanja[i]);
        // ponasanja[i].generate(i * size, (i + 1) * size);
        renderer->AddObject(polja[i]);
    }

    // player
    Mesh *arwingMesh = Mesh::Load("arwing");
    Mesh *glava = Mesh::Load("glava");
    MeshObject arwing("playerModel", arwingMesh, fullbrightShader);
    MeshObject arwing2("a2", glava, fullbrightShader);
    arwing2.getTransform()->translate(glm::vec3(-2.5, 1, 1.5));

    glm::vec3 firstPlayerPosition = glm::vec3(-30.0f, -0.3f, 12.5f);
    Object player("player");
    player.getTransform()->translate(glm::vec3(0.0f, -0.3f, 5.0f));
    arwing.getTransform()->scale(0.2f);
    arwing.getTransform()->rotate(TransformIdentity::up(), 90.0f);
    player.getTransform()->translate(firstPlayerPosition);
    player.addChild(&arwing);

    FunctionalBehavior playerBehavior;

    glm::vec3 initialSpeed(10.0f, 0.0f, 0.0f);
    glm::vec3 maxSpeed(999, 8.0f, 10.0f); // goes in both directions
    glm::vec3 speed(initialSpeed);
    glm::vec3 boundsMin(-INFINITY, -size / 2, 0);
    glm::vec3 boundsMax(INFINITY, size / 2, size);
    // 1 point every 4 secs, get to max speed in 250 ms
    glm::vec3 accel(1.0f / 5, maxSpeed.y / 0.250f, maxSpeed.z / 0.250f);
    glm::vec3 initialPlayerPosition = glm::vec3(-10.0f, -0.3f, 12.5f);
    glm::vec3 initialCameraOffset(20, 1.5, 1);
    glm::vec3 currentCameraOffset;
    bool invertZ = false;
    bool cameraOffsetSet = false;
    bool gamePaused = false;
    bool collisionsActive = true;
    bool freeCamera = false;
    bool cameraStatic = true;

    std::mutex generateMutex;
    bool fieldBusy = false;

    auto setFieldSettings = [&](Field *f, int start) {
        int segment = start / size;
        if (segment < 8) {
            f->suppressGeneration = false;
            f->threshold = 0.5;
            f->stripEdgeMask = 0b0000 | (segment == 0 ? 0b1000 : 0) | (segment == 7 ? 0b0100 : 0);
        } else if (segment == 8) {
            f->suppressGeneration = true;
        } else if (segment < 16) {
            f->suppressGeneration = false;
            f->threshold = 0.3;
            f->stripEdgeMask = 0b0000 | (segment == 9 ? 0b1000 : 0) | (segment == 15 ? 0b0100 : 0);
        } else if (segment == 16) {
            f->suppressGeneration = true;
        } else {
            f->suppressGeneration = false;
            f->threshold = 0.22;
            f->stripEdgeMask = 0b0000 | (segment == 17 ? 0b1000 : 0);
        }
    };

    auto generateField = [&](int i, int start, bool swap) {
        generateMutex.lock();
        fieldBusy = true;

        Field *b = (Field *)polja[i]->behaviors[0];
        if (swap) {
            std::rotate(polja.begin(), polja.begin() + 1, polja.end());
        }

        setFieldSettings(b, start);
        b->generate(start, start + size);

        if (swap) {
            currentPoljeStart += size;
        }

        fieldBusy = false;
        generateMutex.unlock();
    };

    auto setGamePaused = [&](bool paused) { gamePaused = paused; };

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
        currentPoljeStart = 0;

        for (int i = 0; i < segments; i++) {
            int j = i;
            ponasanja[i].reset();
            pool.enqueue([&, j]() { generateField(j, j * size, false); });
        }
    };

    playerBehavior.onInit = [&](Object *_) {
        (void)_;
        resetGame();
        currentCameraOffset = initialCameraOffset;
        player.getTransform()->setPosition(firstPlayerPosition);
        camera->setPosition(player.getTransform()->position() + initialCameraOffset);
    };

    playerBehavior.onUpdate = [&](Object *player, float deltaTime) {
        glm::vec3 pos = player->getTransform()->position();

        UI::Build([speed, pos, &collisionsActive, &freeCamera]() {
            bool c = Input::ControllerConnected();
            ImGui::Text("Controller connected: %s", c ? Input::GetControllerName() : "No");
            ImGui::Text("Pos: %s", glm::to_string(pos).c_str());
            ImGui::Text("Saved Speed: %s", glm::to_string(speed).c_str());
            if (ImGui::Button(std::format("Toggle Draw Culled ({})", showCulled ? "on" : "off").c_str())) {
                showCulled = !showCulled;
            }
            if (ImGui::Button(std::format("Toggle Collisions ({})", collisionsActive ? "on" : "off").c_str())) {
                collisionsActive = !collisionsActive;
            }
            if (ImGui::Button(std::format("Free Camera ({})", freeCamera ? "on" : "off").c_str())) {
                freeCamera = !freeCamera;
            }
        });

        if (gamePaused)
            return;

        // switch fields
        if (player->getTransform()->position().x > currentPoljeStart + 2 * size && !fieldBusy) {
            pool.enqueue([&]() { generateField(0, currentPoljeStart + segments * size, 1); });
        }
        // generateField(0, currentPoljeStart, 1);

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
        if (collisionsActive) {
            for (int i = 0; i < polja[0]->pointNumber(); i++) {
                glm::vec3 point = polja[0]->getPoint(i);
                glm::vec2 point2d(point.x, point.z);
                hit = mtr::isPointInAABB2D(point2d, pointPlayer - playerSize, pointPlayer + playerSize);
                if (hit) {
                    std::cout << "hit" << std::endl;
                    break;
                }
            }
        }

        // if collision, revert to the old position
        if (hit) {
            player->getTransform()->setPosition(oldPlayerPos);
            stopGame();
            return;
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
        if (!cameraStatic || currentCameraOffset.x < newCameraOffset.x) {
            cameraStatic = false;
            t->setPosition(player->getTransform()->position() + interpolatedCameraOffset);
        }
        currentCameraOffset = t->position() - player->getTransform()->position();
        t->pointAt(player->getTransform()->position(), TransformIdentity::up());
    };
    player.addBehavior(&playerBehavior);
    renderer->AddObject(&player);
    renderer->AddObject(&arwing2);

    renderer->input.addPerFrameListener([&](auto a) {
        if (!freeCamera)
            return;

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

    renderer->input.addPerFrameListener([&](auto a) {
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
        if (event.key == GLFW_KEY_P) {
            setGamePaused(!gamePaused);
        }
        if (event.key == GLFW_KEY_1) {
            showCulled = true;
        }
    });

    renderer->EnableVSync();

    renderer->Loop();

    return EXIT_SUCCESS;
}
#endif