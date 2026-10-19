#include "Game/AreaObj/PostFogArea.hpp"
#include "Game/Util.hpp"

PostFogArea::PostFogArea(const char* pName) : ImageEffectArea(IMAGE_EFFECT_TYPE_POST_FOG, pName) {
}

void PostFogArea::init(const JMapInfoIter& rIter) {
    ImageEffectArea::init(rIter);
    MR::createPostFog();
}

s32 PostFogArea::getMinRange() const {
    if (mObjArg0 < 0) {
        return 0;
    }

    if (mObjArg0 > 0x2710) {
        return 0x2710;
    }

    return static_cast< u16 >(mObjArg0);
}

s32 PostFogArea::getMaxRange() const {
    if (mObjArg1 < 0) {
        return 0x2710;
    }

    if (mObjArg1 > 0x2710) {
        return 0x2710;
    }

    return static_cast< u16 >(mObjArg1);
}

u8 PostFogArea::getAlpha() const {
    if (mObjArg2 < 0) {
        return 0xFF;
    }

    if (mObjArg2 > 0xFF) {
        return 0xFF;
    }

    return static_cast< u8 >(mObjArg2);
}

u8 PostFogArea::getRed() const {
    if (mObjArg3 < 0) {
        return 0xFF;
    }

    if (mObjArg3 > 0xFF) {
        return 0xFF;
    }

    return static_cast< u8 >(mObjArg3);
}

u8 PostFogArea::getGreen() const {
    if (mObjArg4 < 0) {
        return 0xFF;
    }

    if (mObjArg4 > 0xFF) {
        return 0xFF;
    }

    return static_cast< u8 >(mObjArg4);
}

u8 PostFogArea::getBlue() const {
    if (mObjArg5 < 0) {
        return 0xFF;
    }

    if (mObjArg5 > 0xFF) {
        return 0xFF;
    }

    return static_cast< u8 >(mObjArg5);
}

s32 PostFogArea::getGradType() const {
    return mObjArg6;
}

const char* PostFogArea::getManagerName() const {
    return "ImageEffectArea";
}
