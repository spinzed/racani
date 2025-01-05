#pragma once

#include <GLFW/glfw3.h>

#include <vector>
#include <functional>

class UI {
  public:
    inline static std::vector<std::function<void()>> builderFuncs;

    static void Init(GLFWwindow *window);
    static void AddBuilderFunction(std::function<void()> f);
    static void BuildUI();
    static void Render();
    static void Cleanup();
};
