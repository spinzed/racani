#pragma once

#include "objects/BSpline.h"
#include "objects/PolyLine.h"
#include "renderer/Renderer.h"

#include <string>

class ExampleBSpline {
  public:
    int width = 500, height = 500;
    float moveSensitivity = 3, sprintMultiplier = 5, mouseSensitivity = 0.15f;

    Renderer *renderer;

    Transform *movingObject;
    Transform *movingObject2;

    bool axis = false;

    BSpline *cameraCurve = nullptr;
    BSpline *helix = nullptr;
    PolyLine *tangenta;

    int run(std::string dir);
    void KeyCallback(InputGlobalListenerData event);
    void cursorPositionCallback(WindowCursorEvent event);
};
