#pragma once

#include <GLFW/glfw3.h>
#include <glad/glad.h>

#define DEBUG_PRINT_OPENGL_ERRORS 1

#define GLCheckError() (glCheckError_(__FILE__, __LINE__))
#define GLCheckFramebuffer() (glCheckFramebuffer_(__FILE__, __LINE__))

void glCheckError_(const char *file, int line);

void glCheckFramebuffer_(const char *file, int line);

void GLAPIENTRY debugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
                              const GLchar *message, const void *userParam);
