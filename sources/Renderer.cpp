#include "Renderer.h"
#include "Camera.h"
#include "Framebuffer.h"
#include "Mesh.h"
#include "Raster.h"
#include "Shader.h"
#include "Texture.h"
#include "TextureObject.h"
#include "ThreadPool.h"
#include "Timer.h"
#include "mtr.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <GLFW/glfw3.h>
#include <future>
#include <glm/glm.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/compatibility.hpp>
#include <glm/gtx/string_cast.hpp>

#include <algorithm>
#include <iostream>
#include <map>
#include <mutex>
#include <stdexcept>

std::vector<Raster *> rasteri;
int currentRasterIndex = 0;

#define LIGHT_MAX_RANGE 20
// #define RAYTRACE_AMBIENT glm::vec3(0, 0, 0)

#define SKYBOX_COLOR glm::vec3(0.3, 0.4, 1)

#define RENDER_SHADOWMAPS 0

Timer t = Timer::start();

Texture *tx;
TextureObject *to;
Framebuffer *fb;

Renderer::Renderer(GLFWwindow *w, int width, int height) {
    _window = w;
    setResolution(width, height);
    setDepth(RAYTRACE_DEPTH);
    setRenderingMethod(Rasterize);

    _clearColor = SKYBOX_COLOR;

    // boja brisanja platna izmedu iscrtavanja dva okvira
    glClearColor(_clearColor[0], _clearColor[1], _clearColor[2], 1);
    glEnable(GL_DEPTH_TEST); // ukljuci z spremnik
    glDepthFunc(GL_LESS);

    glEnable(GL_CULL_FACE);
    glEnable(GL_FRONT_AND_BACK);

    _camera = new Camera(width, height);
    _camera->addChangeListener(this);

    tx = new Texture(width, height);
    to = new TextureObject("texture"); // ili depthMapTexture
    to->setTexture(tx);
    fb = new Framebuffer();
    fb->setDepthTexture(tx);

    int RASTER_NUM = 2;

    for (int i = 0; i < RASTER_NUM; i++) {
        Raster *r = new Raster(width, height);
        rasteri.push_back(r);
    }

    lightMapShader = Shader::load("depthMap");
}

Renderer::~Renderer() { delete _camera; }

void Renderer::setResolution(int width, int height) {
    _width = width;
    _height = height;
    for (Raster *r : rasteri) {
        r->resize(width, height);
    }
    if (_camera != nullptr) {
        _camera->setSize(width, height);
    }
}

Camera *Renderer::getCamera() { return _camera; }

void Renderer::AddObject(Object *o) { objects.push_back(o); }

void Renderer::AddLight(Light *l) {
    lightPositions.insert(lightPositions.end(), {l->position[0], l->position[1], l->position[2]});
    lightIntensities.insert(lightIntensities.end(), {l->intensity[0], l->intensity[1], l->intensity[2]});
    lightColors.insert(lightColors.end(), {l->color[0], l->color[1], l->color[2]});
}

void Renderer::Render() {
    switch (method) {
    case Rasterize:
        rasterize();
        break;
    case Raycast:
    case Raytrace:
    case Pathtrace:
        rayRender();
        break;
    case Noop:
        break;
    default:
        assert(method);
    }
}

ThreadPool pool(0);

void Renderer::line(glm::vec3 current, glm::vec3 dx, glm::vec3 dy, int i) {
    glm::vec3 camPos = _camera->position();
    glm::vec3 boja, target;
    float offsetx, offsety;

    for (int j = 0; j < _width; j++) {
        target = current;
        switch (method) {
        case Raycast:
            boja = raycast(camPos, target - camPos);
            break;
        case Raytrace:
            boja = raytrace(camPos, target - camPos, getDepth());
            break;
        case Pathtrace:
            offsetx = ((double)rand() / (RAND_MAX));
            offsety = ((double)rand() / (RAND_MAX));
            target = current + dx * offsetx + dy * offsety;
            boja = pathtrace(camPos, target - camPos, getDepth());
            break;
        default:
            std::runtime_error("unknown renderer type");
        }
        rasteri[currentRasterIndex]->setFragmentColor(j, i, boja);
        current += dx;
    }
    rasteri[currentRasterIndex]->setFragmentColor(0, 0, glm::vec3(0, 1, 0));
    rasteri[currentRasterIndex]->setFragmentColor(1, 0, glm::vec3(0, 1, 0));
    rasteri[currentRasterIndex]->setFragmentColor(0, 1, glm::vec3(0, 1, 0));
    rasteri[currentRasterIndex]->setFragmentColor(1, 1, glm::vec3(0, 1, 0));
}

void Renderer::rayRender() {
    glm::vec3 camPos = _camera->position();

    t.reset();

    CameraConstraints c = _camera->constraints;

    glm::vec3 start = camPos + c.nearp * _camera->forward() + c.top * _camera->up() + c.left * _camera->right();
    glm::vec3 current = start;
    glm::vec3 row = (c.right - c.left) * _camera->right();
    glm::vec3 dx = row * (1.0f / _width);
    glm::vec3 column = -(c.top - c.bottom) * _camera->up();
    glm::vec3 dy = column * (1.0f / _height);

    to->shader->use();

    pool.setJobQueue(_height);
    for (int i = 0; i < _height; i++) {
        current = start + column * ((float)i / (_height - 1));
#if RAYTRACE_MULTICORE
        pool.enqueue(&Renderer::line, this, current, dx, dy, i);
#else
        line(current, dx, dy, i);
#endif
    }
#if RAYTRACE_MULTICORE
    pool.wait();
#endif

    std::cout << "Render done, number of renders: " << ++renderCount << std::endl;
    t.printElapsed("Time elapsed since last render: \t");
    totalTime += t.elapsed();
    std::cout << "Total elapsed time rendering:   \t" << totalTime << "ms" << std::endl;

    if (monteCarlo) {
        Raster *current = rasteri[currentRasterIndex];
        Raster *other = rasteri[!currentRasterIndex];
        for (int i = 0; i < _height; i++) {
            for (int j = 0; j < _width; j++) {
                glm::vec3 c1 = current->getFragmentColor(j, i);
                glm::vec3 c2 = other->getFragmentColor(j, i);
                glm::vec3 color = ((c1 * (float)(renderCount - 1)) + c2) * (1.0f / renderCount);
                rasteri[currentRasterIndex]->setFragmentColor(j, i, color);
            }
        }
    }

    iscrtajRaster();

    _cameraMatrixChanged = false;
}

void Renderer::rasterize() {
    glm::mat4 pv = _camera->getProjectionViewMatrix();
    glm::vec3 lightPos(-3, 3, 2);

    glm::mat4 lightProjection, lightView;
    glm::mat4 lightSpaceMatrix;
    float near_plane = 0.1f, far_plane = 50.0f;
    float size = 2.0f;
    lightProjection = glm::ortho(-size, size, -size, size, near_plane, far_plane);
    lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
    lightSpaceMatrix = lightProjection * lightView;

    lightMapShader->use();

    GLCheckError();
    fb->use();
    fb->setupDepth();
    GLCheckError();

    for (Object *o : objects) {
        // postavite model matricu za objekt
        UpdateShader(o, lightSpaceMatrix);
        o->render();
    }
    fb->cleanDepth();
    glViewport(0, 0, _width, _height);

    for (Object *o : objects) {
        UpdateShader(o, pv);
        glUniformMatrix4fv(glGetUniformLocation(o->shader->ID, "lightSpaceMatrix"), 1, GL_FALSE,
                           glm::value_ptr(lightSpaceMatrix));
        o->render();
    }
    return;
}

void Renderer::UpdateShader(Object *object, glm::mat4 projViewMat) {
    Shader *shader = object->shader;
    glm::vec3 cameraPos = _camera->position();

    shader->use();

    shader->setUniform(SHADER_MMATRIX, 1, object->getModelMatrix());
    shader->setUniform(SHADER_PVMATRIX, 1, projViewMat);
    shader->setUniform(SHADER_CAMERA, 1, cameraPos);

    shader->setUniform(SHADER_LIGHT_NUM, int(lightPositions.size() / 3));
    shader->setUniform(SHADER_LIGHT_POSITION, lightPositions.size() / 3, lightPositions);
    shader->setUniform(SHADER_LIGHT_INTENSITY, lightIntensities.size() / 3, lightIntensities);
    shader->setUniform(SHADER_LIGHT_COLOR, lightColors.size() / 3, lightColors);

    bool hasTextures = false;
    if (object->getMesh()->materials.size() > 0) {
        std::vector<Materials> mats = object->getMesh()->materials;
        Materials m = mats[mats.size() - 1];
        shader->setUniform(SHADER_MATERIAL_COLOR_AMBIENT, 1, m.colorAmbient);
        shader->setUniform(SHADER_MATERIAL_COLOR_DIFFUSE, 1, m.colorDiffuse);
        shader->setUniform(SHADER_MATERIAL_COLOR_SPECULAR, 1, m.colorSpecular);
        shader->setUniform(SHADER_MATERIAL_SHININESS, m.shininess);
        shader->setUniform(SHADER_MATERIAL_COLOR_REFLECTIVE, 1, m.colorReflective);
        shader->setUniform(SHADER_MATERIAL_COLOR_EMISSIVE, 1, m.colorEmissive);

        if (m.texture > 0) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, m.texture);
            shader->setUniform(SHADER_TEXTURE, 0);
            hasTextures = true;
        }
    }
    shader->setUniform(SHADER_HAS_TEXTURES, hasTextures);

    glActiveTexture(GL_TEXTURE1);
    tx->use();
    shader->setUniform(SHADER_SHADOWMAP, 1);
    shader->setUniform(SHADER_HAS_SHADOWMAP, RENDER_SHADOWMAPS);
}

void Renderer::Clear() { glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); }

void Renderer::onCameraChange() { _cameraMatrixChanged = true; }

glm::vec3 calculateLight(const glm::vec3 &lightPos, const glm::vec3 &lightIntensity, const glm::vec3 &lightColor,
                         const glm::vec3 &normal, const glm::vec3 shadingPoint, const glm::vec3 &cameraPos) {
    glm::vec3 lightDir = glm::normalize(lightPos - shadingPoint);
    glm::vec3 cameraDir = glm::normalize(cameraPos - shadingPoint);

    // diffuse
    float diffuseStrength = std::max(0.0f, glm::dot(lightDir, normal));

    // specular
    glm::vec3 reflected = glm::normalize(glm::reflect(-lightDir, normal));
    float specularBase = std::max(0.0f, glm::dot(cameraDir, reflected));
    float specularStrength = glm::pow(specularBase, 32);

    float d = glm::distance(lightPos, shadingPoint);
    float i = glm::max((LIGHT_MAX_RANGE - d) / LIGHT_MAX_RANGE, 0.0f);

    glm::vec3 a = lightColor * (diffuseStrength + specularStrength) * lightIntensity * i;
    // std::cout << "CalcL: " << diffuseStrength << " --- " << specularStrength << " --> " << glm::to_string(a) <<
    // std::endl;
    return a;
}

glm::vec3 Renderer::phong(Intersection &p, glm::vec3 diffuseColor) {
    glm::vec3 lpos = glm::vec3(lightPositions[0], lightPositions[1], lightPositions[2]);
    glm::vec3 lint = glm::vec3(lightIntensities[0], lightIntensities[1], lightIntensities[2]);
    glm::vec3 lcol = glm::vec3(lightColors[0], lightColors[1], lightColors[2]);

    glm::vec3 light = diffuseColor;

    Object *o = nullptr;
    std::optional<Intersection> p2 = raycast(p.point, lpos - p.point, o); // shadow ray

    // std::cout << "########" << std::endl;
    float t = p2.has_value() ? p2.value().t : -696969;
    // std::cout << p2.has_value() << " -O- " << t << std::endl;

    if (!p2.has_value() || p2.value().t > 1) {
        glm::vec3 c = calculateLight(lpos, lint, lcol, p.normal, p.point, _camera->position());
        light += c;
        // std::cout << "C    : " << glm::to_string(c) << std::endl;
    }
    // std::cout << "Light: " << glm::to_string(light) << std::endl;

    glm::vec3 a = light * p.color;
    // std::cout << "Combi: " << glm::to_string(light) << " --- " << glm::to_string(p.color) << " --> " <<
    // glm::to_string(a) << std::endl;
    return a;
}

std::optional<Intersection> Renderer::raycast(glm::vec3 origin, glm::vec3 direction, Object *&intersectedObject) {
    Intersection intersect;
    bool found = false;

    for (Object *o : objects) {
        std::optional<Intersection> p = o->findIntersection(origin, direction);
        if (!p.has_value()) {
            continue;
        }
        // std::cout << p.value().t << std::endl;
        if (!found || (p.value().t < intersect.t && p.value().t > 1e-5)) {
            intersect = p.value();
            intersectedObject = o;
            found = true;
        }
    }
    // std::cout << "Chosen " << intersect.t << std::endl;
    if (!found)
        return std::nullopt;

    return intersect;
}

glm::vec3 Renderer::raycast(glm::vec3 origin, glm::vec3 direction) {
    // Object *intersectedObject = nullptr;
    // IntersectPoint intersect = raycast(origin, direction, intersectedObject);
    // return intersectedObject ? phong(intersect, glm::vec3(0.2, 0.2, 0.2)) : _clearColor;
    return raytrace(origin, direction, 1);
}

int test = 1;
glm::vec3 Renderer::raytrace(glm::vec3 origin, glm::vec3 direction, int depth) {
    if (depth == 0)
        return glm::vec3(0);

    Object *object = nullptr;
    // std::cout << "####" << std::endl;
    std::optional<Intersection> intersection = raycast(origin, direction, object);
    if (intersection.has_value()) {
        // std::cout << "RAYTRCE COLLISION " << glm::to_string(origin) << " " << glm::to_string(direction) << " "
        //          << glm::to_string(intersection.value().point) << std::endl;
    }
    // std::cout << "###" << std::endl;
    if (!intersection.has_value())
        return _clearColor;

    Intersection p = intersection.value();

    glm::vec3 light = RAYTRACE_AMBIENT;
    glm::vec3 normal = p.normal;
    glm::vec3 color = p.color * light + phong(p, glm::vec3(0));
    // std::cout << "Color: " << glm::to_string(color) << std::endl;

    if (k_specular > 0) {
        color += k_specular * raytrace(p.point, glm::reflect(direction, normal), depth - 1);
    }
    if (k_transmit > 0) {
        float eta = 1.0f;
        glm::vec3 refractedDir = glm::refract(direction, normal, eta);
        if (glm::length(refractedDir) > 0) {
            color += k_transmit * raytrace(p.point, refractedDir, depth - 1);
        }
    }

    return color;
}

glm::vec3 Renderer::pathtrace(glm::vec3 origin, glm::vec3 direction, int depth) {
    if (depth == 0)
        return glm::vec3(0);

    Object *object = nullptr;
    std::optional<Intersection> intersection = raycast(origin, direction, object);
    if (!intersection.has_value())
        return _clearColor;

    Intersection p = intersection.value();

    glm::vec3 light = RAYTRACE_AMBIENT;
    glm::vec3 normal = p.normal;
    glm::vec3 color = p.color * light + phong(p, glm::vec3(0));

    if (k_specular > 0) {
        glm::vec3 randomDirection = glm::reflect(direction, normal + k_roughness * mtr::linearRandVec3(-0.5f, 0.5f));
        color += k_specular * pathtrace(p.point, randomDirection, depth - 1);
    }

    return color;
}

void Renderer::iscrtajRaster() {
    to->render(rasteri[currentRasterIndex]);

    currentRasterIndex = !currentRasterIndex;
}

void Renderer::spremiRaster() {
    int *buffer = new int[_width * _height * 3];
    glReadPixels(0, 0, _width, _height, GL_BGR, GL_UNSIGNED_BYTE, buffer);
}

void Renderer::EnableVSync() { glfwSwapInterval(1); }

void Renderer::DisableVSync() { glfwSwapInterval(0); }

void Renderer::SwapBuffers() { glfwSwapBuffers(_window); }
