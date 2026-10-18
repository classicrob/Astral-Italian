#include "Game/AreaObj/BgmVolumeFadeArea.hpp"
#include "Game/Util.hpp"

BgmVolumeFadeArea::BgmVolumeFadeArea(const char* pName) : AreaObj(pName) {
    _48 = false;
    _49 = true;
}

void BgmVolumeFadeArea::init(const JMapInfoIter& rIter) {
    AreaObj::init(rIter);
    MR::connectToSceneAreaObj(this);
    _49 = true;
}

void BgmVolumeFadeArea::movement() {
    if (!isValid()) {
        return;
    }

    if (isInVolume(*MR::getPlayerPos())) {
        if (!_48) {
            s32 fadeTime = getFadeTime();
            MR::moveVolumeStageBGM(getVolume(), fadeTime);
            _48 = true;
            _49 = false;
        }
    } else {
        if (!_49) {
            MR::moveVolumeStageBGM(1.0f, getFadeTime());
            _49 = true;
            _48 = false;
        }
    }
}

s32 BgmVolumeFadeArea::getFadeTime() const {
    if (mObjArg0 < 0) {
        return 90;
    }

    return mObjArg0;
}

// NONMATCHING: the negative-arg default is the runtime global lbl_807E2FC0 (sbss2), not a literal.
f32 BgmVolumeFadeArea::getVolume() const {
    f32 volume;
    if (mObjArg1 < 0) {
        volume = 100.0f;
    } else {
        volume = mObjArg1;
    }

    return volume / 100.0f;
}
