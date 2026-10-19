#include "Game/Boss/PoltaRock.hpp"
#include "Game/Util.hpp"
#include "Game/Enemy/WalkerStateBindStarPointer.hpp"
#include "Game/Boss/Polta.hpp"
#include "Game/Boss/PoltaArm.hpp"
#include "Game/Boss/PoltaFunction.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util/ActorMovementUtil.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/ActorShadowUtil.hpp"
#include "Game/Util/EffectUtil.hpp"
#include "Game/Util/LayoutUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/SoundUtil.hpp"

namespace NrvPoltaRock {
    NEW_NERVE(PoltaRockNrvFloat, PoltaRock, Float);
    NEW_NERVE(PoltaRockNrvSign, PoltaRock, Sign);
    NEW_NERVE(PoltaRockNrvMove, PoltaRock, Move);
    NEW_NERVE(PoltaRockNrvBindStarPointer, PoltaRock, BindStarPointer);
};  // namespace NrvPoltaRock

PoltaRock::PoltaRock(const char* pName)
    : LiveActor(pName), mOwner(nullptr), mPointerBindState(0), mPoseQuat(0.0f, 0.0f, 0.0f, 1.0f), mChaseDirection(0.0f, 0.0f, 1.0f), mOwnerLocalPos(0.0f, 0.0f, 0.0f), mType(0) {
}

void PoltaRock::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, "PoltaRock", false);
    mPointerBindState = new WalkerStateBindStarPointer(this, nullptr);
    initNerve(GET_NERVE(PoltaRock, PoltaRockNrvFloat), 1);
    MR::initActorState(this, mPointerBindState, GET_NERVE(PoltaRock, PoltaRockNrvBindStarPointer), "SupportFreeze");
    makeActorDead();
}

void PoltaRock::kill() {
    MR::startActionSound(this, "OjPoltaRockBreak");
    LiveActor::kill();
    switch (mType) {
    case 2:
        MR::emitEffect(this, "BreakYellow");
        break;
    case 0:
        MR::emitEffect(this, "BreakWhite");
        break;
    }
    mOwner = nullptr;
}

void PoltaRock::control() {
}

void PoltaRock::calcAndSetBaseMtx() {
    MR::setBaseTRMtx(this, mPoseQuat);
    MR::setBaseScale(this, mScale);
}

void PoltaRock::start(Polta* pPolta, const TVec3f& rPosition, const TVec3f& rVec2) {
    mOwner = pPolta;
    mPosition.set(rPosition);
    mOwnerLocalPos.set(rVec2);
    setNerve(GET_NERVE(PoltaRock, PoltaRockNrvFloat));
    appear();
    MR::offBind(this);
}

void PoltaRock::setColorWhite() {
    mType = 0;
    MR::startBva(this, "Color");
    MR::setBvaFrameAndStop(this, 0.0f);
}

void PoltaRock::setColorBlack() {
    mType = 1;
    MR::startBva(this, "Color");
    MR::setBvaFrameAndStop(this, 1.0f);
}

void PoltaRock::setColorYellow() {
    mType = 2;
    MR::startBva(this, "Color");
    MR::setBvaFrameAndStop(this, 2.0f);
}

void PoltaRock::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorPlayer(pReceiver)) {
        if (isEnableAttack()) {
            if (MR::sendMsgEnemyAttack(pReceiver, pSender)) {
                generateKill();
            }
        } else {
            MR::sendMsgPush(pReceiver, pSender);
        }
    }
}

bool PoltaRock::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgLockOnStarPieceShoot(msg)) {
        return true;
    }
    if (MR::isMsgStarPieceAttack(msg)) {
        MR::zeroVelocity(this);
        generateKill();
        return true;
    }
    if (MR::isMsgPlayerSpinAttackOrSupportTico(msg) && isEnableAttack()) {
        MR::zeroVelocity(this);
        generateKill();
        return true;
    }
    return false;
}

bool PoltaRock::receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgExplosionAttack(msg) && isEnableExplosion()) {
        MR::zeroVelocity(this);
        generateKill();
        return true;
    }
    return false;
}

void PoltaRock::exeFloat() {
    TVec3f v3;

    if (MR::isFirstStep(this)) {
        MR::startActionSound(this, "OjPoltaRockFloatSt");
        MR::startAction(this, "Froat");
        MR::emitEffect(this, "Appear");
    }
    MR::startActionSound(this, "OjLvPoltaRockFloat");
    PSMTXMultVec(getOwner()->getBaseMtx(), &mOwnerLocalPos, v3);
    MR::addVelocityMoveToTargetHorizon(this, v3, 0.0f, 1.2f, 0.0f, 200.0f);
    MR::addVelocityKeepHeightUseShadow(this, 800.0f, 1.5f, 300.0f, nullptr);
    MR::attenuateVelocity(this, 0.92f);
    if (MR::isGreaterStep(this, 20) && tryStartPointBind(true)) {
        return;
    }
    if (MR::isGreaterStep(this, 90)) {
        setNerve(GET_NERVE(PoltaRock, PoltaRockNrvSign));
    }
}

void PoltaRock::exeSign() {
    TVec3f v3;

    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Sign");
        MR::onBind(this);
    }
    PSMTXMultVec(getOwner()->getBaseMtx(), &mOwnerLocalPos, v3);
    MR::addVelocityMoveToTargetHorizon(this, v3, 0.0f, 1.2f, 0.0f, 200.0f);
    MR::addVelocityKeepHeightUseShadow(this, 800.0f, 1.5f, 300.0f, nullptr);
    MR::attenuateVelocity(this, 0.92f);
    if (MR::isGreaterStep(this, 60)) {
        mChaseDirection.set(*MR::getPlayerCenterPos() - mPosition);
        MR::normalizeOrZero(&mChaseDirection);
        setNerve(GET_NERVE(PoltaRock, PoltaRockNrvMove));
        MR::onBind(this);
    } else {
        if (tryStartPointBind(true)) {
            return;
        }
    }
}

void PoltaRock::exeMove() {
    if (MR::isFirstStep(this)) {
        MR::startActionSound(this, "OjPoltaRockFly");
        MR::zeroVelocity(this);
    }

    if (MR::isLessStep(this, 30)) {
        mVelocity.add(mChaseDirection * (3.0f * MR::calcNerveRate(this, 30)));
    }
    MR::addVelocityToGravity(this, -0.02f);
    MR::attenuateVelocity(this, 0.999f);
    MR::rotateQuatRollBall(&mPoseQuat, mVelocity, -mGravity, 120.0f);
    if (MR::isBinded(this)) {
        MR::sendMsgEnemyAttackToBindedSensor(this, getSensor("Body"));
        MR::tryRumbleDefaultHit(this, 0);
        generateKill();
        return;
    }

    if (MR::isGreaterStep(this, 300)) {
        kill();
    } else {
        if (tryStartPointBind(false)) {
            return;
        }
    }
}

void PoltaRock::generateKill() {
    switch (mType) {
    case 1: {
        TVec3f velocity;
        velocity.x = 0.0f;
        velocity.y = 0.0f;
        velocity.z = 0.0f;
        PoltaFunction::appearBombTeresaNormal(mOwner, mPosition, velocity);
        break;
    }
    case 2:
        MR::appearCoinPop(getOwner(), mPosition, 1);
        break;
    }
    kill();
}

bool PoltaRock::isEnableAttack() const {
    if (isNerve(GET_NERVE(PoltaRock, PoltaRockNrvMove)) || isNerve(GET_NERVE(PoltaRock, PoltaRockNrvBindStarPointer))) {
        return true;
    }
    return false;
}

bool PoltaRock::isEnableExplosion() const {
    if (isNerve(GET_NERVE(PoltaRock, PoltaRockNrvFloat)) && MR::isLessStep(this, 200)) {
        return false;
    }
    return true;
}

bool PoltaRock::tryStartPointBind(bool aimAtPlayer) {
    if (mPointerBindState->tryStartPointBind()) {
        if (aimAtPlayer) {
            mChaseDirection.set(*MR::getPlayerCenterPos() - mPosition);
            MR::normalizeOrZero(&mChaseDirection);
        }
        setNerve(GET_NERVE(PoltaRock, PoltaRockNrvBindStarPointer));
        return true;
    }
    return false;
}

PoltaRock::~PoltaRock() {
}

void PoltaRock::exeBindStarPointer() {
    if (MR::isFirstStep(this)) {
        MR::onBind(this);
    }
    MR::updateActorStateAndNextNerve(this, GET_NERVE(PoltaRock, PoltaRockNrvMove));
}
