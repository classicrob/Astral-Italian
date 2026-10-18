#include "Game/Screen/DestroyCounter.hpp"
#include "Game/Screen/CounterLayoutAppearer.hpp"
#include "Game/Screen/CountUpPaneRumbler.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include "Game/Util/ActionSoundUtil.hpp"

namespace NrvDestroyCounter {
    NEW_NERVE(DestroyCounterNrvAppear, DestroyCounter, Appear);
    NEW_NERVE(DestroyCounterNrvWait, DestroyCounter, Wait);
    NEW_NERVE(DestroyCounterNrvDisappear, DestroyCounter, Disappear);
}

DestroyCounter::DestroyCounter(s32 maximum) : LayoutActor("全滅カウンター", true), mMaximum(maximum), mDisplayedCount(0), mCount(0), mCountDelay(0), mAppearer(0), mRumbler(0), mIsActive(false) {}

void DestroyCounter::init(const JMapInfoIter&) {
    initLayoutManager("DestroyCounter", 2);
    MR::createAndAddPaneCtrl(this, "Counter", 1);
    MR::createAndAddPaneCtrl(this, "Max", 1);
    initEffectKeeper(0, 0, 0);
    mAppearer = new CounterLayoutAppearer(this, TVec2f(-50.0f, 0.0f));
    mRumbler = new CountUpPaneRumbler(this, "Counter", -1.0f);
    initActionSound(3, "DestroyCounter", 0);
    MR::connectToSceneLayout(this);
    initNerve(&NrvDestroyCounter::DestroyCounterNrvAppear::sInstance);
    if (mMaximum == 20) MR::startPaneAnim(this, "Max", "Max20", 0);
    else MR::startPaneAnim(this, "Max", "Max30", 0);
}

void DestroyCounter::appear() {
    if (mIsActive && MR::isDead(this)) {
        mAppearer->reset();
        mRumbler->reset();
        mCountDelay = 0;
        MR::setTextBoxFormatRecursive(this, "Counter", L"%02d", mDisplayedCount);
        setNerve(&NrvDestroyCounter::DestroyCounterNrvAppear::sInstance);
        LayoutActor::appear();
        MR::requestMovementOn(this);
    }
}

void DestroyCounter::activate() {
    mIsActive = true;
    appear();
}

void DestroyCounter::control() {
    updateCount();
    mAppearer->update();
    mRumbler->update();
}

void DestroyCounter::updateCount() {
    if (mCountDelay > 0) --mCountDelay;
    else if (mDisplayedCount != mCount) {
        if (isNerve(&NrvDestroyCounter::DestroyCounterNrvWait::sInstance)) {
            mDisplayedCount = mCount;
            mCountDelay = 3;
            MR::startAnim(this, "Flash", 0);
            MR::startActionSound(this, "Change");
            if (mDisplayedCount != mMaximum) mRumbler->start();
        }
        if (!isNerve(&NrvDestroyCounter::DestroyCounterNrvAppear::sInstance)) {
            if (!isNerve(&NrvDestroyCounter::DestroyCounterNrvWait::sInstance)) setNerve(&NrvDestroyCounter::DestroyCounterNrvAppear::sInstance);
            else setNerve(&NrvDestroyCounter::DestroyCounterNrvWait::sInstance);
        }
    }
    MR::setTextBoxFormatRecursive(this, "Counter", L"%02d", mDisplayedCount);
    if (!isNerve(&NrvDestroyCounter::DestroyCounterNrvDisappear::sInstance) && mDisplayedCount == mMaximum) setNerve(&NrvDestroyCounter::DestroyCounterNrvDisappear::sInstance);
}

void DestroyCounter::exeAppear() {
    if (MR::isFirstStep(this)) {
        MR::showLayout(this);
        mAppearer->appear(TVec2f(0.0f, 0.0f));
        MR::startAnim(this, "Wait", 1);
    }
    if (mAppearer->isAppeared()) setNerve(&NrvDestroyCounter::DestroyCounterNrvWait::sInstance);
}

void DestroyCounter::exeWait() {}

void DestroyCounter::exeDisappear() {
    if (MR::isFirstStep(this)) mAppearer->disappear();
    if (mAppearer->isDisappeared()) kill();
}

DestroyCounter::~DestroyCounter() {}
