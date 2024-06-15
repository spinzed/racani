#pragma once

#include "Curve.h"

class Animation {
  public:
    Curve *curve;
    float current;
    float totalDuration;

    Animation(Curve *c, float duration);
    virtual ~Animation() {};

    void addDuration(float duration) {
        current += duration;
        if (current > totalDuration) {
            current = totalDuration;
        }
    }

    virtual void onChange(float current, float totalDuration) { std::cout << "original Func" << std::endl; };

    bool done() { return totalDuration == current; };

};