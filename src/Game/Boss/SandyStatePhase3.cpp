#include "Game/Boss/Sandy.hpp"
// SMG2: MR::appearCoinPop returns bool (ObjUtil.hpp declares it void), declare the real one here.
#define appearCoinPop appearCoinPopUnused
#include "Game/Util.hpp"
#undef appearCoinPop
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/Enemy/WalkerStateBindStarPointer.hpp"
#include "Game/LiveActor/ActorCameraInfo.hpp"
#include "Game/LiveActor/ActorJointCtrl.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/ModelObj.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/LiveActor/Spine.hpp"
#include "Game/Util/FixedPosition.hpp"
#include <cstdio>

// Unnamed SMG2 helpers outside this unit.
// PLACEHOLDER(objects): unnamed target symbol fn_8009A780, declared extern "C" so the reloc name matches; resolve when fn_8009A780 gets its real name in symbols.txt
extern "C" void fn_8009A780(TPos3f*, const TQuat4f*, const TVec3f*);

namespace NrvSandyStatePhase3 {
    class SandyStatePhase3NrvClose;
    class SandyStatePhase3NrvDamageClose;
    class SandyStatePhase3NrvOpen;

    NEW_NERVE(SandyStatePhase3NrvStart, SandyStatePhase3, Start);
    class SandyStatePhase3NrvClose : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static SandyStatePhase3NrvClose sInstance;
    };
    NEW_NERVE(SandyStatePhase3NrvOpen, SandyStatePhase3, Open);
    NEW_NERVE(SandyStatePhase3NrvOpenWait, SandyStatePhase3, OpenWait);
    NEW_NERVE(SandyStatePhase3NrvDamage, SandyStatePhase3, Damage);
    class SandyStatePhase3NrvDamageClose : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static SandyStatePhase3NrvDamageClose sInstance;
    };
    NEW_NERVE(SandyStatePhase3NrvThrow, SandyStatePhase3, Throw);
    NEW_NERVE(SandyStatePhase3NrvThrowBomb, SandyStatePhase3, ThrowBomb);
    SandyStatePhase3NrvClose SandyStatePhase3NrvClose::sInstance ATTRIBUTE_WEAK;
    SandyStatePhase3NrvDamageClose SandyStatePhase3NrvDamageClose::sInstance ATTRIBUTE_WEAK;

    inline void SandyStatePhase3NrvClose::execute(Spine* pSpine) const {
        SandyStatePhase3* pState = reinterpret_cast< SandyStatePhase3* >(pSpine->mExecutor);
        if (sandyStateClose(pState, false)) {
            pState->setNerve(&SandyStatePhase3NrvThrow::sInstance);
        }
    }
    inline void SandyStatePhase3NrvDamageClose::execute(Spine* pSpine) const {
        SandyStatePhase3* pState = reinterpret_cast< SandyStatePhase3* >(pSpine->mExecutor);
        if (sandyStateClose(pState, true)) {
            pState->setNerve(&SandyStatePhase3NrvThrow::sInstance);
        }
    }
};  // namespace NrvSandyStatePhase3

// 80102320
SandyStatePhase3::SandyStatePhase3(Sandy* pSandy)
    : ActorStateBase< Sandy >("\203t\203F\201[\203Y\202R", pSandy), mDamage(nullptr), mMiniWait0(-1), mMiniWait1(-1) {
    initNerve(&NrvSandyStatePhase3::SandyStatePhase3NrvThrow::sInstance, 1);
    mDamage = new SandyStateDamage(mHost, 120, 120, 40);
    MR::initActorState(this, mDamage, &NrvSandyStatePhase3::SandyStatePhase3NrvDamage::sInstance, nullptr);
}

// 801023D0
void SandyStatePhase3::appear() {
    mIsDead = false;
    setNerve(&NrvSandyStatePhase3::SandyStatePhase3NrvStart::sInstance);
}

// 801023E0
void SandyStatePhase3::control() {
    if (!isNerve(&NrvSandyStatePhase3::SandyStatePhase3NrvDamage::sInstance)) {
        mHost->updateTarget();
        fn_8009A780(&mHost->mMtx, &mHost->mTargetQuat, &mHost->mPosition);
    } else {
        TQuat4f quat;
        mHost->mMtx.getQuat(quat);
        turnToPlayer(&quat, mHost, 1.2f);
        fn_8009A780(&mHost->mMtx, &quat, &mHost->mPosition);
    }
    updateMinis();
}

// 80102480
void SandyStatePhase3::updateMinis() {
    if (mHost->getLeftMini()->isDownOrDead() && mMiniWait0 < 0) {
        mMiniWait0 = 900;
    }
    if (mHost->getRightMini()->isDownOrDead() && mMiniWait1 < 0) {
        mMiniWait1 = 900;
    }
    if (mMiniWait0 == 0) {
        if (mHost->getLeftMini()->isDownOrDead()) {
            mHost->getLeftMini()->setRecover();
        }
    }
    if (mMiniWait1 == 0) {
        if (mHost->getRightMini()->isDownOrDead()) {
            mHost->getRightMini()->setRecover();
        }
    }
    if (mMiniWait0 >= 0) {
        mMiniWait0--;
    }
    if (mMiniWait1 >= 0) {
        mMiniWait1--;
    }
}

// 80102580
bool SandyStatePhase3::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(&NrvSandyStatePhase3::SandyStatePhase3NrvDamage::sInstance)) {
        return mDamage->receiveMsgPlayerAttack(msg, pSender, pReceiver);
    }
    if (isNerve(&NrvSandyStatePhase3::SandyStatePhase3NrvOpenWait::sInstance)) {
        if (MR::isMsgFireBallAttack(msg)) {
            if (!mHost->isSensorBack(pSender)) {
                setNerve(&NrvSandyStatePhase3::SandyStatePhase3NrvDamage::sInstance);
            } else {
                mHost->startScaleHitReaction();
            }
            return true;
        }
        return false;
    }
    if (!MR::isEqualSensor(pReceiver, mHost, "Mouth")) {
        if (MR::isMsgStarPieceReflect(msg) || MR::isMsgFireBallAttack(msg)) {
            mHost->startScaleHitReaction();
            return true;
        }
        return false;
    }
    return false;
}

// 801026B0
void SandyStatePhase3::exeStart() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mHost, "WaitNormal");
        mHost->setHandsStep(90);
        mHost->setHandsBallType0();
    }
    if (MR::isGreaterStep(this, 180) && MR::isBckLooped(mHost)) {
        setNerve(&NrvSandyStatePhase3::SandyStatePhase3NrvThrow::sInstance);
    }
}

// 80102740
void SandyStatePhase3::exeOpen() {
    if (MR::isFirstStep(this)) {
        mHost->invalidateHands();
    }
    if (sandyStateOpenStart(this, 60)) {
        setNerve(&NrvSandyStatePhase3::SandyStatePhase3NrvOpenWait::sInstance);
    }
}

// 801027A0
void SandyStatePhase3::exeOpenWait() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mHost, "OpenWait");
        mHost->invalidateHands();
        mHost->getLeftMini()->tryStartNext();
    }
    if (MR::isStep(this, 90)) {
        mHost->getRightMini()->tryStartNext();
    }
    if (MR::isGreaterStep(this, 105)) {
        setNerve(&NrvSandyStatePhase3::SandyStatePhase3NrvClose::sInstance);
    }
}

// 80102840
void SandyStatePhase3::exeDamage() {
    MR::updateActorState(this);
    if (mDamage->mIsDead) {
        mHost->invalidateHands();
        if (mDamage->mIsEnd) {
            kill();
        } else {
            setNerve(&NrvSandyStatePhase3::SandyStatePhase3NrvDamageClose::sInstance);
        }
    }
}

// 801028C0
bool SandyStatePhase3::isMinisReady() const {
    if (mHost->getLeftMini()->isDownOrDead()) {
        if (mHost->getRightMini()->isDownOrDead()) {
            return true;
        }
    }
    if (mHost->isTotalCount(4)) {
        if (!mHost->getLeftMini()->isShot()) {
            if (!mHost->getRightMini()->isShot()) {
                return true;
            }
        }
    }
    return false;
}

// 80102960
void SandyStatePhase3::exeThrow() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mHost, "WaitNormal");
        mHost->invalidateHands();
        mHost->resetCount();
    }
    if (isMinisReady()) {
        setNerve(&NrvSandyStatePhase3::SandyStatePhase3NrvThrowBomb::sInstance);
    }
}

// 801029D0
void SandyStatePhase3::exeThrowBomb() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mHost, "WaitNormal");
        mHost->resetCount();
        mHost->setHandsBallType3();
        mHost->validateHands();
        mHost->setMinisNext();
    }
    if (MR::isGreaterStep(this, 60)) {
        setNerve(&NrvSandyStatePhase3::SandyStatePhase3NrvOpen::sInstance);
    }
}

// 80102A60
SandyStatePhase3::~SandyStatePhase3() {
}
