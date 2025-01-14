#pragma once

#include "backends/imgui_impl_glfw.h"
#include <assert.h>
#include <functional>
#include <vector>

#include <GLFW/glfw3.h>
#include <cstring>
#include <stdio.h>

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

enum XboxOneButtons {
    A = 0,
    B = 1,
    X = 2,
    Y = 3,
    UP = 11,
    DOWN = 13,
    LEFT = 14,
    RIGHT = 12,
    LB = 4,
    RB = 5,
    L3 = 9,
    R3 = 10,
    START = 6,
    PAUSE = 7,
    XBOX = 8, // sometimes doesn't work
};

enum XboxOneAnalog {
    LEFT_X = 0,
    LEFT_Y = 1,
    RIGHT_X = 3,
    RIGHT_Y = 4,
    LT = 2,
    RT = 5,
};

class Input {
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

    inline static std::function<void(int, int)> boundsGetter;

    inline static GLFWwindow *window;

    inline static bool cursorCaptured = 0;

    static void captureCursor() { cursorCaptured = 1; }

    // *********
    // * MOUSE *
    // *********

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

    static bool checkKeyEvent(int event, int type) {
        assert(window != nullptr);
        return glfwGetKey(window, event) == type;
    }

    static void firePerFrame(float deltaTime) {
        InputListenerData data = {.deltaTime = deltaTime};

        for (const auto &func : keyPerFrameListeners) {
            func(data);
        }
    }

    // **************
    // * CONTROLLER *
    // **************

    static bool ControllerConnected(int controllerNum = 1) {
        return glfwJoystickPresent(GLFW_JOYSTICK_1 + controllerNum - 1);
    }

    struct Controller {
        int initialized;
        int ButtonCount;
        const unsigned char *Buttons;
        int HatCount;
        const unsigned char *Hats; // these are usually d-pads, also mapped with buttons
        int AxisCount;
        const float *Axes;
    };

    inline static const int MaxControllers = GLFW_JOYSTICK_LAST + 1;

    inline static Controller controllers[MaxControllers] = {{0, 0, nullptr, 0, nullptr, 0, nullptr}};
    inline static Controller oldControllers[MaxControllers] = {{0, 0, nullptr, 0, nullptr, 0, nullptr}};
    inline static bool oldControllersSet = false;
    inline static bool controllersSet = false;

    static void ClearControllerStates() {
        if (!controllersSet) {
            for (int i = 0; i < MaxControllers; i++) {
                controllers[i].Buttons = new unsigned char[controllers[i].ButtonCount];
                controllers[i].Hats = new unsigned char[controllers[i].HatCount];
                controllers[i].Axes = new float[controllers[i].AxisCount];

                oldControllers[i].Buttons = new unsigned char[controllers[i].ButtonCount];
                oldControllers[i].Hats = new unsigned char[controllers[i].HatCount];
                oldControllers[i].Axes = new float[controllers[i].AxisCount];
            }
            controllersSet = true;
            return;
        }

        for (int i = 0; i < MaxControllers; i++) {
            // oldControllers[i] = controllers[i];
            oldControllers[i].initialized = controllers[i].initialized;
            oldControllers[i].ButtonCount = controllers[i].ButtonCount;
            oldControllers[i].HatCount = controllers[i].HatCount;
            oldControllers[i].AxisCount = controllers[i].AxisCount;

            std::memcpy((void *)oldControllers[i].Buttons, controllers[i].Buttons, controllers[i].ButtonCount);
            std::memcpy((void *)oldControllers[i].Hats, controllers[i].Hats, controllers[i].HatCount);
            std::memcpy((void *)oldControllers[i].Axes, controllers[i].Axes, controllers[i].AxisCount * sizeof(float));

            controllers[i].initialized = 0;
        }
        oldControllersSet = true;
    }

    static void FetchControllerState(int controllerNum = 1) {
        if (!ControllerConnected(controllerNum))
            return;

        int jid = GLFW_JOYSTICK_1 + controllerNum - 1;
        const unsigned char *bs = glfwGetJoystickButtons(jid, &c(controllerNum)->ButtonCount);
        const unsigned char *hs = glfwGetJoystickHats(jid, &c(controllerNum)->HatCount);
        const float *ax = glfwGetJoystickAxes(jid, &c(controllerNum)->AxisCount);

        // this is necessary because of the stupid imgui
        std::memcpy((void *)c(controllerNum)->Buttons, bs, c(controllerNum)->ButtonCount);
        std::memcpy((void *)c(controllerNum)->Hats, hs, c(controllerNum)->HatCount);
        std::memcpy((void *)c(controllerNum)->Axes, ax, c(controllerNum)->AxisCount * sizeof(float));

        c(controllerNum)->initialized = true;
    }

    static const char *GetControllerName(int controllerNum = 1) {
        int jid = GLFW_JOYSTICK_1 + controllerNum - 1;
        return glfwJoystickIsGamepad(jid) ? glfwGetGamepadName(jid) : glfwGetJoystickName(jid);
    }

    static void PrintControllerState(int controllerNum = 1) {
        if (!ControllerConnected(controllerNum)) {
            printf("Controller not connected.\n");
            return;
        }
        if (!c(controllerNum)->initialized)
            FetchControllerState(controllerNum);

        if (!c(controllerNum)->Buttons) {
            printf("Failed to get button states.\n");
            return;
        }
        int jid = GLFW_JOYSTICK_1 + controllerNum - 1;

        printf("Joystick name: %s\n", glfwGetJoystickName(jid));
        if (glfwJoystickIsGamepad(jid)) {
            printf("Joystick is also a gamepad with the name: %s\n", glfwGetGamepadName(jid));
        }

        printf("Joystick has %d buttons:\n", c(controllerNum)->ButtonCount);
        for (int i = 0; i < c(controllerNum)->ButtonCount; ++i) {
            printf("Button %d: %s\n", i, c(controllerNum)->Buttons[i] == GLFW_PRESS ? "Pressed" : "Released");
        }
        if (c(controllerNum)->Hats) {
            printf("Joystick has %d hats:\n", c(controllerNum)->HatCount);
            for (int i = 0; i < c(controllerNum)->HatCount; ++i) {
                printf("Hat %d: ", i);

                int state = c(controllerNum)->Hats[i];
                if (state == GLFW_HAT_CENTERED) {
                    printf("Centered");
                } else {
                    if (state & GLFW_HAT_UP)
                        printf("Up ");
                    if (state & GLFW_HAT_RIGHT)
                        printf("Right ");
                    if (state & GLFW_HAT_DOWN)
                        printf("Down ");
                    if (state & GLFW_HAT_LEFT)
                        printf("Left ");
                }

                printf("\n");
            }
        } else {
            printf("No controller hats found.\n");
        }
        if (c(controllerNum)->Axes) {
            printf("Joystick has %d axes:\n", c(controllerNum)->AxisCount);
            for (int i = 0; i < c(controllerNum)->AxisCount; ++i) {
                printf("Axis %d: %f\n", i, c(controllerNum)->Axes[i]);
            }
        } else {
            printf("Failed to get axis states.\n");
        }
    }

    static bool ControllerButtonDown(int button, int controllerNum = 1) {
        if (!c(controllerNum)->initialized)
            FetchControllerState(controllerNum);

        if (!ControllerConnected(controllerNum) || !c(controllerNum)->Buttons ||
            button >= c(controllerNum)->ButtonCount)
            return false;

        return c(controllerNum)->Buttons[button] == GLFW_PRESS;
    }

    static bool ControllerButtonPressed(int button, int controllerNum = 1) {
        if (!c(controllerNum)->initialized)
            FetchControllerState(controllerNum);

        if (!ControllerConnected(controllerNum) || !c(controllerNum)->Buttons ||
            button >= c(controllerNum)->ButtonCount || !c_p(controllerNum)->initialized)
            return false;

        return c(controllerNum)->Buttons[button] == GLFW_PRESS && c_p(controllerNum)->Buttons[button] != GLFW_PRESS;
    }

    static int GetHatState(int hat, int controllerNum = 1) {
        if (!c(controllerNum)->initialized)
            FetchControllerState(controllerNum);

        if (!ControllerConnected(controllerNum) || !c(controllerNum)->Hats || hat >= c(controllerNum)->HatCount)
            return false;

        return c(controllerNum)->Hats[hat];
    }

    static float GetAnalog(int axes, int controllerNum = 1) {
        if (!c(controllerNum)->initialized)
            FetchControllerState(controllerNum);

        if (!ControllerConnected(controllerNum) || !c(controllerNum)->Axes || axes >= c(controllerNum)->AxisCount)
            return false;

        return c(controllerNum)->Axes[axes];
    }

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

    // controller helper functions
    static Controller *c(int controllerNum = 1) { return &controllers[controllerNum - 1]; }
    static Controller *c_p(int controllerNum = 1) { return &oldControllers[controllerNum - 1]; }
};
