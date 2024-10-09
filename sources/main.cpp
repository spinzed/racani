// Local Headers
#include "models/Mesh.h"
#include "models/Raster.h"
#include "objects/MeshObject.h"
#include "objects/Object.h"
#include "objects/Skybox.h"
#include "objects/SphereObject.h"
#include "renderer/Animation.h"
#include "renderer/Animator.h"
#include "renderer/Camera.h"
#include "renderer/Cubemap.h"
#include "renderer/Importer.h"
#include "renderer/Renderer.h"
#include "renderer/Shader.h"
#include "renderer/WindowManager.h"
#include "utils/GLDebug.h"
#include "utils/mtr.h"
#include <objects/BSpline.h>
#include <objects/Bezier.h>

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

// Standard Headers
#include <cstdio>
#include <cstdlib>

#include <iostream>

int width = 500, height = 500;
float moveSensitivity = 2, sprintMultiplier = 4, mouseSensitivity = 0.15f;

Renderer *renderer;

Object *movingObject;

class MoveCameraAnimation : public Animation {
  public:
    using Animation::Animation;
    ~MoveCameraAnimation() {};

    MoveCameraAnimation(BSpline *c, float duration, Camera *camera) : Animation(c, duration) { cam = camera; }

    Camera *cam;
    void setCamera(Camera *camera) { cam = camera; };
    void onChange(float current, float total) override {
        BSpline *c = (BSpline *)curve;
        glm::vec3 point = curve->evaluatePoint(current / total);
        glm::vec3 forward = c->evaluateTangent(current / total);

        movingObject->getTransform()->setPosition(point);
        movingObject->getTransform()->pointAtDirection(forward, TransformIdentity::up());
    }
};

// funkcija koja se poziva prilikom mijenjanja velicine prozora, moramo ju povezati pomocu
// glfwSetFramebufferSizeCallback
void framebuffer_size_callback(GLFWwindow *window, int Width, int Height) {
    width = Width;
    height = Height;
    glViewport(0, 0, width, height);
    renderer->setResolution(width, height);
}

bool axis = false;

BSpline *cameraCurve;

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (action != GLFW_PRESS && action != GLFW_REPEAT)
        return;
    Camera *camera = renderer->getCamera();

    if (key == GLFW_KEY_G) {
        // std::cout << "axis changed" << std::endl;
        axis = !axis;
    } else if (key == GLFW_KEY_1 && action == GLFW_PRESS) {
        renderer->setRenderingMethod(RenderingMethod::Rasterize);
        std::cout << "Set to rasterize" << std::endl;
    } else if (key == GLFW_KEY_2 && action == GLFW_PRESS) {
        renderer->setRenderingMethod(RenderingMethod::Raytrace);
        std::cout << "Set to raytrace" << std::endl;
    } else if (key == GLFW_KEY_3 && action == GLFW_PRESS) {
        renderer->setRenderingMethod(RenderingMethod::Pathtrace);
        std::cout << "Set to pathtrace" << std::endl;
    } else if (key == GLFW_KEY_E && action == GLFW_PRESS) {
        renderer->setIntegrationEnabled(!renderer->integrationEnabled());
        renderer->resetStats();
        std::string status = renderer->integrationEnabled() ? "on" : "off";
        std::cout << "Automatic RT rerendering with integration: " << status << std::endl;
        std::cout << "Stats have been reset" << std::endl;
    } else if (key == GLFW_KEY_UP) {
        renderer->setDepth(renderer->getDepth() + 1);
        std::cout << "Set depth to " << renderer->getDepth() << std::endl;
    } else if (key == GLFW_KEY_DOWN) {
        if (renderer->getDepth() > 1) {
            renderer->setDepth(renderer->getDepth() - 1);
            std::cout << "Set depth to " << renderer->getDepth() << std::endl;
        }
    } else if (key == GLFW_KEY_LEFT) {
        renderer->setKSpecular(std::max(renderer->kSpecular() - 0.05f, 0.0f));
        std::cout << "Set reflection factor to " << renderer->kSpecular() << std::endl;
    } else if (key == GLFW_KEY_RIGHT) {
        renderer->setKSpecular(std::min(renderer->kSpecular() + 0.05f, 1.0f));
        std::cout << "Set reflection factor to " << renderer->kSpecular() << std::endl;
    } else if (key == GLFW_KEY_PAGE_DOWN) {
        renderer->setKRougness(std::max(renderer->kRoughness() - 0.01f, 0.0f));
        std::cout << "Set roughness factor to " << renderer->kRoughness() << std::endl;
    } else if (key == GLFW_KEY_PAGE_UP) {
        renderer->setKRougness(std::min(renderer->kRoughness() + 0.01f, 1.0f));
        if (renderer->kRoughness() > 1)
            renderer->setKSpecular(1);
        std::cout << "Set roughness factor to " << renderer->kRoughness() << std::endl;
    } else if (key == GLFW_KEY_H) {
        std::cout << "##################" << std::endl;
        std::cout << "Forward:  " << glm::to_string(camera->forward()) << std::endl;
        std::cout << "Up:     " << glm::to_string(camera->up()) << std::endl;
        std::cout << "Right:  " << glm::to_string(camera->right()) << std::endl;
        std::cout << "Position: " << glm::to_string(camera->position()) << std::endl;
        std::cout << "#################" << std::endl;
    } else if (key == GLFW_KEY_C && action == GLFW_PRESS) {
        cameraCurve->addControlPoint(camera->position());
        cameraCurve->finish();
        std::cout << "Added control point" << std::endl;
    } else if (key == GLFW_KEY_P && action == GLFW_PRESS) {
        std::cout << "Started animation" << std::endl;
        Animation *a = new MoveCameraAnimation(cameraCurve, 3000.0f, camera);
        Animator::registerAnimation(a);
    }
}

bool mouseEnabled = false;

void mouse_button_callback(GLFWwindow *window, int button, int action, int mods) {
    if (!mouseEnabled)
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    mouseEnabled = true;
    glfwSetCursorPos(window, (float)width / 2, (float)height / 2);
}

void cursor_position_callback(GLFWwindow *window, double xpos, double ypos) {
    if (!mouseEnabled)
        return;

    int dx = (float)width / 2 - xpos;
    int dy = (float)height / 2 - ypos;
    Camera *camera = renderer->getCamera();

    camera->rotate(mouseSensitivity * dx, mouseSensitivity * dy);
    camera->recalculateMatrix();

    glfwSetCursorPos(window, (float)width / 2, (float)height / 2);
}

void window_focus_callback(GLFWwindow *window, int focused) {
    if (!focused) {
        mouseEnabled = false;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
}

int main(int argc, char *argv[]) {
    std::string execDirectory(argv[0], 0, std::string(argv[0]).find_last_of("\\/"));

    Shader::setBaseDirectory(execDirectory + "/shaders");
    Importer::setPath(execDirectory + "/resources");

    /*********************************************************************************************/
    WindowManager manager(width, height, 60, 1.0, "Renderer");

    GLCheckError();
    glfwSetKeyCallback(manager.window, key_callback);
    glfwSetMouseButtonCallback(manager.window, mouse_button_callback);
    if (glfwRawMouseMotionSupported())
        glfwSetInputMode(manager.window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
    glfwSetWindowFocusCallback(manager.window, window_focus_callback);
    glfwSetCursorPosCallback(manager.window, cursor_position_callback);
    glfwSetFramebufferSizeCallback(
        manager.window, framebuffer_size_callback); // funkcija koja se poziva prilikom mijenjanja velicine prozora
    GLCheckError();

    // glEnable(GL_CULL_FACE); //ukljuci uklanjanje straznjih poligona -- za ovaj primjer je iskljuceno
    // glCullFace(GL_BACK);

    renderer = new Renderer(manager.window, width, height);
    GLCheckError();

    /*********************************************************************************************/
    Shader *phongShader = Shader::Load("phong");

    Mesh *glavaMesh = Mesh::Load("glava");
    MeshObject *glava = new MeshObject("glavaRobota", glavaMesh, phongShader);

    movingObject = glava;

    glm::vec3 min, max;
    glavaMesh->getBoundingBox(min, max);
    glava->getTransform()->normalize(min, max);
    glava->getTransform()->translate(glm::vec3(-1, 0, 0));

    renderer->AddObject(glava);
    GLCheckError();

    Mesh *kockaMesh = Mesh::Load("kocka", glm::vec3(1, 0.2, 0.3));
    Mesh *kockaMesh2 = Mesh::Load("kocka", glm::vec3(0.2, 0.2, 0.8));
    Mesh *kockaMesh3 = Mesh::Load("kocka", glm::vec3(0.2, 0.8, 0.6));
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
    floor->getTransform()->rotate(TransformIdentity::up(), 45.0f);
    floor->getTransform()->scale(glm::vec3(30.0f, 0.1f, 30.0f));

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

    // Mesh *catMesh = Mesh::Load("cat");
    //  Object *cat = new Object(catMesh, phongShader);

    // struct BoundingBox box2 = catMesh->getBoundingBox();
    // cat->getTransform()->normalize(box2.minX, box2.minY, box2.minZ, box2.maxX, box2.maxY, box2.maxZ);
    // cat->getTransform()->setPosition(glm::vec3(-2, 1, -1));
    // cat->getTransform()->rotate(TransformIdentity::right(), -90.0f);

    // renderer->AddObject(cat);

    Camera *camera = renderer->getCamera();
    camera->translate(glm::vec3(3.0f, 3.0f, -3.0f));
    camera->rotate(145, -30);
    camera->recalculateMatrix();

    Light *l = new Light(3, 3.1, -0.5, 1, 1, 1, 1, 1, 1);
    renderer->AddLight(l);

    // SphereObject *zutaKugla = new SphereObject("zutaKugla", glm::vec3(-2, 2, -4), 2, glm::vec3(1, 1, 0));
    // zutaKugla->mesh->material->colorReflective = glm::vec3(0.5);
    // renderer->AddObject(zutaKugla);

    // SphereObject *prozirnaKugla = new SphereObject("zutaKugla", glm::vec3(5, 2, 4), 2, glm::vec3(0.3, 1, 0));
    // prozirnaKugla->mesh->material->colorReflective = glm::vec3(1);
    // renderer->AddObject(prozirnaKugla);

    cameraCurve = new BSpline();
    renderer->AddObject(cameraCurve);

    Mesh *planeMesh = Mesh::Load("airplane");
    MeshObject *plane = new MeshObject("avion", planeMesh, phongShader);
    renderer->AddObject(plane);

    Cubemap skybox = Cubemap::Load({
        "skybox/right.png",
        "skybox/left.png",
        "skybox/top.png",
        "skybox/bottom.png",
        "skybox/front.png",
        "skybox/back.png",
    });

    renderer->SetSkybox(&skybox);

    renderer->EnableVSync();
    GLCheckError();

    // glavna petlja za prikaz
    while (!glfwWindowShouldClose(manager.window)) {
        float deltaTime = (float)manager.LimitFPS(false);

        Animator::passTime(1000 * deltaTime);

        if (renderer->getRenderingMethod() != RenderingMethod::Noop) {
            renderer->Clear();
        }
        GLCheckError();
        renderer->Render();
        GLCheckError();

        // crosshair
        // glUseProgram(0);

        // glColor3f(1.0f, 0.0f, 0.0f); // Red color
        // glLineWidth(2.0f);

        // float size = 0.02f;
        // glBegin(GL_LINES);
        // glVertex2f(-size, 0.0f);
        // glVertex2f(size, 0.0f);
        // glVertex2f(0.0f, -size);
        // glVertex2f(0.0f, size);
        // glEnd();

        if (renderer->getRenderingMethod() == RenderingMethod::Noop) {
            std::this_thread::sleep_for(std::chrono::microseconds(16666));
        } else {
            renderer->SwapBuffers();
        }

        // stop rendering raytracing after first render
        if (!renderer->integrationEnabled() && renderer->getRenderingMethod() != RenderingMethod::Rasterize) {
            renderer->setRenderingMethod(RenderingMethod::Noop);
        }

        manager.PollEvents();

        float multiplier = moveSensitivity * deltaTime;
        // camera->getTransform()->translate(camera->getTransform()->forward() * 0.01f);
        // camera->recalculateMatrix();

        if (glfwGetKey(manager.window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
            multiplier *= sprintMultiplier;
        }
        if (glfwGetKey(manager.window, GLFW_KEY_W) == GLFW_PRESS) {
            // camera->translate(multiplier * Transform::Identity().forward());
            camera->setPosition(camera->position() + multiplier * camera->forward());
            camera->recalculateMatrix();
            // camera->translate(multiplier * camera->forward());
        }
        if (glfwGetKey(manager.window, GLFW_KEY_A) == GLFW_PRESS) {
            camera->translate(multiplier * -TransformIdentity::right());
            camera->recalculateMatrix();
        }
        if (glfwGetKey(manager.window, GLFW_KEY_S) == GLFW_PRESS) {
            camera->translate(multiplier * -TransformIdentity::forward());
            camera->recalculateMatrix();
        }
        if (glfwGetKey(manager.window, GLFW_KEY_D) == GLFW_PRESS) {
            camera->translate(multiplier * TransformIdentity::right());
            camera->recalculateMatrix();
        }
        if (glfwGetKey(manager.window, GLFW_KEY_SPACE) == GLFW_PRESS) {
            camera->translate(multiplier * camera->vertical());
            camera->recalculateMatrix();
        }
        if (glfwGetKey(manager.window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
            camera->translate(-multiplier * camera->vertical());
            camera->recalculateMatrix();
        }
        if (glfwGetKey(manager.window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(manager.window, true);
        }
    }

    glfwTerminate();

    delete phongShader;

    return EXIT_SUCCESS;
}
