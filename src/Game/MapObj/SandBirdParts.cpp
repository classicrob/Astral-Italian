#include "Game/MapObj/SandBirdParts.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/MapObj/MapObjConnector.hpp"
#include "Game/Util.hpp"

// Nerve executors that the target inlines into their Nrv::execute (defined before the NEW_NERVE block).
inline void SandBirdParts::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::tryStartAllAnim(this, "Wait");
    }
}

inline void SandBirdParts::exeMoveSign() {
    if (MR::isFirstStep(this)) {
        MR::tryStartAllAnim(this, "MoveSign");
    }
}

inline void SandBirdParts::exeMove() {
    if (MR::isFirstStep(this)) {
        MR::tryStartAllAnim(this, "Move");
    }
}

namespace NrvSandBirdParts {
    NEW_NERVE(SandBirdPartsNrvWait, SandBirdParts, Wait);
    NEW_NERVE(SandBirdPartsNrvMoveSign, SandBirdParts, MoveSign);
    NEW_NERVE(SandBirdPartsNrvMove, SandBirdParts, Move);
};  // namespace NrvSandBirdParts

inline void SandBirdParts::exeAppear() {
    if (updateAppear()) {
        setNerve(GET_NERVE(SandBirdParts, SandBirdPartsNrvWait));
    }
}


namespace NrvSandBirdParts {
    NEW_NERVE(SandBirdPartsNrvBreak, SandBirdParts, Break);
    NEW_NERVE(SandBirdPartsNrvHide, SandBirdParts, Hide);
    NEW_NERVE(SandBirdPartsNrvRevive, SandBirdParts, Revive);
    NEW_NERVE(SandBirdPartsNrvShrink, SandBirdParts, Shrink);
    NEW_NERVE(SandBirdPartsNrvShrinkHide, SandBirdParts, ShrinkHide);
    NEW_NERVE(SandBirdPartsNrvAppear, SandBirdParts, Appear);
};  // namespace NrvSandBirdParts

SandBirdParts::SandBirdParts(const char* pName, LiveActor* pParent, HitSensor* pSensor)
    : LiveActor(pName), mParent(pParent), mConnector(nullptr), mBodySensor(pSensor), mBaseScale(1.0f, 1.0f, 1.0f), mStepLimit(100) {
    mCollisionMtx.identity();
}

void SandBirdParts::init(const JMapInfoIter& rIter) {
    const char* name;
    MR::getObjectName(&name, rIter);
    MR::initActor(this, rIter, name, false);
    MR::initCollisionParts(this, name, mBodySensor, mCollisionMtx);
    initNerve(GET_NERVE(SandBirdParts, SandBirdPartsNrvWait), 0);
    mBaseScale.set(mScale);
    MR::getJMapInfoArg0NoInit(rIter, &mStepLimit);
    MR::excludeCalcShadowToActorAll(this, mParent);
    MR::validateClipping(this);
    mConnector = new MapObjConnector(this);
    mConnector->attachToActor(mParent);
    makeActorAppeared();
}

bool SandBirdParts::receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgEnemyAttack(msg) || MR::isMsgEnemyAttackFire(msg) || MR::isMsgExplosionAttack(msg) || MR::isMsgToEnemyAttackBlow(msg)
        || MR::isMsgToEnemyAttackRoll(msg)) {
        if (isNerve(GET_NERVE(SandBirdParts, SandBirdPartsNrvMove))) {
            MR::invalidateClipping(this);
            setNerve(GET_NERVE(SandBirdParts, SandBirdPartsNrvBreak));
            return !MR::isMsgEnemyAttackFire(msg);
        }
    }
    return false;
}

bool SandBirdParts::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgFloorTouch(msg)) {
        mParent->receiveOtherMsg(msg, pSender, pReceiver);
    }
    return false;
}

void SandBirdParts::control() {}

// NONMATCHING: 98%, psq_l/psq_st copy uses f5..f0 in reverse register order (regalloc only)
void SandBirdParts::calcAndSetBaseMtx() {
    LiveActor::calcAndSetBaseMtx();
    mConnector->connect();
    JMath::gekko_ps_copy12(&mCollisionMtx, getBaseMtx());
}

void SandBirdParts::exeBreak() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Shurink");
        MR::tryHitReactionBreak(this);
        MR::startActionSound(this, "Break", -1, -1, -1);
    }

    if (updateShrink()) {
        setNerve(GET_NERVE(SandBirdParts, SandBirdPartsNrvHide));
    }
}

void SandBirdParts::exeHide() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Hide");
        MR::hideModelAndOnCalcAnim(this);
        MR::invalidateCollisionParts(this);
        mScale.set(mBaseScale * 0.05f);
    }

    if (MR::isGreaterStep(this, mStepLimit)) {
        setNerve(GET_NERVE(SandBirdParts, SandBirdPartsNrvRevive));
    }
}

void SandBirdParts::exeRevive() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Revive");
    }
    MR::startActionSound(this, "Revive", -1, -1, -1);

    if (updateAppear()) {
        setNerve(GET_NERVE(SandBirdParts, SandBirdPartsNrvMove));
    }
}

void SandBirdParts::exeShrink() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Shurink");
    }

    if (updateShrink()) {
        setNerve(GET_NERVE(SandBirdParts, SandBirdPartsNrvShrinkHide));
    }
}

void SandBirdParts::exeShrinkHide() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Hide");
        MR::hideModelAndOnCalcAnim(this);
        MR::invalidateCollisionParts(this);
        mScale.set(mBaseScale * 0.05f);
    }
}

bool SandBirdParts::updateShrink() {
    f32 ease = MR::calcNerveEaseInOutValue(this, 0x1E, 1.0f, 0.05f);
    mScale.set(mBaseScale * ease);
    return MR::isGreaterStep(this, 0x1E);
}

bool SandBirdParts::updateAppear() {
    if (MR::isFirstStep(this)) {
        MR::showModel(this);
        MR::validateCollisionParts(this);
    }

    f32 ease = MR::calcNerveEaseInOutValue(this, 0x1E, 0.05f, 1.0f);
    mScale.set(mBaseScale * ease);
    if (MR::isGreaterStep(this, 0x1E)) {
        return true;
    }
    return false;
}

bool isPartsWait(LiveActor* pPart) {
    return pPart->isNerve(GET_NERVE(SandBirdParts, SandBirdPartsNrvWait));
}

void setPartsMoveSign(LiveActor* pPart) {
    MR::invalidateClipping(pPart);
    pPart->setNerve(GET_NERVE(SandBirdParts, SandBirdPartsNrvMoveSign));
}

void setPartsMove(LiveActor* pPart) {
    MR::invalidateClipping(pPart);
    pPart->setNerve(GET_NERVE(SandBirdParts, SandBirdPartsNrvMove));
}

void setPartsShrink(LiveActor* pPart) {
    if (pPart->isNerve(GET_NERVE(SandBirdParts, SandBirdPartsNrvWait)) || pPart->isNerve(GET_NERVE(SandBirdParts, SandBirdPartsNrvBreak))) {
        pPart->setNerve(GET_NERVE(SandBirdParts, SandBirdPartsNrvShrink));
    } else {
        pPart->setNerve(GET_NERVE(SandBirdParts, SandBirdPartsNrvShrinkHide));
    }
}

void setPartsAppear(LiveActor* pPart) {
    pPart->setNerve(GET_NERVE(SandBirdParts, SandBirdPartsNrvAppear));
}

SandBirdParts::~SandBirdParts() {}

