#pragma once

#include "Transform.h"

#include "vector"

typedef struct {
    float near;
    float far;
    float angle;
    float left;
    float right;
    float bottom;
    float top;
} CameraConstraints;

class CameraObserver {
  public:
    virtual void onCameraChange() = 0;
    virtual ~CameraObserver() {}
};

class Camera: public Transform {
  private:
    glm::mat4 projectionMatrix;
    glm::mat4 projectionViewMatrix;

    std::vector<CameraObserver *> observers;

    void notify() {
        for (auto o : observers) {
            o->onCameraChange();
        }
    };

  public:
    Camera(int width, int height);
    //~Camera();

    CameraConstraints constraints;

    void setSize(int width, int height);

    glm::mat4 getViewMatrix();
    glm::mat4 getProjectionMatrix();
    glm::mat4 getProjectionViewMatrix();
    void recalculateMatrix();

    void addChangeListener(CameraObserver *observer) {
        observers.push_back(observer);
    };
};