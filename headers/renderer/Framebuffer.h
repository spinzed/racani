#pragma once

#include "objects/Object.h"
#include "renderer/Texture.h"
#include "utils/GLDebug.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>

class Framebuffer {
  public:
    Framebuffer();
    void setDepthTexture(Texture *texture);
    void setupDepth();
    void cleanDepth(int width, int height);
    void use();

  private:
    GLuint FBO;
    Texture *depthTexture = nullptr;
};

Framebuffer::Framebuffer() {
    glGenFramebuffers(1, &FBO);
    // Povezivanje teksture s frame bufferom
    // shader = Shader::Load("depthMap");
}

void Framebuffer::use() { glBindFramebuffer(GL_FRAMEBUFFER, FBO); }

void Framebuffer::setDepthTexture(Texture *texture) {
    use();
    depthTexture = texture;
    if (texture->glType == GL_TEXTURE_CUBE_MAP) {
        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, texture->id, 0);
    } else {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, texture->glType, texture->id, 0);
    }
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::setupDepth() {
    assert(depthTexture != nullptr);

    GLCheckError();
    glViewport(0, 0, depthTexture->width, depthTexture->height);
    // glClear(GL_DEPTH_BUFFER_BIT); // not needed if cleared already
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    GLCheckError();
    glCullFace(GL_FRONT);
    GLCheckError();
}

void Framebuffer::cleanDepth(int width, int height) {
    assert(depthTexture != nullptr);

    glCullFace(GL_BACK);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, width, height);
}
