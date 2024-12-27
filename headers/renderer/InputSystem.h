#pragma once

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

// TODO: make it fully static
class InputSystem {
  public:
    std::vector<std::function<void(InputListenerData)>> keyEventListeners;
    std::vector<std::function<void(InputListenerData)>> keyPerFrameListeners;

    inline static void init(GLFWwindow *w) {
        window = w;
        if (glfwRawMouseMotionSupported())
            glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
        glfwSetMouseButtonCallback(window, mouseCallbackWrapper);
    }

    inline static GLFWwindow *window;

    inline static std::function<void(InputGlobalListenerData)> globalKeyCallback;

    inline static void hideCursor() { glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN); }

    inline static bool cursorCaptured = 0;

    inline static void captureCursor() { cursorCaptured = 1; }

    static void addMouseListener(std::function<void(MouseClickOptions)> l) { mouseCallbacks.emplace_back(l); }

    // TODO: refactor this
    void registerGlobalListener(std::function<void(InputGlobalListenerData)> l) { // global glfw listener
        globalKeyCallback = l;
        // glfwSetWindowUserPointer(window, this);
        glfwSetKeyCallback(window, globalKeyCallbackWrapper);
    }
    void registerKeyEventListener(std::function<void(InputListenerData)> l) { // ??
        keyEventListeners.emplace_back(l);
    }
    void registerPerFrameListener(std::function<void(InputListenerData)> l) { // runs every frame before draws
        keyPerFrameListeners.emplace_back(l);
    }

    void fireKeyEvent(float deltaTime) {
        InputListenerData data = {.deltaTime = deltaTime};

        for (const auto &func : keyEventListeners) {
            func(data);
        }
    }
    void firePerFrame(float deltaTime) {
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
        if (globalKeyCallback) {
            InputGlobalListenerData data = {.key = key, .scancode = scancode, .action = action, .mods = mods};
            globalKeyCallback(data);
        }
    }

    inline static std::vector<std::function<void(MouseClickOptions)>> mouseCallbacks;

    static void mouseCallbackWrapper(GLFWwindow *window, int button, int action, int mods) {
        (void)window;
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
