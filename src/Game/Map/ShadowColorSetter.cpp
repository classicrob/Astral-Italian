#include "Game/Map/ShadowColorSetter.hpp"
#include "Game/AreaObj/AreaObj.hpp"
#include "Game/AreaObj/ShadowColorArea.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/Util/AreaObjUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"

// NONMATCHING: target stores constant GXColor values as single 32-bit words (lis/addi + stw); byte-wise field stores here
ShadowColorSetter::ShadowColorSetter(const char* pName) : NameObj(pName) {
    mArea = nullptr;
    mCurColor.r = 0x00;
    mCurColor.g = 0xFF;
    mCurColor.b = 0x00;
    mCurColor.a = 0xFF;
    mPrevColor.r = 0x00;
    mPrevColor.g = 0x00;
    mPrevColor.b = 0x00;
    mPrevColor.a = 0xFF;
    mTargetColor.r = 0x00;
    mTargetColor.g = 0x00;
    mTargetColor.b = 0x00;
    mTargetColor.a = 0xFF;
    mBlendT = 0.0f;
    MR::connectToSceneMapObjMovement(this);
}

// NONMATCHING: null-area branch: target stores 0xFF as one word (li+stw), we copy via operator= (bl __as__8_GXColor); regalloc of clamp temps differs
void ShadowColorSetter::movement() {
    AreaObj* area = MR::getAreaObj("ShadowColorArea", *MR::getPlayerPos());
    if (mArea != area) {
        if (area != nullptr) {
            u8 b = MR::clamp(area->mObjArg2, 0, 255);
            u8 g = MR::clamp(area->mObjArg1, 0, 255);
            u8 r = MR::clamp(area->mObjArg0, 0, 255);
            mTargetColor.r = r;
            mTargetColor.g = g;
            mTargetColor.b = b;
            mTargetColor.a = 0xFF;
        } else {
            GXColor black = {0x00, 0x00, 0x00, 0xFF};
            mTargetColor = black;
        }
        mPrevColor = mCurColor;
        mBlendT = 0.0f;
        mArea = area;
    }

    mBlendT += 0.02f;
    if (mBlendT > 1.0f) {
        mBlendT = 1.0f;
    }

    MR::blendColor(&mCurColor, mPrevColor, mTargetColor, mBlendT);
}

namespace MR {
    void createShadowColorSetter() {
        MR::createSceneObj(SceneObj_ShadowColorSetter);
    }

    GXColor calcShadowColor(u8 alpha) {
        GXColor color = {0x00, 0x00, 0x00, 0xFF};
        if (MR::isExistSceneObj(SceneObj_ShadowColorSetter)) {
            ShadowColorSetter* pSetter = MR::getSceneObj< ShadowColorSetter >(SceneObj_ShadowColorSetter);
            GXColor cur = pSetter->mCurColor;
            color = cur;
        }
        color.a = alpha;
        return color;
    }
};  // namespace MR
