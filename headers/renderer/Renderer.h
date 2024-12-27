#pragma once

#include "models/Light.h"
#include "objects/Object.h"
#include "renderer/Camera.h"
#include "renderer/InputSystem.h"
#include "renderer/ParticleSystem.h"
#include "renderer/WindowManager.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include <memory>
#include <objects/Skybox.h>
#include <optional>

#define RAYTRACE_MULTICORE 1
#define RAYTRACE_DEPTH 1
#define RAYTRACE_NUM_OF_SAMPLES 1
#define RAYTRACE_RANDOMNESS 0
#define RAYTRACE_OFFSET 1
#define RAYTRACE_AMBIENT glm::vec3(0.2, 0.2, 0.2)

#define K_ROUGNESS 0.0f // pathrace diffusion
#define K_SPECULAR 0.0f // reflection
#define k_transmit 0.0f // translucency

enum RenderingMethod {
    Noop,
    Rasterize,
    Raycast,
    Raytrace,
    Pathtrace,
};

typedef glm::vec3 (*RayStrategy)(glm::vec3, glm::vec3, int);

// Nenjin
class Renderer : CameraObserver {
  public:
    Renderer(int width, int height);

    WindowManager *manager = nullptr;
    InputSystem input;

    void SetResolution(int width, int height);
    void SetSkybox(Cubemap *cb) { skybox = new Skybox(cb); };

    void AddObject(Object *o);
    void AddLight(Light *l);
    void AddParticleCluster(ParticleCluster *pc);

    Camera *GetCamera();
    void onCameraChange(); // private?

    void Loop();
    void Render();
    void Clear();
    void SetShouldClose(); // marks the engine close after the current frame finishes rendering

    void EnableVSync();
    void DisableVSync();
    void SwapBuffers();

    // RT settings, TODO: move

    void resetStats() {
        renderCount = 0;
        totalTime = 0;
    }

    unsigned int getDepth() { return depth; };
    void setDepth(int depth) { this->depth = depth; };

    unsigned int getRenderingMethod() { return method; };
    void setRenderingMethod(RenderingMethod method) { this->method = method; };

    unsigned int integrationEnabled() { return monteCarlo; }
    void setIntegrationEnabled(bool enabled) { monteCarlo = enabled; }

    float kSpecular() { return k_specular; }
    void setKSpecular(float k_specular) { this->k_specular = k_specular; }

    float kRoughness() { return k_roughness; }
    void setKRougness(float k_roughness) { this->k_roughness = k_roughness; }

    void rasterize();
    void rayRender();

    void line(glm::vec3 current, glm::vec3 dx, glm::vec3 dy, int i);

    glm::vec3 phong(Intersection &p, glm::vec3 diffuseColor);
    std::optional<Intersection> raycast(glm::vec3 origin, glm::vec3 direction, Object *&intersectedObject);

    glm::vec3 raycast(glm::vec3 origin, glm::vec3 direction);              // returns color
    glm::vec3 raytrace(glm::vec3 origin, glm::vec3 direction, int depth);  // returns color
    glm::vec3 pathtrace(glm::vec3 origin, glm::vec3 direction, int depth); // returns color
    void iscrtajRaster();
    void spremiRaster();

  private:
    std::vector<Object *> objects;
    std::vector<Light *> lights;
    std::vector<float> lightPositions;
    std::vector<float> lightIntensities;
    std::vector<float> lightColors;

    std::unique_ptr<Camera> camera;
    Skybox *skybox = nullptr;
    int _width;
    int _height;
    glm::vec3 _clearColor;

    bool _cameraMatrixChanged = true;
    Shader *lightMapShader;

    unsigned int depth = 0;
    RenderingMethod method;

    unsigned int renderCount = 0;
    bool monteCarlo = false;
    float totalTime = 0;

    float k_specular = K_SPECULAR;
    float k_roughness = K_ROUGNESS;

    void UpdateShader(Object *object, glm::mat4 projMat, glm::mat4 viewMat);
};
