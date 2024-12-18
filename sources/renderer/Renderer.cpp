#include "renderer/Renderer.h"

#include "models/Mesh.h"
#include "models/Raster.h"
#include "objects/FullscreenTexture.h"
#include "renderer/Framebuffer.h"
#include "renderer/ParticleSystem.h"
#include "renderer/Shader.h"
#include "renderer/Texture.h"
#include "utils/GLDebug.h"
#include "utils/ThreadPool.h"
#include "utils/Timer.h"
#include "utils/mtr.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/compatibility.hpp>
#include <glm/gtx/string_cast.hpp>

#include <algorithm>
#include <iostream>
#include <stdexcept>

std::vector<Raster<float> *> rasteri;
int currentRasterIndex = 0;

#define LIGHT_MAX_RANGE 20

#define SKYBOX_COLOR glm::vec3(0.3, 0.4, 1)

#define RENDER_SHADOWMAPS 1

Timer t = Timer::start();

Texture *rasterTexture = nullptr;
FullscreenTexture *textureShower = nullptr;
Framebuffer *depthFramebuffer = nullptr;
Shader *rt = nullptr;
PointLight *light = nullptr;

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
    glDepthMask(GL_TRUE);

    glEnable(GL_CULL_FACE);
    glEnable(GL_FRONT_AND_BACK);

    glEnable(GL_PROGRAM_POINT_SIZE);

    camera = std::make_unique<Camera>(width, height);
    camera->addChangeListener(this);

    rasterTexture = new Texture(GL_TEXTURE_2D, width, height);
    textureShower = new FullscreenTexture("tekstura", "texture"); // ili depthMapTexture
    textureShower->setTexture(rasterTexture);

    depthFramebuffer = new Framebuffer();

    int RASTER_NUM = 2;

    for (int i = 0; i < RASTER_NUM; i++) {
        Raster<float> *r = new Raster<float>(width, height);
        rasteri.push_back(r);
    }

    lightMapShader = Shader::Load("pointLight");
    rt = Shader::LoadCompute("raytrace");
}

void Renderer::setResolution(int width, int height) {
    _width = width;
    _height = height;
    if (rasterTexture != nullptr) {
        rasterTexture->setSize(width, height);
    }
    for (Raster<float> *r : rasteri) {
        r->resize(width, height);
    }
    if (camera != nullptr) {
        camera->setSize(width, height);
    }
}

Camera *Renderer::getCamera() { return camera.get(); }

void Renderer::AddObject(Object *o) { objects.push_back(o); }

void Renderer::AddLight(Light *l) {
    lights.push_back(l);

    light = dynamic_cast<PointLight *>(l);
    assert(light != nullptr);
    depthFramebuffer->setDepthTexture(&light->cb); // enough to be ran only once
}

void Renderer::AddParticleCluster(ParticleCluster *pc) {
    ParticleSystem::registerCluster(pc);
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
    glm::vec3 camPos = camera->position();
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
}

void Renderer::rayRender() {
    t.reset();

    glm::vec3 camPos = camera->position();
    CameraConstraints c = camera->constraints;

    glm::vec3 start = camPos + c.nearPlane * camera->forward() + c.top * camera->up() + c.left * camera->right();
    glm::vec3 current = start;
    glm::vec3 row = (c.right - c.left) * camera->right();
    glm::vec3 dx = row * (1.0f / _width);
    glm::vec3 column = -(c.top - c.bottom) * camera->up();
    glm::vec3 dy = column * (1.0f / _height);

    textureShower->shader->use();

#if RAYTRACE_MULTICORE
    pool.setJobQueue(_height);
    for (int i = 0; i < _height; i++) {
        current = start + column * ((float)i / (_height - 1));
        pool.enqueue(&Renderer::line, this, current, dx, dy, i);
    }
    pool.wait();
#endif
#if !RAYTRACE_MULTICORE
    for (int i = 0; i < _height; i++) {
        current = start + column * ((float)i / (_height - 1));
        line(current, dx, dy, i);
    }
#endif

    // rt->compute(_width, _height);
    // textureShower->setTexture(tx);
    // textureShower->render();

    std::cout << "Render done, number of renders: " << ++renderCount << std::endl;
    t.printElapsed("Time elapsed since last render: ");
    totalTime += t.elapsed();
    std::cout << "Total elapsed time rendering:   " << totalTime << "ms" << std::endl;

    if (monteCarlo) {
        Raster<float> *current = rasteri[currentRasterIndex];
        Raster<float> *other = rasteri[!currentRasterIndex];
        for (int i = 0; i < _height; i++) {
            for (int j = 0; j < _width; j++) {
                glm::vec3 c1 = current->getFragmentColor(j, i);
                glm::vec3 c2 = other->getFragmentColor(j, i);
                glm::vec3 color = (c1 + (c2 * (float)(renderCount - 1))) * (1.0f / renderCount);
                rasteri[currentRasterIndex]->setFragmentColor(j, i, color);
            }
        }
    }

    iscrtajRaster();
    currentRasterIndex = !currentRasterIndex;

    _cameraMatrixChanged = false;
}

void Renderer::rasterize() {
    // glm::vec3 lightPos(-3, 3, 2);

    // glm::mat4 lightProjection, lightView;
    // glm::mat4 lightSpaceMatrix;
    // float near_plane = 0.1f, far_plane = 50.0f;
    // float size = 2.0f;
    // lightProjection = glm::ortho(-size, size, -size, size, near_plane, far_plane);
    // lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
    // lightSpaceMatrix = lightProjection * lightView;

    // inefficient, but will make do
    lightPositions.clear();
    lightIntensities.clear();
    lightColors.clear();
    for (const auto &l : lights) {
        glm::vec3 pos = l->getTransform()->position();
        lightPositions.insert(lightPositions.end(), {pos[0], pos[1], pos[2]});
        lightIntensities.insert(lightIntensities.end(), {l->intensity[0], l->intensity[1], l->intensity[2]});
        lightColors.insert(lightColors.end(), {l->color[0], l->color[1], l->color[2]});
    }

    GLCheckError();
    depthFramebuffer->use();
    depthFramebuffer->setupDepth();
    GLCheckError();
    GLCheckFramebuffer();

    if (light != nullptr) {
        lightMapShader->use();
        lightMapShader->setFloat("farPlane", light->farPlane);
        lightMapShader->setMatrices("shadowMatrices", light->transforms);
        light->cb.use(0);
    }

    // 1st pass - depth
    for (Object *o : objects) {
        lightMapShader->setUniform(SHADER_MMATRIX, 1, o->getModelMatrix());
        lightMapShader->setVector("lightPos", light->getTransform()->position());
        o->render(lightMapShader);
    }
    depthFramebuffer->cleanDepth(_width, _height);

    // 2nd pass - scene with shadows
    if (skybox) {
        UpdateShader(skybox, camera->getProjectionMatrix(), camera->getViewMatrix());
        skybox->render();
    }
    for (Object *o : objects) {
        UpdateShader(o, camera->getProjectionMatrix(), camera->getViewMatrix());
        // if (o->shader != nullptr) { // TODO: remove this ugly stuff
        //     glUniformMatrix4fv(glGetUniformLocation(o->shader->ID, "lightSpaceMatrix"), 1, GL_FALSE,
        //                        glm::value_ptr(lightSpaceMatrix));
        // }
        o->render();
    }
    for (ParticleCluster *pc: ParticleSystem::clusters) {
        UpdateShader(pc, camera->getProjectionMatrix(), camera->getViewMatrix());
        pc->render();
    }
}

void Renderer::UpdateShader(Object *object, glm::mat4 projMat, glm::mat4 viewMat) {
    Shader *shader = object->shader;
    if (shader == nullptr)
        return;

    glm::vec3 cameraPos = camera->position();
    Transform viewTransform(viewMat);

    shader->use();

    shader->setUniform(SHADER_MMATRIX, 1, object->getModelMatrix());
    shader->setUniform(SHADER_PVMATRIX, 1, projMat * viewTransform.matrix);
    viewTransform.setPosition(glm::vec3(0));
    glm::mat4 centered = projMat * viewTransform.matrix;
    shader->setUniform(SHADER_PVCENTERMATRIX, 1, centered);
    shader->setUniform(SHADER_CAMERA, 1, cameraPos);

    shader->setUniform(SHADER_LIGHT_NUM, int(lightPositions.size() / 3));
    shader->setUniform(SHADER_LIGHT_POSITION, lightPositions.size() / 3, lightPositions);
    shader->setUniform(SHADER_LIGHT_INTENSITY, lightIntensities.size() / 3, lightIntensities);
    shader->setUniform(SHADER_LIGHT_COLOR, lightColors.size() / 3, lightColors);

    shader->setFloat("range", light ? light->farPlane : 10);

    if (object->mesh && object->mesh->material) {
        Material *m = object->mesh->material;
        shader->setUniform(SHADER_MATERIAL_COLOR_AMBIENT, 1, m->colorAmbient);
        shader->setUniform(SHADER_MATERIAL_COLOR_DIFFUSE, 1, m->colorDiffuse);
        shader->setUniform(SHADER_MATERIAL_COLOR_SPECULAR, 1, m->colorSpecular);
        shader->setUniform(SHADER_MATERIAL_SHININESS, m->shininess);
        shader->setUniform(SHADER_MATERIAL_COLOR_REFLECTIVE, 1, m->colorReflective);
        shader->setUniform(SHADER_MATERIAL_COLOR_EMISSIVE, 1, m->colorEmissive);

        if (m->texture > 0) {
            shader->setTexture(SHADER_TEXTURE, 0, m->texture);
        }
        shader->setUniform(SHADER_HAS_TEXTURES, m->texture > 0);
    }

    shader->setTexture(SHADER_SHADOWMAP, 1, rasterTexture->id);
    shader->setUniform(SHADER_HAS_SHADOWMAP, RENDER_SHADOWMAPS);

    light->cb.use(3);
    shader->setUniform(SHADER_SHADOWMAPCUBE, 3);
    shader->setUniform(SHADER_HAS_SHADOWMAPCUBE, RENDER_SHADOWMAPS);

    if (skybox != nullptr) {
        skybox->cubemap->use(2);
        shader->setUniform(SHADER_SKYBOX, 2);
    }
    shader->setUniform(SHADER_HAS_SKYBOX, skybox != nullptr);
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

    return lightColor * (diffuseStrength + specularStrength) * lightIntensity * i;
}

glm::vec3 Renderer::phong(Intersection &p, glm::vec3 diffuseColor) {
    glm::vec3 lpos = glm::vec3(lightPositions[0], lightPositions[1], lightPositions[2]);
    glm::vec3 lint = glm::vec3(lightIntensities[0], lightIntensities[1], lightIntensities[2]);
    glm::vec3 lcol = glm::vec3(lightColors[0], lightColors[1], lightColors[2]);

    glm::vec3 light = diffuseColor;

    Object *o = nullptr;
    std::optional<Intersection> p2 = raycast(p.point, lpos - p.point, o); // shadow ray

    if (!p2.has_value() || p2.value().t > 1) {
        glm::vec3 c = calculateLight(lpos, lint, lcol, p.normal, p.point, camera->position());
        light += c;
    }

    return light * p.color;
}

std::optional<Intersection> Renderer::raycast(glm::vec3 origin, glm::vec3 direction, Object *&intersectedObject) {
    Intersection intersect;
    bool found = false;

    for (Object *o : objects) {
        std::optional<Intersection> p = o->findIntersection(origin, direction);
        if (!p.has_value()) {
            continue;
        }
        if (!found || (p.value().t < intersect.t && p.value().t > 1e-5)) {
            intersect = p.value();
            intersectedObject = o;
            found = true;
        }
    }
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
    std::optional<Intersection> intersection = raycast(origin, direction, object);
    if (!intersection.has_value())
        return _clearColor;

    Intersection p = intersection.value();

    glm::vec3 light = RAYTRACE_AMBIENT;
    glm::vec3 normal = p.normal;
    glm::vec3 color = p.color * light + phong(p, glm::vec3(0));

    bool hasReflective = object->mesh->material && object->mesh->material->colorReflective != glm::vec3(0);
    glm::vec3 reflectiveMat = hasReflective ? object->mesh->material->colorReflective : glm::vec3(k_specular);
    if (reflectiveMat != glm::vec3(0) && depth > 1) {
        glm::vec3 rayColor = raytrace(p.point, glm::reflect(direction, normal), depth - 1);
        color = glm::lerp(color, reflectiveMat * rayColor, reflectiveMat);
    }
    bool hasTransmitive = object->mesh->material && object->mesh->material->colorTransmitive != glm::vec3(0);
    glm::vec3 transmitiveMat = hasTransmitive ? object->mesh->material->colorTransmitive : glm::vec3(k_transmit);
    if (transmitiveMat != glm::vec3(0) && depth > 1) {
        float eta = 1.0f;
        glm::vec3 refractedDir = glm::refract(direction, normal, eta);
        color = glm::lerp(color, raytrace(p.point + 0.001f * refractedDir, refractedDir, depth - 1), transmitiveMat);
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

    bool hasReflective = object->mesh->material && object->mesh->material->colorReflective != glm::vec3(0);
    glm::vec3 reflectiveMat = hasReflective ? object->mesh->material->colorReflective : glm::vec3(k_specular);
    if (reflectiveMat != glm::vec3(0) && depth > 1) {
        glm::vec3 randomDirection = glm::reflect(direction, normal + k_roughness * mtr::linearRandVec3(-0.5f, 0.5f));
        glm::vec3 rayColor = pathtrace(p.point, randomDirection, depth - 1);
        color = glm::lerp(color, reflectiveMat * rayColor, reflectiveMat);
    }
    bool hasTransmitive = object->mesh->material && object->mesh->material->colorTransmitive != glm::vec3(0);
    glm::vec3 transmitiveMat = hasTransmitive ? object->mesh->material->colorTransmitive : glm::vec3(k_transmit);
    if (transmitiveMat != glm::vec3(0) && depth > 1) {
        float eta = 1.0f;
        glm::vec3 refractedDir = glm::refract(direction, normal + k_roughness * mtr::linearRandVec3(-0.5f, 0.5f), eta);
        color = glm::lerp(color, raytrace(p.point + 0.001f * refractedDir, refractedDir, depth - 1), transmitiveMat);
    }

    return color;
}

void Renderer::iscrtajRaster() {
    std::cout << "cranje rastera" << std::endl;
    // rasteri[currentRasterIndex]->setFragmentColor(0, rasteri[currentRasterIndex]->height - 1, glm::vec3(69));
    textureShower->loadRaster(rasteri[currentRasterIndex]);
    textureShower->render();
}

void Renderer::spremiRaster() {
    int *buffer = new int[_width * _height * 3];
    glReadPixels(0, 0, _width, _height, GL_BGR, GL_UNSIGNED_BYTE, buffer);
}

void Renderer::EnableVSync() { glfwSwapInterval(1); }

void Renderer::DisableVSync() { glfwSwapInterval(0); }

void Renderer::SwapBuffers() { glfwSwapBuffers(_window); }
