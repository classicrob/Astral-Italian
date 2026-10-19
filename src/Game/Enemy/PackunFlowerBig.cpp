#include "Game/Util.hpp"
#include "Game/Enemy/PackunFlowerBig.hpp"
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/Enemy/WalkerStateBindStarPointer.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util/EffectUtil.hpp"
#include "Game/Util/ObjUtil.hpp"

bool PackunFlowerBig::isSensorInAttackRange(HitSensor* pSender, HitSensor* pReceiver, TVec3f* pDir, f32 scale) {
    TVec3f diff;
    diff.set(0.0f);
    diff.sub(*MR::getSensorPos(pSender), *MR::getSensorPos(pReceiver));
    MR::vecKillElement(diff, *pDir, &diff);
    return PSVECMag(&diff) < scale;
}

namespace NrvPackunFlowerBig {
    NEW_NERVE(PackunFlowerBigNrvHidden, PackunFlowerBig, Hidden);
    NEW_NERVE(PackunFlowerBigNrvAppear, PackunFlowerBig, Appear);
    NEW_NERVE(PackunFlowerBigNrvWait, PackunFlowerBig, Wait);
    NEW_NERVE(PackunFlowerBigNrvHide, PackunFlowerBig, Hide);
    NEW_NERVE(PackunFlowerBigNrvSpinReaction, PackunFlowerBig, SpinReaction);
    NEW_NERVE(PackunFlowerBigNrvFreeze, PackunFlowerBig, Freeze);
    NEW_NERVE(PackunFlowerBigNrvFreezeEnd, PackunFlowerBig, FreezeEnd);
};  // namespace NrvPackunFlowerBig

PackunFlowerBig::PackunFlowerBig(const char* pName) : LiveActor(pName) {
    mAnimScaleController = nullptr;
    mBindState = nullptr;
    mTimer = 0;
}

void PackunFlowerBig::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, "PackunFlowerBig", false);
    mAnimScaleController = new AnimScaleController(nullptr);
    mAnimScaleController->setParamTight();
    mBindState = new WalkerStateBindStarPointer(this, mAnimScaleController);
    initNerve(&NrvPackunFlowerBig::PackunFlowerBigNrvHidden::sInstance, 1);
    MR::initActorState(this, mBindState, &NrvPackunFlowerBig::PackunFlowerBigNrvFreezeEnd::sInstance, "SupportFreeze");
    makeActorAppeared();
}

void PackunFlowerBig::exeHidden() {
    if (MR::isFirstStep(this)) {
        MR::hideModel(this);
    }
    if (MR::isStep(this, 150)) {
        setNerve(&NrvPackunFlowerBig::PackunFlowerBigNrvAppear::sInstance);
    }
}

void PackunFlowerBig::exeAppear() {
    if (MR::isFirstStep(this)) {
        MR::showModel(this);
        MR::startAction(this, "Appear");
    }
    if (MR::isActionEnd(this)) {
        setNerve(&NrvPackunFlowerBig::PackunFlowerBigNrvWait::sInstance);
    }
}

void PackunFlowerBig::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Wait");
    }
    if (MR::isStep(this, 120)) {
        setNerve(&NrvPackunFlowerBig::PackunFlowerBigNrvHide::sInstance);
    }
}

void PackunFlowerBig::exeHide() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Hide");
    }
    if (MR::isActionEnd(this)) {
        setNerve(&NrvPackunFlowerBig::PackunFlowerBigNrvHidden::sInstance);
    }
}

void PackunFlowerBig::exeSpinReaction() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "SpinReaction");
        MR::tryRumbleDefaultHit(this, 0);
    }
    if (MR::isActionEnd(this)) {
        setNerve(&NrvPackunFlowerBig::PackunFlowerBigNrvWait::sInstance);
    }
}

void PackunFlowerBig::exeFreeze() {
    if (MR::isFirstStep(this)) {
        mAnimScaleController->startHitReaction();
    }
    if (!mAnimScaleController->isHitReaction(-1)) {
        setNerve(&NrvPackunFlowerBig::PackunFlowerBigNrvWait::sInstance);
    }
}

void PackunFlowerBig::exeFreezeEnd() {
    MR::updateActorStateAndNextNerve(this, &NrvPackunFlowerBig::PackunFlowerBigNrvWait::sInstance);
}

void PackunFlowerBig::calcAndSetBaseMtx() {
    LiveActor::calcAndSetBaseMtx();
    MR::updateBaseScale(this, mAnimScaleController);
}

void PackunFlowerBig::control() {
    mAnimScaleController->update();
    tryStarPointerBind();
    if (mTimer > 0) {
        mTimer--;
    }
}

void PackunFlowerBig::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorEnemyAttack(pSender) && MR::isSensorPlayerOrRide(pReceiver) && !isNerve(&NrvPackunFlowerBig::PackunFlowerBigNrvHidden::sInstance)) {
        if (isNerve(&NrvPackunFlowerBig::PackunFlowerBigNrvFreezeEnd::sInstance)) {
            if (isSensorInAttackRange(pSender, pReceiver, &mGravity, 400.0f)) {
                MR::sendMsgPush(pReceiver, pSender);
            }
        } else if (isSensorInAttackRange(pSender, pReceiver, &mGravity, 300.0f) && MR::sendMsgEnemyAttackOrPush(pReceiver, pSender)) {
            MR::tryHitReaction(this, "プレイヤー接触");
            if (!isNerve(&NrvPackunFlowerBig::PackunFlowerBigNrvHide::sInstance)) {
                setNerve(&NrvPackunFlowerBig::PackunFlowerBigNrvFreeze::sInstance);
            }
        }
    }
}

bool PackunFlowerBig::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (!MR::isSensorMapObj(pReceiver)) {
        return false;
    }
    if (MR::isMsgStarPieceReflect(msg)) {
        MR::tryHitReaction(this, "スターピースはじく");
        if (!isNerve(&NrvPackunFlowerBig::PackunFlowerBigNrvHide::sInstance)) {
            setNerve(&NrvPackunFlowerBig::PackunFlowerBigNrvFreeze::sInstance);
        }
        return true;
    }
    bool isAttackable = false;
    if (isNerve(&NrvPackunFlowerBig::PackunFlowerBigNrvWait::sInstance) || isNerve(&NrvPackunFlowerBig::PackunFlowerBigNrvSpinReaction::sInstance) || isNerve(&NrvPackunFlowerBig::PackunFlowerBigNrvFreeze::sInstance)) {
        isAttackable = true;
    }
    if (!isAttackable) {
        return false;
    }
    if (MR::isMsgPlayerSpinAttackOrSupportTico(msg) && mTimer <= 0) {
        mTimer = 30;
        MR::emitEffectHitBetweenSensors(this, pSender, pReceiver, 0.0f, nullptr);
        setNerve(&NrvPackunFlowerBig::PackunFlowerBigNrvSpinReaction::sInstance);
        return true;
    }
    return false;
}

bool PackunFlowerBig::tryStarPointerBind() {
    bool isBindable = false;
    if (isNerve(&NrvPackunFlowerBig::PackunFlowerBigNrvWait::sInstance) || isNerve(&NrvPackunFlowerBig::PackunFlowerBigNrvSpinReaction::sInstance) || isNerve(&NrvPackunFlowerBig::PackunFlowerBigNrvFreeze::sInstance)) {
        isBindable = true;
    }
    if (!isBindable) {
        return false;
    }
    if (isNerve(&NrvPackunFlowerBig::PackunFlowerBigNrvFreeze::sInstance)) {
        return false;
    }
    if (!mBindState->tryStartPointBind()) {
        return false;
    }
    setNerve(&NrvPackunFlowerBig::PackunFlowerBigNrvFreezeEnd::sInstance);
    return true;
}
