#include "Game/AreaObj/DepthOfFieldArea.hpp"
#include "Game/Util.hpp"
#include "Game/Util/ScreenUtil.hpp"

DepthOfFieldArea::DepthOfFieldArea(const char* pName) : ImageEffectArea(IMAGE_EFFECT_TYPE_DOF, pName), mIsPlayerSync() {
}

void DepthOfFieldArea::init(const JMapInfoIter& rIter) {
    ImageEffectArea::init(rIter);
    MR::createDepthOfFieldBlur();
    mIsPlayerSync = mObjArg0 > 0;
}

f32 DepthOfFieldArea::getIntensity() const {
    if (mObjArg1 > -1) {
        return mObjArg1 / 255.0f;
    }

    return 1.0f;
}

s32 DepthOfFieldArea::getBlurMaxDist() const {
    return mObjArg2;
}

s32 DepthOfFieldArea::getBlurMinDist() const {
    return mObjArg3;
}

s32 DepthOfFieldArea::getArg4() const {
    return mObjArg4;
}
