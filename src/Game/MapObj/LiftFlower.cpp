#include "Game/MapObj/LiftFlower.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include "math_types.hpp"
#include <revolution/types.h>

namespace NrvLiftFlower {
    NEW_NERVE(LiftFlowerNrvWait, LiftFlower, Wait);
    NEW_NERVE(LiftFlowerNrvMoveUp, LiftFlower, MoveUp);
    NEW_NERVE(LiftFlowerNrvMoveStop, LiftFlower, MoveStop);
    NEW_NERVE(LiftFlowerNrvDisappearSign, LiftFlower, DisappearSign);
    NEW_NERVE(LiftFlowerNrvDisappear, LiftFlower, Disappear);
    NEW_NERVE(LiftFlowerNrvHidden, LiftFlower, Hidden);
    NEW_NERVE(LiftFlowerNrvAppear, LiftFlower, Appear);
};  // namespace NrvLiftFlower

LiftFlower::LiftFlower(const char* pName) : LiveActor(pName) {
    mHomePos.zero();
    mMoveHeight = 100.0f;
    mBounceTimer = 0;
    mStopStep = 0;
    mDemoObj = nullptr;
    mDemoMtx.identity();
}

void LiftFlower::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, false);
    mHomePos.set< f32 >(mPosition);
    MR::getJMapInfoArg0NoInit(rIter, &mMoveHeight);
    MR::getJMapInfoArg1NoInit(rIter, &mStopStep);
    getBaseMtx();
    mDemoObj = MR::createDemoObj("LiftFlowerFrame", "LiftFlower", mDemoMtx);
    MR::excludeCalcShadowToMyCollision(this, nullptr);
    initNerve(&NrvLiftFlower::LiftFlowerNrvWait::sInstance, 0);
    MR::calcGravity(this);
    makeActorAppeared();
}

void LiftFlower::control() {
    if (mBounceTimer > 0) {
        mBounceTimer--;
    }
}

bool LiftFlower::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgFloorTouch(msg)) {
        mBounceTimer = 10;
        return false;
    }
    return false;
}

void LiftFlower::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Wait");
        MR::validateClipping(this);
    }
    TVec3f up;
    MR::calcUpVec(&up, this);
    TVec3f offset = up * (mMoveHeight * 1.0f);
    TVec3f pos = mHomePos + offset;
    MR::setPosition(this, pos);
    if (MR::isGreaterStep(this, mStopStep)) {
        setNerve(&NrvLiftFlower::LiftFlowerNrvMoveUp::sInstance);
    }
}

void LiftFlower::exeMoveUp() {
    TVec3f up;
    MR::calcUpVec(&up, this);
    f32 t = MR::calcNerveEaseOutRate(this, mStopStep);
    TVec3f offset = up * (mMoveHeight * t);
    TVec3f pos = mHomePos + offset;
    MR::setPosition(this, pos);
    MR::startActionSound(this, "MoveUp", -1, -1, -1);
    if (MR::isGreaterStep(this, mStopStep)) {
        MR::startActionSound(this, "MoveStop", -1, -1, -1);
        setNerve(&NrvLiftFlower::LiftFlowerNrvMoveStop::sInstance);
    }
}

void LiftFlower::exeMoveStop() {
    TVec3f up;
    MR::calcUpVec(&up, this);
    TVec3f offset = up * (mMoveHeight * 0.5f);
    TVec3f pos = mHomePos + offset;
    MR::setPosition(this, pos);
    if (MR::isGreaterStep(this, 30)) {
        setNerve(&NrvLiftFlower::LiftFlowerNrvDisappearSign::sInstance);
    }
}

void LiftFlower::exeDisappearSign() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "DisappearSign");
    }
    if (MR::isStep(this, 10) || MR::isStep(this, 38) || MR::isStep(this, 66)) {
        MR::startActionSound(this, "DisappearSign", -1, -1, -1);
    }
    TVec3f up;
    MR::calcUpVec(&up, this);
    TVec3f offset = up * (mMoveHeight * 0.5f);
    TVec3f pos = mHomePos + offset;
    MR::setPosition(this, pos);
    if (MR::isBrkStopped(this)) {
        setNerve(&NrvLiftFlower::LiftFlowerNrvDisappear::sInstance);
    }
}

void LiftFlower::exeDisappear() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Disappear");
    }
    if (MR::isActionEnd(this)) {
        setNerve(&NrvLiftFlower::LiftFlowerNrvHidden::sInstance);
        MR::invalidateCollisionParts(this);
    }
}

void LiftFlower::exeHidden() {
    if (MR::isFirstStep(this)) {
        MR::hideModel(this);
    }
    if (MR::isGreaterStep(this, 45)) {
        MR::resetPosition(this, mHomePos);
        setNerve(&NrvLiftFlower::LiftFlowerNrvAppear::sInstance);
    }
}

void LiftFlower::exeAppear() {
    if (MR::isFirstStep(this)) {
        MR::showModel(this);
        MR::startAction(this, "Appear");
    }
    if (MR::isStep(this, 1)) {
        MR::validateCollisionParts(this);
    }
    if (MR::isActionEnd(this)) {
        setNerve(&NrvLiftFlower::LiftFlowerNrvWait::sInstance);
    }
}

LiftFlower::~LiftFlower() {
}
