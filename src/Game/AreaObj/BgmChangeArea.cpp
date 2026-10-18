#include "Game/AreaObj/BgmChangeArea.hpp"
#include "Game/Util.hpp"
#include "Game/Util/BgmUtil.hpp"
#include "Game/Util/DemoUtil.hpp"
#include "Game/Util/EventUtil.hpp"
#include "Game/Util/SceneUtil.hpp"
#include "Game/Util/SoundUtil.hpp"

// Stage BGM lookups by scenario/stage name (target symbols are unnamed fn_ helpers).
extern "C" s32 fn_8008A150(const char*, s32, s32);
extern "C" s32 fn_8008A1A0(const char*, s32);

namespace MR {
    bool isStageTwisterTowerGalaxy();
    bool isStageStateScenarioOpeningCamera();
};  // namespace MR

BgmChangeArea::BgmChangeArea(const char* pName) : AreaObj(pName) {
    _48 = false;
}

void BgmChangeArea::init(const JMapInfoIter& rIter) {
    AreaObj::init(rIter);
    MR::isStageTwisterTowerGalaxy();

    switch (mObjArg0) {
    case 0:
        if (mObjArg1 >= 0) {
            s32 scenario = MR::getCurrentScenarioNo();
            fn_8008A150(MR::getCurrentStageName(), scenario, mObjArg1);
        }
        if (mObjArg2 >= 0) {
            fn_8008A1A0(MR::getCurrentStageName(), mObjArg2);
        }
        break;
    case 1:
        if (mObjArg2 >= 0) {
            s32 scenario = MR::getCurrentScenarioNo();
            fn_8008A150(MR::getCurrentStageName(), scenario, mObjArg2);
        }
        break;
    case 2:
        if (mObjArg1 >= 0) {
            fn_8008A1A0(MR::getCurrentStageName(), mObjArg1);
        }
        break;
    }

    MR::connectToSceneAreaObj(this);
}

// NONMATCHING: movement() is a behavioural port; the target's BGM id/fade paths
// (AudWrap vtable calls and fn_8008A150/fn_8008A1A0 result checks) are not reproduced exactly.
void BgmChangeArea::movement() {
    if (!isValid()) {
        return;
    }

    if (MR::isCubeBgmChangeInvalid()) {
        return;
    }

    if (MR::isPowerStarGetDemoActive()) {
        invalidate();
        return;
    }

    if (MR::isPlayingStageBgmID(0x200) || MR::isPlayingStageBgmID(0x201)) {
        invalidate();
        return;
    }

    if (MR::isPlayerDead()) {
        invalidate();
        return;
    }

    if (MR::isStageTwisterTowerGalaxy() && isValidSwitchB() && isOnSwitchB()) {
        invalidate();
        return;
    }

    if (MR::isStageStateScenarioOpeningCamera()) {
        return;
    }

    if (!isInVolume(*MR::getPlayerPos())) {
        if (mObjArg4 == 0 && _48) {
            MR::startCurrentStageBGM(true);
        }
        _48 = false;
        return;
    }

    if (_48) {
        return;
    }

    switch (mObjArg0) {
    case 0:
        if (mObjArg1 < 0) {
            MR::startCurrentStageBGM(true);
        } else {
            s32 scenario = MR::getCurrentScenarioNo();
            fn_8008A150(MR::getCurrentStageName(), scenario, mObjArg1);
        }
        break;
    case 1:
        if (mObjArg3 != 1 && MR::isGalaxyCometEvent()) {
            break;
        }
        if (mObjArg2 >= 0) {
            s32 scenario = MR::getCurrentScenarioNo();
            fn_8008A150(MR::getCurrentStageName(), scenario, mObjArg2);
        } else {
            MR::stopStageBGM(getFadeTime());
        }
        break;
    case 2:
        if (mObjArg1 >= 0) {
            fn_8008A1A0(MR::getCurrentStageName(), mObjArg1);
        }
        break;
    }

    _48 = true;
}

s32 BgmChangeArea::getFadeTime() const {
    if (mObjArg2 < 0) {
        return 30;
    }

    return mObjArg2;
}
