#include "Game/MapObj/LiftFlower.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include "math_types.hpp"
#include <revolution/types.h>

namespace {
    extern const f32 sWaitHeightRate;
    extern const f32 sRaisedHeightRate;
}

namespace NrvLiftFlower {
    class LiftFlowerNrvWait : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static LiftFlowerNrvWait sInstance;
    };
    LiftFlowerNrvWait LiftFlowerNrvWait::sInstance;
    class LiftFlowerNrvWaitImmediate : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static LiftFlowerNrvWaitImmediate sInstance;
    };
    LiftFlowerNrvWaitImmediate LiftFlowerNrvWaitImmediate::sInstance;
    NEW_NERVE(LiftFlowerNrvMoveUp, LiftFlower, MoveUp);
    NEW_NERVE(LiftFlowerNrvMoveStop, LiftFlower, MoveStop);
    NEW_NERVE(LiftFlowerNrvDisappearSign, LiftFlower, DisappearSign);
    NEW_NERVE(LiftFlowerNrvDisappear, LiftFlower, Disappear);
    NEW_NERVE(LiftFlowerNrvHidden, LiftFlower, Hidden);
    NEW_NERVE(LiftFlowerNrvAppear, LiftFlower, Appear);
};  // namespace NrvLiftFlower

LiftFlower::LiftFlower(const char* pName) : LiveActor(pName) {
    mHomePos.x = 0.0f;
    mHomePos.y = 0.0f;
    mHomePos.z = 0.0f;
    mMoveHeight = 600.0f;
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
    mDemoMtx.setInline(getBaseMtx());
    mDemoObj = MR::createDemoObj("リフトフラワーフレーム", "LiftFlowerFrame", mDemoMtx);
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
    }
    return false;
}

bool LiftFlower::exeWait(s32 delay) {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Wait");
        MR::validateClipping(this);
    }
    TVec3f up;
    MR::calcUpVec(&up, this);
    MR::setPosition(this, mHomePos + up * (mMoveHeight * sWaitHeightRate));
    if (MR::isGreaterStep(this, delay + 60)) return true;
    return false;
}

void LiftFlower::exeMoveUp() {
    TVec3f up;
    MR::calcUpVec(&up, this);
    s32 moveSteps = mMoveHeight / 10.0f;
    f32 t = MR::calcNerveEaseOutRate(this, moveSteps);
    MR::setPosition(this, mHomePos + up * (mMoveHeight * t));
    MR::startActionSound(this, "MoveUp", -1, -1, -1);
    if (MR::isGreaterStep(this, moveSteps)) {
        MR::startActionSound(this, "MoveStop", -1, -1, -1);
        setNerve(&NrvLiftFlower::LiftFlowerNrvMoveStop::sInstance);
    }
}

void LiftFlower::exeMoveStop() {
    TVec3f up;
    MR::calcUpVec(&up, this);
    MR::setPosition(this, mHomePos + up * (mMoveHeight * sRaisedHeightRate));
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
    MR::setPosition(this, mHomePos + up * (mMoveHeight * sRaisedHeightRate));
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
        setNerve(&NrvLiftFlower::LiftFlowerNrvWaitImmediate::sInstance);
    }
}

LiftFlower::~LiftFlower() {
}

inline void NrvLiftFlower::LiftFlowerNrvWaitImmediate::execute(Spine* spine) const {
    LiftFlower* actor = static_cast<LiftFlower*>(spine->mExecutor);
    if (actor->exeWait(0)) actor->setNerve(&LiftFlowerNrvMoveUp::sInstance);
}

inline void NrvLiftFlower::LiftFlowerNrvWait::execute(Spine* spine) const {
    LiftFlower* actor = static_cast<LiftFlower*>(spine->mExecutor);
    if (actor->exeWait(actor->mStopStep)) actor->setNerve(&LiftFlowerNrvMoveUp::sInstance);
}

namespace {
    const f32 sWaitHeightRate = 0.0f;
    const f32 sRaisedHeightRate = 1.0f;
}
