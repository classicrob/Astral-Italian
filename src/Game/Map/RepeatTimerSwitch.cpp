#include "Game/Map/RepeatTimerSwitch.hpp"
#include "Game/GameAudio/ActionSoundKeeper.hpp"
#include "Game/Map/StageSwitch.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/ObjUtil.hpp"

// Object name, then the sound event names at offsets 0x18 (TimeOut), 0x20 (Timer1), 0x28 (Timer2).
char sRepeatTimerSwitchNames[0x30] = "RepeatTimerSwitch\0\0\0\0\0\0TimeOut\0\0Timer1\0\0Timer2";

RepeatTimerSwitch::~RepeatTimerSwitch() {
}

RepeatTimerSwitch::RepeatTimerSwitch(const char* pName)
    : NameObj(pName), mStageSwitchCtrl(NULL), mTimeLimit(60), mTimeOffset(-1), mTimer(0), mSound(NULL) {
}

void RepeatTimerSwitch::init(const JMapInfoIter& rIter) {
    MR::connectToSceneMapObjMovement(this);
    MR::getJMapInfoArg0NoInit(rIter, &mTimeLimit);
    MR::getJMapInfoArg1NoInit(rIter, &mTimeOffset);
    mStageSwitchCtrl = MR::createStageSwitchCtrl(this, rIter);
    mSound = new ActionSoundKeeper(4, sRepeatTimerSwitchNames, NULL);
}

void RepeatTimerSwitch::movement() {
    const char* pNames = sRepeatTimerSwitchNames;
    if (mStageSwitchCtrl->isValidSwitchAppear() && !mStageSwitchCtrl->isOnSwitchAppear()) {
        return;
    }

    mSound->update();

    s32 limit = mTimeLimit;
    s32 offset = mTimeOffset;
    if (offset < 0) {
        offset = limit;
    }

    if (mTimer < limit) {
        mStageSwitchCtrl->onSwitchA();
    } else {
        mStageSwitchCtrl->offSwitchA();
    }

    s32 period = limit + offset;
    bool wrapped = false;
    if (++mTimer >= period) {
        mTimer = 0;
        wrapped = true;
    }

    if (mTimer == limit) {
        wrapped = true;
    }

    if (wrapped) {
        mSound->startSound(pNames + 0x18, -1, -1);
        return;
    }

    s32 divisor = period / (period / 60);
    s32 rem = mTimer % divisor;
    if (rem == 0) {
        mSound->startSound(pNames + 0x20, -1, -1);
    } else if (rem == 30) {
        mSound->startSound(pNames + 0x28, -1, -1);
    }
}
