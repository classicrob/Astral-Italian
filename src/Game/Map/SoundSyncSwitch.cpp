#include "Game/RhythmLib/AudRhythmTiming.hpp"
#include "Game/Map/SoundSyncSwitch.hpp"
#include "Game/GameAudio/ActionSoundKeeper.hpp"
#include "Game/Map/StageSwitch.hpp"
#include "Game/Util.hpp"
#include "Game/Util/DemoUtil.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/SoundUtil.hpp"

// PLACEHOLDER(core): unnamed SMG2 callees (no symbol name yet); resolve when their owners name them
// Unnamed SMG2 helpers (sound object and beat/timing checks).
extern "C" bool fn_80056EB0();

SoundSyncSwitch::SoundSyncSwitch(const char* pName)
    : NameObj(pName), mStageSwitchCtrl(NULL), mBeatNum(8), mTimeOffset(0), mInterval(0), mSwitchOnTime(-1), mCounter(0), mSoundType(0), mSound(NULL) {
}

void SoundSyncSwitch::init(const JMapInfoIter& rIter) {
    MR::connectToSceneMapObjMovement(this);
    MR::getJMapInfoArg0NoInit(rIter, &mBeatNum);
    MR::getJMapInfoArg1NoInit(rIter, &mTimeOffset);
    MR::getJMapInfoArg2NoInit(rIter, &mInterval);
    MR::getJMapInfoArg3NoInit(rIter, &mSoundType);
    MR::getJMapInfoArg4NoInit(rIter, &mSwitchOnTime);

    mStageSwitchCtrl = MR::createStageSwitchCtrl(this, rIter);
    MR::isExistStageSwitchB(rIter);

    mSound = new ActionSoundKeeper(4, "SoundSyncSwitch", nullptr);

    mIsSwitchOnAfterCamera = mStageSwitchCtrl->isOnSwitchA();
}

void SoundSyncSwitch::movement() {
    if (mIsCameraWait) {
        if (!MR::isStageStateScenarioOpeningCamera()) {
            mCounter = 0;
            mIsCameraWait = false;

            if (mIsSwitchOnAfterCamera) {
                mStageSwitchCtrl->onSwitchA();
            } else {
                mStageSwitchCtrl->offSwitchA();
            }
        }
    } else {
        if (MR::isStageStateScenarioOpeningCamera()) {
            mIsCameraWait = true;
        }
    }

    mSound->update();

    if (mStageSwitchCtrl->isValidSwitchAppear() && !mStageSwitchCtrl->isOnSwitchAppear()) {
        return;
    }

    if (mStageSwitchCtrl->isValidSwitchB() && mStageSwitchCtrl->isOnSwitchB()) {
        return;
    }

    if (mStageSwitchCtrl->isOnSwitchA()) {
        if (mSwitchOnTime > 0 && mCounter >= mSwitchOnTime) {
            mStageSwitchCtrl->offSwitchA();
        }
    }

    if (mCounter >= mInterval) {
        emit();

        if (AudRhythmWrap::isBeatTiming(mBeatNum, (f32)mTimeOffset)) {
            if (mStageSwitchCtrl->isOnSwitchA()) {
                mStageSwitchCtrl->offSwitchA();
            } else {
                mStageSwitchCtrl->onSwitchA();
            }

            mCounter = 0;
            return;
        }
    }

    mCounter++;
}

void SoundSyncSwitch::emit() {
    if (MR::isDemoActive()) {
        return;
    }

    if (MR::isStageStateScenarioOpeningCamera()) {
        return;
    }

    if (fn_80056EB0()) {
        return;
    }

    if (mSoundType == 2) {
        return;
    }

    if (mSoundType == 1) {
        if (AudRhythmWrap::isBeatTiming(8, 0.07f)) {
            MR::startCSSound("CS_METRO_CLICK2", "SE_SY_CSP_ALT_METRO_CLICK2", -1, 0, 0.7f);
        } else if (AudRhythmWrap::isBeatTiming(8, 5.0f + 0.07f) || AudRhythmWrap::isBeatTiming(8, 6.0f + 0.07f) || AudRhythmWrap::isBeatTiming(8, 7.0f + 0.07f)) {
            MR::startCSSound("CS_METRO_CLICK1", "SE_SY_CSP_ALT_METRO_CLICK1", -1, 0, 0.7f);
        }
        return;
    }

    if (AudRhythmWrap::isBeatTiming(2, 1.0f + 0.07f)) {
        MR::startCSSound("CS_METRO_CLICK2", "SE_SY_CSP_ALT_METRO_CLICK2", -1, 0, 0.7f);
    } else if (AudRhythmWrap::isBeatTiming(2, 0.07f)) {
        MR::startCSSound("CS_METRO_CLICK1", "SE_SY_CSP_ALT_METRO_CLICK1", -1, 0, 0.7f);
    }

    if (AudRhythmWrap::isBeatTiming(8, 0.0f)) {
        mSound->startSound("Change", -1, -1);
    } else if (AudRhythmWrap::isBeatTiming(8, 5.0f) || AudRhythmWrap::isBeatTiming(8, 6.0f) || AudRhythmWrap::isBeatTiming(8, 7.0f)) {
        mSound->startSound("ChangeSign", -1, -1);
    }
}

SoundSyncSwitch::~SoundSyncSwitch() {
}
