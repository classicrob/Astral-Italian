#include "Game/Map/ShadowColorSetter.hpp"
#include "Game/AreaObj/AreaObj.hpp"
#include "Game/AreaObj/ShadowColorArea.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/Util/AreaObjUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include <JSystem/JUtility/TColor.hpp>

ShadowColorSetter::ShadowColorSetter(const char* pName) : NameObj(pName), mArea(nullptr), mCurColor(JUtility::TColor(0x00FF00FF)), mPrevColor(JUtility::TColor(0x000000FF)), mTargetColor(JUtility::TColor(0x000000FF)) {
    mBlendT = 0.0f;
    MR::connectToSceneMapObjMovement(this);
}

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
            GXColor black = JUtility::TColor(0x000000FF);
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
        GXColor color = JUtility::TColor(0x000000FF);
        if (MR::isExistSceneObj(SceneObj_ShadowColorSetter)) {
            ShadowColorSetter* pSetter = MR::getSceneObj< ShadowColorSetter >(SceneObj_ShadowColorSetter);
            GXColor cur = pSetter->mCurColor;
            color = cur;
        }
        color.a = alpha;
        return color;
    }
};  // namespace MR
