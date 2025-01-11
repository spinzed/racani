#include "renderer/Framebuffer.h"

#include "utils/GLDebug.h"

#include <stdexcept>

Framebuffer::Framebuffer() {
    glGenFramebuffers(1, &FBO);
    // Povezivanje teksture s frame bufferom
    // shader = Shader::Load("depthMap");
}

void Framebuffer::use() { glBindFramebuffer(GL_FRAMEBUFFER, FBO); }

void Framebuffer::setDepthTexture(Texture *texture) {
    assert(texture != nullptr);
    use();
    depthTexture = texture;
    if (texture->glType == GL_TEXTURE_CUBE_MAP) {
        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, texture->id, 0);
    } else if (texture->glType == GL_TEXTURE_2D) {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, texture->glType, texture->id, 0);
    } else {
        std::runtime_error("unsupported texture type");
    }
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::setDepthTexture(CubemapArray *array, int index) {
    use();
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, array->ID, 0);
    // glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, texture->ID, 0, index * 6 + faceIdx); // one face
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
