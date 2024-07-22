#include <renderer/Animation.h>
#include <renderer/Animator.h>

#include <vector>

std::vector<Animation *> Animator::_animations;

void Animator::registerAnimation(Animation *anim) {
    _animations.push_back(anim);
    anim->onChange(0, anim->totalDuration);
};

void Animator::passTime(float time) {
    for (auto i = _animations.begin(); i != _animations.end(); ++i) {
        Animation *a = *i;

        a->addDuration(time);
        a->onChange(a->current, a->totalDuration);
        if (a->done()) {
            _animations.erase(i--);
            delete a;
        }
    }
}
