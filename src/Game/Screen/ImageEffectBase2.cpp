#include "Game/Screen/ImageEffectBase.hpp"

ImageEffectBase::ImageEffectBase(const char* name) : NameObj(name), mIsTurnedOn(false), mIsVisible(false), mFadeRate(0.0f) {}

void ImageEffectBase::calcAnim() {
    if (mIsTurnedOn) {
        mIsVisible = true;
        mFadeRate += 0.06666667f;
        if (mFadeRate > 1.0f) mFadeRate = 1.0f;
    } else if (isSomething()) {
        mFadeRate -= 0.06666667f;
        if (mFadeRate < 0.0f) {
            mFadeRate = 0.0f;
            mIsVisible = false;
        }
    }
    calcAnimSub();
}
