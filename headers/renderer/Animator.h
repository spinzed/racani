#pragma once

#include <renderer/Animation.h>

#include <vector>

class Animator {
  public:
    static void registerAnimation(Animation *anim);
    static void passTime(float time);

  private:
    Animator();

    static std::vector<Animation *> _animations;
};