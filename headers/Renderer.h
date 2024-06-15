#pragma once

#include "Camera.h"
#include "Object.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>

#define RAYTRACE_DEPTH 5
#define RAYTRACE_NUM_OF_SAMPLES 1
#define RAYTRACE_RANDOMNESS 0
#define RAYTRACE_OFFSET 1
#define RAYTRACE_AMBIENT glm::vec3(0.2, 0.2, 0.2)

#define K_ROUGNESS 0.1f  // pathrace diffusion
#define K_SPECULAR 0.2f // reflection
#define k_transmit 0.0f // translucency

enum RenderingMethod {
    Noop,
    Rasterize,
    Raycast,
    Raytrace,
    Pathtrace,
};

typedef glm::vec3 (*RayStrategy)(glm::vec3, glm::vec3, int);

class Renderer : CameraObserver {
  public:
    Renderer(GLFWwindow *w, int width, int height);
    ~Renderer();

    Camera *getCamera();

    void setResolution(int width, int height);
    void AddObject(Object *o);
    void AddLight(Light *l);

    uint getDepth() { return depth; };
    void setDepth(int depth) { this->depth = depth; };

    uint getRenderingMethod() { return method; };
    void setRenderingMethod(RenderingMethod method) { this->method = method; };

    uint integrationEnabled() { return monteCarlo; }
    void setIntegrationEnabled(bool enabled) { monteCarlo = enabled; }

    void resetStats() {
        renderCount = 0;
        totalTime = 0;
    }

    float kSpecular() { return k_specular; }
    void setKSpecular(float k_specular) { this->k_specular = k_specular; }

    float kRougness() { return k_roughness; }
    void setKRougness(float k_roughness) { this->k_roughness = k_roughness; }

    void Render();
    void Clear();

    void onCameraChange();

    void rasterize();
    void rayRender();

    void line(glm::vec3 current, glm::vec3 dx, glm::vec3 dy, int i);

    glm::vec3 phong(IntersectPoint &p, glm::vec3 diffuseColor, glm::vec3 normal); // cached normal
    IntersectPoint raycast(glm::vec3 origin, glm::vec3 direction, Object *&intersectedObject);

    glm::vec3 raycast(glm::vec3 origin, glm::vec3 direction);              // returns color
    glm::vec3 raytrace(glm::vec3 origin, glm::vec3 direction, int depth);  // returns color
    glm::vec3 pathtrace(glm::vec3 origin, glm::vec3 direction, int depth); // returns color
    void osvijetliFragment(int x, int y, glm::vec3 boja);
    glm::vec3 getFragmentColor(int x, int y, float *raster);
    void iscrtajRaster();
    void spremiRaster();

    void EnableVSync();
    void DisableVSync();
    void SwapBuffers();

  private:
    std::vector<Object *> objects;
    // std::vector<Light*> lights;
    std::vector<float> lightPositions;
    std::vector<float> lightIntensities;
    std::vector<float> lightColors;

    Camera *_camera = nullptr;
    int _width;
    int _height;
    glm::vec3 _clearColor;
    GLFWwindow *_window;

    bool _cameraMatrixChanged = true;
    Shader *lightMapShader;

    uint depth = 0;
    RenderingMethod method;

    uint renderCount = 0;
    bool monteCarlo = false;
    uint totalTime = 0;

    float k_specular = K_SPECULAR;
    float k_roughness = K_ROUGNESS;

    void UpdateShader(Object *object, glm::mat4 projMat);
};
