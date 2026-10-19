#include "Game/AreaObj/BgmVolumeFadeArea.hpp"
#include "Game/Util.hpp"

namespace {
    const volatile f32 sDefaultVolumePercent = 0.0f;
}


BgmVolumeFadeArea::BgmVolumeFadeArea(const char* pName) : AreaObj(pName) {
    mIsInsideApplied = false;
    mIsOutsideApplied = true;
}

void BgmVolumeFadeArea::init(const JMapInfoIter& rIter) {
    AreaObj::init(rIter);
    MR::connectToSceneAreaObj(this);
    mIsOutsideApplied = true;
}

void BgmVolumeFadeArea::movement() {
    if (!isValid()) {
        return;
    }

    if (isInVolume(*MR::getPlayerPos())) {
        if (!mIsInsideApplied) {
            s32 fadeTime = getFadeTime();
            MR::moveVolumeStageBGM(getVolume(), fadeTime);
            mIsInsideApplied = true;
            mIsOutsideApplied = false;
        }
    } else {
        if (!mIsOutsideApplied) {
            MR::moveVolumeStageBGM(1.0f, getFadeTime());
            mIsOutsideApplied = true;
            mIsInsideApplied = false;
        }
    }
}

s32 BgmVolumeFadeArea::getFadeTime() const {
    if (mObjArg0 < 0) {
        return 90;
    }

    return mObjArg0;
}

f32 BgmVolumeFadeArea::getVolume() const {
    if (mObjArg1 < 0) {
        return sDefaultVolumePercent / 100.0f;
    }
    return mObjArg1 / 100.0f;
}
