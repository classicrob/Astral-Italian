#include "Game/Map/RepeatTimerSwitch.hpp"
#include "Game/GameAudio/ActionSoundKeeper.hpp"
#include "Game/Map/StageSwitch.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/ObjUtil.hpp"

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
    mSound = new ActionSoundKeeper(4, "RepeartTimerSwitch", NULL);
}

void RepeatTimerSwitch::movement() {
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
        mSound->startSound("TimeOut", -1, -1);
        return;
    }

    s32 divisor = period / (period / 60);
    s32 rem = mTimer % divisor;
    if (rem == 0) {
        mSound->startSound("Timer1", -1, -1);
    } else if (rem == 30) {
        mSound->startSound("Timer2", -1, -1);
    }
}
