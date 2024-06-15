#include "Animation.h"
#include "Curve.h"

Animation::Animation(Curve *c, float duration) {
    curve = c;
    totalDuration = duration;
    current = 0;
}