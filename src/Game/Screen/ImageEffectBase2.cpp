#include "Game/Screen/ImageEffectBase.hpp"

ImageEffectBase::ImageEffectBase(const char* name) : NameObj(name), _C(false), _D(false), _10(0.0f) {}

void ImageEffectBase::calcAnim() {
    if (_C) {
        _D = true;
        _10 += 0.06666667f;
        if (_10 > 1.0f) _10 = 1.0f;
    } else if (isSomething()) {
        _10 -= 0.06666667f;
        if (_10 < 0.0f) {
            _10 = 0.0f;
            _D = false;
        }
    }
    calcAnimSub();
}
