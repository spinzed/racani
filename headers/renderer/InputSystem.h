#pragma once

#include "backends/imgui_impl_glfw.h"
#include <assert.h>
#include <functional>
#include <vector>

#include <GLFW/glfw3.h>

struct InputListenerData {
    // int key;  // GLFW_KEY
    // int type; // GLFW_TYPE
    float deltaTime;
};

struct InputGlobalListenerData {
    int key;
    int scancode;
    int action;
    int mods;
};

struct MouseClickOptions {
    int button;
    int action;
    int mods;
};

class InputSystem {
  public:
    inline static std::vector<std::function<void(InputGlobalListenerData)>> keyEventListeners;
    inline static std::vector<std::function<void(InputListenerData)>> keyPerFrameListeners;

    inline static void init(GLFWwindow *w) {
        window = w;
        if (glfwRawMouseMotionSupported())
            glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
        glfwSetMouseButtonCallback(window, mouseCallbackWrapper);
        glfwSetKeyCallback(window, globalKeyCallbackWrapper);
    }

    inline static GLFWwindow *window;

    inline static bool cursorCaptured = 0;

    static void captureCursor() { cursorCaptured = 1; }

    static void addMouseListener(std::function<void(MouseClickOptions)> l) { mouseCallbacks.emplace_back(l); }

    // TODO: refactor this

    // key event that happen once
    static void addKeyEventListener(std::function<void(InputGlobalListenerData)> l) { // ??
        keyEventListeners.emplace_back(l);
    }

    // fires per frame
    static void addPerFrameListener(std::function<void(InputListenerData)> l) { // runs every frame before draws
        keyPerFrameListeners.emplace_back(l);
    }

    // will automatically fire key event listeners
    static void PollEvents() {
        // here would usually be the PollEvents method of the underlying input system subsystem,
        // but in this case it is ommited since it is already called by the windowing subsystem
        // and it polls both windowing and input events
    }

    static void firePerFrame(float deltaTime) {
        InputListenerData data = {.deltaTime = deltaTime};

        for (const auto &func : keyPerFrameListeners) {
            func(data);
        }
    }

    static bool checkKeyEvent(int event, int type) {
        assert(window != nullptr);
        return glfwGetKey(window, event) == type;
    }

    inline static std::function<void(int, int)> boundsGetter;

  private:
    static void globalKeyCallbackWrapper(GLFWwindow *window, int key, int scancode, int action, int mods) {
        (void)window;
        for (const auto &callback : keyEventListeners) {
            InputGlobalListenerData data = {.key = key, .scancode = scancode, .action = action, .mods = mods};
            callback(data);
        }
    }

    inline static std::vector<std::function<void(MouseClickOptions)>> mouseCallbacks;

    static void mouseCallbackWrapper(GLFWwindow *window, int button, int action, int mods) {
        (void)window;

        // also forward the click event to imgui as this isn't done automatically if glfw mouse
        // callback is set. Forwarding the events can also be done by unregistering the listener
        // when the UI is active, might consider if this solution proves to be problematic
        ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);

        int width = 0, height = 0;
        if (!boundsGetter)
            return;

        boundsGetter(width, height);
        for (const auto &callback : mouseCallbacks) {
            MouseClickOptions options = {.button = button, .action = action, .mods = mods};
            callback(options);
        }
    }
};
