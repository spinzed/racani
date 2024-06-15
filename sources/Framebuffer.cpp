#include "Framebuffer.h"
#include "Texture.h"

Framebuffer::Framebuffer() {
    glGenFramebuffers(1, &FBO);
    // Povezivanje teksture s frame bufferom
    // shader = Shader::load("depthMap");
}

void Framebuffer::use() {
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
}

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

    glViewport(0, 0, depthTexture->width, depthTexture->height);
    glClear(GL_DEPTH_BUFFER_BIT);
    glCullFace(GL_FRONT);
}

void Framebuffer::cleanDepth() {
    assert(depthTexture != nullptr);

    glCullFace(GL_BACK);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    //glViewport(0, 0, _width, _height);
}

// void Framebuffer::render() {
//
// }
