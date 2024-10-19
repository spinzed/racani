#include "renderer/Renderer.h"
#include "models/Mesh.h"
#include "models/Raster.h"
#include "objects/FullscreenTexture.h"
#include "renderer/Framebuffer.h"
#include "renderer/Shader.h"
#include "renderer/Texture.h"
#include "utils/GLDebug.h"
#include "utils/ThreadPool.h"
#include "utils/Timer.h"
#include "utils/mtr.h"

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

std::vector<Raster<float> *> rasteri;
int currentRasterIndex = 0;

#define LIGHT_MAX_RANGE 20

#define SKYBOX_COLOR glm::vec3(0.3, 0.4, 1)

#define RENDER_SHADOWMAPS 0

Timer t = Timer::start();

Texture *rasterTexture = nullptr;
FullscreenTexture *textureShower;
Framebuffer *depthFramebuffer;
Shader *rt;

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

PointLight light(glm::vec3(-3, 3, 2), glm::vec3(1, 1, 1), glm::vec3(1, 1, 1));

void Renderer::rasterize() {
    //glm::vec3 lightPos(-3, 3, 2);

    //glm::mat4 lightProjection, lightView;
    //glm::mat4 lightSpaceMatrix;
    //float near_plane = 0.1f, far_plane = 50.0f;
    //float size = 2.0f;
    //lightProjection = glm::ortho(-size, size, -size, size, near_plane, far_plane);
    //lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
    //lightSpaceMatrix = lightProjection * lightView;




    unsigned int depthCubemap;
    glGenTextures(1, &depthCubemap);

    const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
    glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
    for (unsigned int i = 0; i < 6; ++i)
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, 
                     SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL); 

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);  

    //glBindFramebuffer(GL_FRAMEBUFFER, depthFramebuffer->);
    depthFramebuffer->use();
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubemap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);  

    // 1. first render to depth cubemap
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    depthFramebuffer->use();
    glClear(GL_DEPTH_BUFFER_BIT);
    for (Object *o : objects) {
        o->shader->setUniform(SHADER_MMATRIX, 1, o->getModelMatrix());
        GLint lp = glGetUniformLocation(lightMapShader->ID, "lightPos");
        glUniform3fv(lp, 1, glm::value_ptr(light.position));
        o->render(lightMapShader);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // 2. then render scene as normal with shadow mapping (using depth cubemap)
    glViewport(0, 0, _width, _height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);

    for (Object *o : objects) {
        // glClear(GL_DEPTH_BUFFER_BIT);
        //  UpdateShader(o, lightProjection, lightView);
        o->shader->setUniform(SHADER_MMATRIX, 1, o->getModelMatrix());
        GLint lp = glGetUniformLocation(lightMapShader->ID, "lightPos");
        glUniform3fv(lp, 1, glm::value_ptr(light.position));
        o->render(lightMapShader);
    }
    depthFramebuffer->cleanDepth();
    glViewport(0, 0, _width, _height);

    if (skybox) {
        UpdateShader(skybox, camera->getProjectionMatrix(), camera->getViewMatrix());
        skybox->render();
    }
    for (Object *o : objects) {
        UpdateShader(o, camera->getProjectionMatrix(), camera->getViewMatrix());
        //if (o->shader != nullptr) { // TODO: remove this ugly stuff
        //    glUniformMatrix4fv(glGetUniformLocation(o->shader->ID, "lightSpaceMatrix"), 1, GL_FALSE,
        //                       glm::value_ptr(lightSpaceMatrix));
        //}
        o->render();
    }
    return;



    depthFramebuffer->setDepthTexture(&light.cb); // enough to run only once
    lightMapShader->use();

    GLint shadowMatricesLocation = glGetUniformLocation(lightMapShader->ID, "shadowMatrices");
    // Set the uniform using std::vector<glm::mat4>
    glUniformMatrix4fv(shadowMatricesLocation, light.transforms.size(), GL_FALSE, glm::value_ptr(light.transforms[0]));

    glEnable(GL_DEPTH_TEST);

    GLCheckError();
    depthFramebuffer->use();
    depthFramebuffer->setupDepth();
    GLCheckError();

    //int a = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    //std::cout << GL_FRAMEBUFFER_COMPLETE << " " << (int)(a == GL_FRAMEBUFFER_COMPLETE) << std::endl;

    GLint depthAttachment;
    glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,
                                          &depthAttachment);

    glViewport(0, 0, light.cb.width, light.cb.height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    lightMapShader->use();
    // ... send uniforms to shader (including light's far_plane value)
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, light.cb.id);

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);

    if (depthAttachment == GL_NONE) {
        // Handle the error, depth buffer not attached correctly
        std::cout << "error" << std::endl;
    }

    for (Object *o : objects) {
        // glClear(GL_DEPTH_BUFFER_BIT);
        //  UpdateShader(o, lightProjection, lightView);
        o->shader->setUniform(SHADER_MMATRIX, 1, o->getModelMatrix());
        GLint lp = glGetUniformLocation(lightMapShader->ID, "lightPos");
        glUniform3fv(lp, 1, glm::value_ptr(light.position));
        o->render(lightMapShader);
    }
    depthFramebuffer->cleanDepth();
    glViewport(0, 0, _width, _height);

    if (skybox) {
        UpdateShader(skybox, camera->getProjectionMatrix(), camera->getViewMatrix());
        skybox->render();
    }
    for (Object *o : objects) {
        UpdateShader(o, camera->getProjectionMatrix(), camera->getViewMatrix());
        //if (o->shader != nullptr) { // TODO: remove this ugly stuff
        //    glUniformMatrix4fv(glGetUniformLocation(o->shader->ID, "lightSpaceMatrix"), 1, GL_FALSE,
        //                       glm::value_ptr(lightSpaceMatrix));
        //}
        o->render();
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
