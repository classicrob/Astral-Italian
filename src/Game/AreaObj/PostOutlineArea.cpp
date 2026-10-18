#include "Game/AreaObj/PostOutlineArea.hpp"
#include "Game/Util.hpp"

PostOutlineArea::PostOutlineArea(const char* pName) : ImageEffectArea(IMAGE_EFFECT_TYPE_POST_OUTLINE, pName) {
}

void PostOutlineArea::init(const JMapInfoIter& rIter) {
    ImageEffectArea::init(rIter);
    MR::createPostOutline();
}

s32 PostOutlineArea::getIntensity() const {
    if (mObjArg0 < 0) {
        return 0x10;
    }

    if (mObjArg0 > 0xFF) {
        return 0xFF;
    }

    return mObjArg0 & 0xFF;
}

const char* PostOutlineArea::getManagerName() const {
    return "ImageEffectArea";
}
