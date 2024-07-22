#include "renderer/Framebuffer.h"
#include "models/Mesh.h"
#include "renderer/Texture.h"

Framebuffer::Framebuffer() {
    glGenFramebuffers(1, &FBO);
    // Povezivanje teksture s frame bufferom
    // shader = Shader::Load("depthMap");
}

void Framebuffer::use() { glBindFramebuffer(GL_FRAMEBUFFER, FBO); }

void Framebuffer::setDepthTexture(Texture *texture) {
    use();
    depthTexture = texture;
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, texture->id, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::setupDepth() {
    assert(depthTexture != nullptr);

    GLCheckError();
    glViewport(0, 0, depthTexture->width, depthTexture->height);
    GLCheckError();
    // glClear(GL_DEPTH_BUFFER_BIT); // not needed if cleared already
    GLCheckError();
    glCullFace(GL_FRONT);
    GLCheckError();
}

void Framebuffer::cleanDepth() {
    assert(depthTexture != nullptr);

    glCullFace(GL_BACK);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    // glViewport(0, 0, _width, _height);
}

// void Framebuffer::render() {
//
// }
