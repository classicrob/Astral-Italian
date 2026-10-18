#include "Game/Boss/Sandy.hpp"
#include "Game/Util.hpp"
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

// Shared state helpers
// 80100030
bool sandyStateOpenStart(ActorStateBase< Sandy >* pState, s32 step) {
    if (MR::isFirstStep(pState)) {
        MR::startAction(pState->mHost, "OpenStart");
    }
    return MR::isGreaterEqualStep(pState, step);
}

// 80100090
bool sandyStateOpenWait(ActorStateBase< Sandy >* pState, s32 step) {
    if (MR::isFirstStep(pState)) {
        MR::startAction(pState->mHost, "OpenWait");
    }
    return MR::isGreaterEqualStep(pState, step);
}

// 801000F0
bool sandyStateClose(ActorStateBase< Sandy >* pState, bool isAll) {
    if (MR::isFirstStep(pState)) {
        if (isAll) {
            pState->mHost->startActionAll("CloseStart", nullptr);
        } else {
            MR::startAction(pState->mHost, "CloseStart");
        }
    }
    return MR::isActionEnd(pState->mHost);
}

// 80100170
// NONMATCHING: commuted floating multiply operands in the half-interval conversion.
bool sandyStateThrow(ActorStateBase< Sandy >* pState, s32 count, s32 interval, bool isBoth) {
    if (MR::isFirstStep(pState)) {
        MR::startAction(pState->mHost, "WaitNormal");
        pState->mHost->getLeftHand()->setWait();
        {
            SandyHand* hand = pState->mHost->getLeftHand();
            hand->mIsThrower = true;
        }
        pState->mHost->getLeftHand()->mWaitStep = interval;
        if (isBoth) {
            {
                SandyHand* hand = pState->mHost->getRightHand();
                hand->mIsThrower = false;
            }
            pState->mHost->getRightHand()->forceWait();
        }
        pState->mHost->resetCount();
    }
    if (isBoth && MR::isStep(pState, (s32)((f32)interval * 0.5f))) {
        {
            SandyHand* hand = pState->mHost->getRightHand();
            hand->mIsThrower = true;
        }
        pState->mHost->getRightHand()->setWait();
        pState->mHost->getRightHand()->mWaitStep = interval;
    }
    if (pState->mHost->isTotalCount(count)) {
        pState->mHost->invalidateHands();
        return true;
    }
    return false;
}

namespace NrvSandyStateDamage {
    class SandyStateDamageNrvWait : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
        }
        static SandyStateDamageNrvWait sInstance;
    };
    SandyStateDamageNrvWait SandyStateDamageNrvWait::sInstance ATTRIBUTE_WEAK;
    NEW_NERVE(SandyStateDamageNrvDamageFirst, SandyStateDamage, DamageFirst);
    NEW_NERVE(SandyStateDamageNrvOpenWait, SandyStateDamage, OpenWait);
    NEW_NERVE(SandyStateDamageNrvOpenWaitTurn, SandyStateDamage, OpenWaitTurn);
    NEW_NERVE(SandyStateDamageNrvDamageMini, SandyStateDamage, DamageMini);
    NEW_NERVE(SandyStateDamageNrvDamage, SandyStateDamage, Damage);
};  // namespace NrvSandyStateDamage

// 80100320
SandyStateDamage::SandyStateDamage(Sandy* pSandy, s32 damageStep, s32 openStep, s32 damageMax)
    : ActorStateBase< Sandy >("\203_\203\201\201[\203W", pSandy), mDamageCount(0), mFrontVec(0.0f, 0.0f, 1.0f), mTurnRate(0.0f), mIsEnd(false),
      mDamageStep(damageStep), mOpenStep(openStep), mPlayerPos(0.0f, 0.0f, 0.0f), mDamageMax(damageMax) {
    initNerve(&NrvSandyStateDamage::SandyStateDamageNrvWait::sInstance, 0);
}

// 801003E0
void SandyStateDamage::appear() {
    mIsDead = false;
    mDamageCount = 0;
    mIsEnd = false;
    mHost->forceWaitHands();
    setNerve(&NrvSandyStateDamage::SandyStateDamageNrvDamageFirst::sInstance);
}

// 80100430
void SandyStateDamage::kill() {
    MR::startBrk(mHost, "Sandy");
    mHost->setWaitHands();
    mIsDead = true;
}

// 80100480
bool SandyStateDamage::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (!MR::isEqualSensor(pReceiver, mHost, "Mouth")) {
        return false;
    }
    if (MR::isMsgFireBallAttack(msg)) {
        bool isDamageable = false;
        if (isNerve(&NrvSandyStateDamage::SandyStateDamageNrvDamageFirst::sInstance) ||
            isNerve(&NrvSandyStateDamage::SandyStateDamageNrvOpenWait::sInstance) ||
            isNerve(&NrvSandyStateDamage::SandyStateDamageNrvOpenWaitTurn::sInstance)) {
            isDamageable = true;
        }
        if (isDamageable && !mHost->isSensorBack(pSender)) {
            incDamage();
            return true;
        }
        mHost->startScaleHitReaction();
        return true;
    }
    return false;
}

// 80100570
void SandyStateDamage::incDamage() {
    mDamageCount++;
    if (mDamageCount == 1) {
        const TVec3f* pPlayerPos = MR::getPlayerPos();
        mPlayerPos.x = pPlayerPos->x;
        mPlayerPos.y = pPlayerPos->y;
        mPlayerPos.z = pPlayerPos->z;
    }
    if (mDamageCount >= 4) {
        setNerve(&NrvSandyStateDamage::SandyStateDamageNrvDamage::sInstance);
        return;
    }
    setNerve(&NrvSandyStateDamage::SandyStateDamageNrvDamageMini::sInstance);
}

// 801005F0
void SandyStateDamage::exeDamageFirst() {
    if (MR::isFirstStep(this)) {
        mHost->startActionAll("DamageFirst", nullptr);
        mHost->mTargetQuat.getZDir(mFrontVec);
        mTurnRate = 0.2f;
        MR::startActionSound(mHost, "BmSandyDamageCount", mDamageCount, -1, -1);
    }
    if (MR::isActionEnd(mHost)) {
        setNerve(&NrvSandyStateDamage::SandyStateDamageNrvOpenWaitTurn::sInstance);
    }
}

// 80100690
void SandyStateDamage::exeOpenWait() {
    if (MR::isFirstStep(this)) {
        mHost->startActionAll("DamageOpenWait", nullptr);
    }
    if (MR::isGreaterStep(this, mOpenStep)) {
        MR::startActionSound(mHost, "DamageOpenWaitEnd", -1, -1, -1);
        kill();
    }
}

// 80100720
void SandyStateDamage::exeOpenWaitTurn() {
    if (MR::isFirstStep(this)) {
        mHost->startActionAll("DamageOpenWait", nullptr);
    }
    f32 rate = mTurnRate;
    TVec3f front;
    TVec3f up = -mHost->mGravity;
    MR::rotateQuatAroundAxis(&mHost->mTargetQuat, mHost->mTargetQuat, up, 0.017453292f * rate);
    mHost->mTargetQuat.getZDir(front);
    if (20.0f < 57.29578f * MR::diffAngleAbs(mFrontVec, front)) {
        mTurnRate *= -1.0f;
    }
    if (MR::isGreaterStep(this, mDamageStep)) {
        MR::startActionSound(mHost, "DamageOpenWaitEnd", -1, -1, -1);
        kill();
    }
}

// 80100830
// NONMATCHING: host-pointer register lifetime around the negative-gravity temporary.
void SandyStateDamage::exeDamageMini() {
    if (MR::isFirstStep(this)) {
        const char* pName;
        if (mDamageCount == 2) {
            pName = "DamageMini2nd";
        } else {
            pName = "DamageMini";
            if (mDamageCount == 3) {
                pName = "DamageMini3rd";
            }
        }
        mHost->startActionAll(pName, nullptr);
        MR::startActionSound(mHost, "BmSandyDamageCount", mDamageCount * 2, -1, -1);
    }
    if (mDamageCount == 1) {
        TVec3f front;
        mHost->mTargetQuat.getZDir(front);
        MR::turnDirectionToTargetDegreeHorizon(mHost, &front, mPlayerPos, 3.0f);
        MR::makeQuatUpFront(&mHost->mTargetQuat, -mHost->mGravity, front);
    }
    if (MR::isActionEnd(mHost)) {
        setNerve(&NrvSandyStateDamage::SandyStateDamageNrvOpenWait::sInstance);
    }
}

// 80100930
// NONMATCHING: host-pointer load scheduled after the action-name branch.
void SandyStateDamage::exeDamage() {
    if (MR::isFirstStep(this)) {
        mHost->startActionAll(mDamageMax != -1 ? "DamageDown" : "DamageNormal", "Damage");
        if (mDamageMax != -1) {
            MR::startBrk(mHost, "Damage");
        }
        MR::startActionSound(mHost, "EstablishDamage", -1, -1, -1);
        mHost->appearStarPieceOnDamage();
    }
    if ((mDamageMax > 0 && MR::isGreaterEqualStep(this, mDamageMax)) || MR::isActionEnd(mHost)) {
        mIsEnd = true;
        kill();
    }
}

// 80100A20
SandyStateDamage::~SandyStateDamage() {
}
