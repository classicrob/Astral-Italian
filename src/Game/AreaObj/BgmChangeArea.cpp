#include "Game/GameAudio/AudStageBgmWrap.hpp"
#include "Game/AreaObj/BgmChangeArea.hpp"
#include "Game/AudioLib/AudWrap.hpp"
#include "Game/AudioLib/AudBgm.hpp"
#include "Game/AudioLib/AudBgmMgr.hpp"
#include "Game/Util.hpp"
#include "Game/Util/BgmUtil.hpp"
#include "Game/Util/DemoUtil.hpp"
#include "Game/Util/EventUtil.hpp"
#include "Game/Util/SceneUtil.hpp"
#include "Game/Util/SoundUtil.hpp"

// Stage BGM lookups by scenario/stage name (target symbols are unnamed fn_ helpers).

#pragma push
#pragma dont_inline on
namespace MR {
    bool isStageTwisterTowerGalaxy() {
        return isEqualStageName("TwisterTowerGalaxy");
    }
};  // namespace MR
#pragma pop

BgmChangeArea::BgmChangeArea(const char* pName) : AreaObj(pName) {
    mIsBgmApplied = false;
}

void BgmChangeArea::init(const JMapInfoIter& rIter) {
    AreaObj::init(rIter);
    MR::isStageTwisterTowerGalaxy();

    switch (mObjArg0) {
    case 0: {
        s32 arg1 = mObjArg1;
        if (arg1 >= 0) {
            s32 scenario = MR::getCurrentScenarioNo();
            AudStageBgmWrap::getStageBgmIDAtIndex(MR::getCurrentStageName(), scenario, arg1);
        }
        s32 arg2 = mObjArg2;
        if (arg2 >= 0) {
            AudStageBgmWrap::getStageBgmVolumeIDAtIndex(MR::getCurrentStageName(), arg2);
        }
        break;
    }
    case 1: {
        s32 arg2 = mObjArg2;
        if (arg2 >= 0) {
            s32 scenario = MR::getCurrentScenarioNo();
            AudStageBgmWrap::getStageBgmIDAtIndex(MR::getCurrentStageName(), scenario, arg2);
        }
        break;
    }
    case 2: {
        s32 arg1 = mObjArg1;
        if (arg1 >= 0) {
            AudStageBgmWrap::getStageBgmVolumeIDAtIndex(MR::getCurrentStageName(), arg1);
        }
        break;
    }
    }

    MR::connectToSceneAreaObj(this);
}

// NONMATCHING: argument and BGM-id locals use different registers; conditions and calls agree.
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
    if (MR::isPlayingStageBgmID(0x02000000)) {
        invalidate();
        return;
    }
    if (MR::isPlayingStageBgmID(0x02000001)) {
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
    if (isInVolume(*MR::getPlayerPos())) {
        if (mIsBgmApplied) {
            return;
        }
        s32 arg1 = mObjArg1;
        s32 arg2 = mObjArg2;
        switch (mObjArg0) {
        case 0:
            if (arg1 < 0) {
                MR::startCurrentStageBGM(true);
            } else {
                s32 scenario = MR::getCurrentScenarioNo();
                u32 bgmId = AudStageBgmWrap::getStageBgmIDAtIndex(MR::getCurrentStageName(), scenario, arg1);
                if (bgmId != 0xFFFFFFFF) {
                    if (AudWrap::getBgmMgr()->mCurrentBGM[AudBgmMgr::BgmType_Stage] == bgmId && MR::isPlayingStageBgm()) {
                        break;
                    }
                    AudWrap::startStageBgm(bgmId, false, -1);
                }
                if (arg2 >= 0) {
                    s32 muteState = AudStageBgmWrap::getStageBgmVolumeIDAtIndex(MR::getCurrentStageName(), arg2);
                    if (muteState >= 0) {
                        AudBgm* bgm = AudWrap::getStageBgm();
                        if (bgm) {
                            bgm->changeTrackMuteState(muteState, 0);
                        }
                    }
                }
            }
            break;
        case 1:
            if (mObjArg3 != 1 && MR::isGalaxyCometEvent()) {
                break;
            }
            if (arg1 < 0) {
                arg1 = 90;
            }
            if (arg2 >= 0) {
                s32 scenario = MR::getCurrentScenarioNo();
                u32 bgmId = AudStageBgmWrap::getStageBgmIDAtIndex(MR::getCurrentStageName(), scenario, arg2);
                if (bgmId != 0xFFFFFFFF) {
                    if (AudWrap::getBgmMgr()->mCurrentBGM[AudBgmMgr::BgmType_Stage] == bgmId && MR::isPlayingStageBgm()) {
                        break;
                    }
                    AudWrap::setNextIdStageBgm(bgmId);
                }
            }
            MR::stopStageBGM(arg1);
            break;
        case 2:
            if (arg1 < 0) {
                AudBgm* bgm = AudWrap::getStageBgm();
                if (bgm) {
                    bgm->changeTrackMuteState(0, getFadeTime());
                }
            } else {
                s32 muteState = AudStageBgmWrap::getStageBgmVolumeIDAtIndex(MR::getCurrentStageName(), arg1);
                if (muteState >= 0) {
                    AudBgm* bgm = AudWrap::getStageBgm();
                    if (bgm) {
                        bgm->changeTrackMuteState(muteState, getFadeTime());
                    }
                }
            }
            break;
        }
        mIsBgmApplied = true;
    } else {
        if (mObjArg4 == 0 && mIsBgmApplied) {
            AudBgm* bgm = AudWrap::getStageBgm();
            if (bgm) {
                bgm->changeTrackMuteState(0, getFadeTime());
            }
        }
        mIsBgmApplied = false;
    }
}

s32 BgmChangeArea::getFadeTime() const {
    if (mObjArg2 < 0) {
        return 30;
    }

    return mObjArg2;
}
