#include "Game/Enemy/JumpSpider.hpp"
#include "Game/Util.hpp"
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/Enemy/SpinHitController.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/ActorItemGenerator.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util/ActorMovementUtil.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/ActorShadowUtil.hpp"
#include "Game/Util/ActorSwitchUtil.hpp"
#include "Game/Util/EffectUtil.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MapUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include "Game/Util/StarPointerUtil.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>
#include <JSystem/JGeometry/TVec.hpp>
#include <revolution/types.h>
#include "Game/LiveActor/ActorItemGenerator.hpp"

namespace {
    // hCommonGravity
    // hNoPowerGravity
    static const f32 hGroundDamp = 0.8f;
    static const f32 hAirDampH = 0.98f;
    static const f32 hAirDampV = 0.98f;
    // hOnGroundTime
    // hJumpDistance
    static const f32 hJumpVelV = 1.0f;
    // hStampDeathFallVel
    static const s32 hStampDeathTime = 20;
    // hSearchTurnLimitRadian
    // hJumpTurnLimitRadian
    // hToPreJumpRestRadian
    // hWaitTime
    // hHideTime
    // hWaitUnderGroundTime
    static const s32 hNoPowerWaitTime = 240;
};  // namespace

namespace NrvJumpSpider {
    NEW_NERVE(HostTypeNrvWait, JumpSpider, Wait);
    NEW_NERVE(HostTypeNrvPreJump, JumpSpider, PreJump);
    NEW_NERVE(HostTypeNrvJump, JumpSpider, Jump);
    NEW_NERVE(HostTypeNrvJumpEnd, JumpSpider, JumpEnd);
    NEW_NERVE(HostTypeNrvHitToPlayer, JumpSpider, HitToPlayer);
    NEW_NERVE_ONEND(HostTypeNrvStampFall, JumpSpider, StampFall, StampFall);
    NEW_NERVE_ONEND(HostTypeNrvStampDeath, JumpSpider, StampDeath, StampDeath);
    NEW_NERVE_ONEND(HostTypeNrvHitBlow, JumpSpider, HitBlow, HitBlow);
    NEW_NERVE_ONEND(HostTypeNrvDpdPointed, JumpSpider, DpdPointed, DpdPointed);
    NEW_NERVE(HostTypeNrvDpdPointedEnd, JumpSpider, NoPowerStart);
    NEW_NERVE(HostTypeNrvStarPieceHit, JumpSpider, NoPowerStart);
    NEW_NERVE(HostTypeNrvDpdPointedLand, JumpSpider, NoPowerLand);
    NEW_NERVE(HostTypeNrvStarPieceHitLand, JumpSpider, NoPowerLand);
    NEW_NERVE(HostTypeNrvStarPieceHitWait, JumpSpider, NoPowerWait);
    NEW_NERVE(HostTypeNrvNoPowerEnd, JumpSpider, NoPowerEnd);
    NEW_NERVE(HostTypeNrvEaten, JumpSpider, Eaten);
};  // namespace NrvJumpSpider

JumpSpider::JumpSpider(const char* pName) : LiveActor(pName), mSpinHitController(), mScaleController(), mFront(0, 0, 1), mRespawnPos(0, 0, 0), mYoshiLockOn(nullptr) {
}

void JumpSpider::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, "JumpSpider", false);
    MR::initFur(this);

    if (MR::isValidInfo(rIter)) {
        MR::initUseStageSwitchWriteDead(this, rIter);
    }

    TPos3f pos;
    pos.setInline(getBaseMtx());
    pos.getZDir(mFront);
    initNerve(GET_NERVE(JumpSpider, HostTypeNrvWait), 0);
    mYoshiLockOn = MR::createYoshiLockOnTarget(this, MR::getJointMtx(this, "Hip"), TVec3f(0.0f));
    mSpinHitController = new SpinHitController(this);
    mScaleController = new AnimScaleController(nullptr);
    MR::offCalcGravity(this);
    MR::onCalcShadowAll(this);
    MR::emitEffect(this, "EyeLight");
}

void JumpSpider::initAfterPlacement() {
    MR::calcGravity(this);
    TVec3f v15(mPosition - mGravity * 10.0f);
    TVec3f v1, v2;

    if (MR::getFirstPolyNormalOnLineToMap(&v1, v15, mGravity * 300.0f, &v2, nullptr)) {
        mRespawnPos.set(v2);
        MR::setPosition(this, v2);
        mGravity.set(-v1);
    } else {
        kill();
        return;
    }

    makeActorAppeared();
}

void JumpSpider::kill() {
    LiveActor::kill();
    MR::emitEffect(this, "Death");

    if (MR::isValidSwitchDead(this)) {
        MR::onSwitchDead(this);
    }
}

void JumpSpider::startClipped() {
    LiveActor::startClipped();
    ;
    MR::deleteEffect(this, "EyeLight");
}

void JumpSpider::endClipped() {
    LiveActor::endClipped();
    MR::emitEffect(this, "EyeLight");
}

void JumpSpider::control() {
    mScaleController->updateNerve();

    if (isEatable()) {
        YoshiFunction::tryYoshiEat(mYoshiLockOn);
    }

    if (isStarPointerBindable() && MR::isStarPointerPointing2PAttachTarget(this)) {
        setNerve(GET_NERVE(JumpSpider, HostTypeNrvDpdPointed));
    }
}

bool JumpSpider::isEatable() const {
    bool ret = true;
    if (isNerve(GET_NERVE(JumpSpider, HostTypeNrvEaten)) || isNerve(GET_NERVE(JumpSpider, HostTypeNrvStampFall)) ||
        isNerve(GET_NERVE(JumpSpider, HostTypeNrvStampDeath)) || isNerve(GET_NERVE(JumpSpider, HostTypeNrvHitBlow))) {
        ret = false;
    }
    return ret;
}

bool JumpSpider::isStarPointerBindable() const {
    bool ret = true;
    if (isNerve(GET_NERVE(JumpSpider, HostTypeNrvEaten)) || isNerve(GET_NERVE(JumpSpider, HostTypeNrvStampFall)) ||
        isNerve(GET_NERVE(JumpSpider, HostTypeNrvStampDeath)) || isNerve(GET_NERVE(JumpSpider, HostTypeNrvHitBlow)) ||
        isNerve(GET_NERVE(JumpSpider, HostTypeNrvDpdPointed)) || isNerve(GET_NERVE(JumpSpider, HostTypeNrvStarPieceHit)) ||
        isNerve(GET_NERVE(JumpSpider, HostTypeNrvStarPieceHitLand))) {
        ret = false;
    }
    return ret;
}

void JumpSpider::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Wait");
    }

    if (MR::calcDistanceToPlayer(this) < 1000.0f) {
        MR::startActionSound(this, "EvJumpspiderFind");
        setNerve(GET_NERVE(JumpSpider, HostTypeNrvPreJump));
    }
}

void JumpSpider::exePreJump() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "JumpStart");
        MR::zeroVelocity(this);
    }

    TVec3f heightVec;
    MR::calcVecToPlayerH(&heightVec, this, nullptr);
    MR::turnVecToVecRadian(&mFront, mFront, heightVec, 0.05f, mGravity);

    if (MR::isActionEnd(this)) {
        setNerve(GET_NERVE(JumpSpider, HostTypeNrvJump));
    }
}

void JumpSpider::exeJump() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Jump");
        MR::startActionSound(this, "EmJumpspiderJump");
        mVelocity.set(-mGravity * 22.0f);
    }

    MR::applyVelocityDampAndGravity(this, 0.3f, ::hGroundDamp, ::hAirDampH, ::hAirDampV, ::hJumpVelV);
    TVec3f heightVec;
    MR::calcVecToPlayerH(&heightVec, this, nullptr);
    MR::turnVecToVecRadian(&mFront, mFront, heightVec, 0.02f, mGravity);

    if (MR::isOnGround(this)) {
        MR::startActionSound(this, "EmJumpspiderLand");
        MR::setPosition(this, mRespawnPos);
        MR::zeroVelocity(this);
        MR::startAction(this, "JumpEnd");
        setNerve(GET_NERVE(JumpSpider, HostTypeNrvJumpEnd));
    }
}

void JumpSpider::exeJumpEnd() {
    if (MR::isFirstStep(this)) {
    }

    MR::setPosition(this, mRespawnPos);

    if (MR::isActionEnd(this)) {
        if (1000.0f < MR::calcDistanceToPlayer(this)) {
            setNerve(GET_NERVE(JumpSpider, HostTypeNrvWait));
        } else {
            setNerve(GET_NERVE(JumpSpider, HostTypeNrvPreJump));
        }
    }
}

void JumpSpider::exeHitToPlayer() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Hit");
        MR::zeroVelocity(this);
    }

    MR::reboundVelocityFromCollision(this, 0.0f, 0.0f, 1.0f);
    MR::applyVelocityDampAndGravity(this, 0.3f, 0.8f, 0.98f, 0.98f, 1.0f);
    MR::killVelocityOnGroundH(this);

    if (MR::isGreaterStep(this, 58)) {
        TVec3f heightVec;
        MR::calcVecToPlayerH(&heightVec, this, nullptr);
        MR::turnVecToVecRadian(&mFront, mFront, heightVec, 0.05f, mGravity);
    }

    if (MR::isActionEnd(this)) {
        setNerve(GET_NERVE(JumpSpider, HostTypeNrvJump));
    }
}

void JumpSpider::exeStampFall() {
    if (MR::isFirstStep(this)) {
        mVelocity.set(mGravity * 30.0f);
        MR::invalidateClipping(this);
    }

    if (MR::isOnGround(this)) {
        MR::zeroVelocity(this);
        MR::setPosition(this, mRespawnPos);
        setNerve(GET_NERVE(JumpSpider, HostTypeNrvStampDeath));
    }
}

void JumpSpider::endStampFall() {
    MR::validateClipping(this);
}


void JumpSpider::exeStampDeath() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Press");
        MR::invalidateClipping(this);
        MR::zeroVelocity(this);
    }

    MR::setPosition(this, mRespawnPos);

    if (MR::isGreaterStep(this, ::hStampDeathTime)) {
        MR::startActionSound(this, "EmExplodeS");
        MR::emitItemStomp(this);
        kill();
    }
}

void JumpSpider::endStampDeath() {
    MR::validateClipping(this);
}

void JumpSpider::exeHitBlow() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Blow");
        MR::startBlowHitSound(this);
        MR::startActionSound(this, "EvJumpspiderDead");
        MR::onCalcGravity(this);
    }

    if (mSpinHitController->execute(this)) {
        MR::emitItemHit(this);
        MR::startActionSound(this, "EmExplodeS");
    }
}

void JumpSpider::endHitBlow() {
    MR::validateClipping(this);
}

void JumpSpider::exeDpdPointed() {
    if (MR::isFirstStep(this)) {
        MR::startDPDHitSound();
        MR::stopBck(this);
        mScaleController->startDpdHitVibration();
        MR::emitEffect(this, "Touch");
    }

    MR::startDPDFreezeLevelSound(this);

    if (!MR::isStarPointerPointing2PAttachTarget(this)) {
        setNerve(GET_NERVE(JumpSpider, HostTypeNrvDpdPointedEnd));
    } else {
        MR::zeroVelocity(this);
    }
}

void JumpSpider::endDpdPointed() {
    mScaleController->startAnim();
    MR::deleteEffect(this, "Touch");
}

void JumpSpider::exeNoPowerStart() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "NoPowerStart");
        MR::zeroVelocity(this);
    }

    MR::applyVelocityDampAndGravity(this, 0.8f, 0.8f, 0.98f, 0.98f, 1.0f);

    if (MR::isOnGround(this) && MR::isActionEnd(this)) {
        MR::setPosition(this, mRespawnPos);
        MR::startActionSound(this, "EmJumpspiderLand");

        if (isNerve(GET_NERVE(JumpSpider, HostTypeNrvStarPieceHit))) {
            MR::setPosition(this, mRespawnPos);
            MR::zeroVelocity(this);
            MR::startAction(this, "NoPowerLand");
            setNerve(GET_NERVE(JumpSpider, HostTypeNrvStarPieceHitLand));
        } else {
            MR::startAction(this, "NoPowerLand");
            setNerve(GET_NERVE(JumpSpider, HostTypeNrvDpdPointedLand));
        }
    }
}

void JumpSpider::exeNoPowerLand() {
    if (MR::isFirstStep(this)) {
        MR::zeroVelocity(this);
    }

    MR::setPosition(this, mRespawnPos);

    if (MR::isActionEnd(this)) {
        if (isNerve(GET_NERVE(JumpSpider, HostTypeNrvStarPieceHitLand))) {
            setNerve(GET_NERVE(JumpSpider, HostTypeNrvStarPieceHitWait));
        } else {
            setNerve(GET_NERVE(JumpSpider, HostTypeNrvNoPowerEnd));
        }
    }
}

void JumpSpider::exeNoPowerWait() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "NoPowerWait");
        MR::zeroVelocity(this);
    }

    MR::setPosition(this, mRespawnPos);
    MR::startActionSound(this, "EmLvSwoonS");

    if (MR::isGreaterStep(this, ::hNoPowerWaitTime)) {
        setNerve(GET_NERVE(JumpSpider, HostTypeNrvNoPowerEnd));
    }
}

void JumpSpider::exeNoPowerEnd() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "NoPowerEnd");
        MR::startActionSound(this, "EmJumpspiderStep");
    }

    MR::setPosition(this, mRespawnPos);

    if (MR::isActionEnd(this)) {
        setNerve(GET_NERVE(JumpSpider, HostTypeNrvWait));
    }
}

void JumpSpider::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(GET_NERVE(JumpSpider, HostTypeNrvHitBlow)) || isNerve(GET_NERVE(JumpSpider, HostTypeNrvStampDeath)) ||
        isNerve(GET_NERVE(JumpSpider, HostTypeNrvStampFall)) || isNerve(GET_NERVE(JumpSpider, HostTypeNrvEaten))) {
        return;
    }

    if (pSender != getSensor("body")) {
        return;
    }

    if (MR::isSensorPlayer(pReceiver)) {
        if (MR::isPlayerHipDropFalling() || MR::isPlayerHipDropLand()) {
            return;
        }

        if (isNerve(GET_NERVE(JumpSpider, HostTypeNrvWait)) || isNerve(GET_NERVE(JumpSpider, HostTypeNrvJumpEnd)) ||
            isNerve(GET_NERVE(JumpSpider, HostTypeNrvStarPieceHit)) || isNerve(GET_NERVE(JumpSpider, HostTypeNrvStarPieceHitLand)) ||
            isNerve(GET_NERVE(JumpSpider, HostTypeNrvStarPieceHitWait)) || isNerve(GET_NERVE(JumpSpider, HostTypeNrvDpdPointed)) ||
            isNerve(GET_NERVE(JumpSpider, HostTypeNrvDpdPointedLand)) || isNerve(GET_NERVE(JumpSpider, HostTypeNrvDpdPointedEnd)) ||
            isNerve(GET_NERVE(JumpSpider, HostTypeNrvNoPowerEnd))) {
            MR::sendMsgPush(pReceiver, pSender);
            return;
        }

        if (MR::isPlayerExistUp(this, MR::getSensorRadius(this, "body"), 0.25f)) {
            MR::sendMsgPush(pReceiver, pSender);
            return;
        }

        if (MR::sendMsgEnemyAttack(pReceiver, pSender) == true) {
            setNerve(GET_NERVE(JumpSpider, HostTypeNrvHitToPlayer));
        } else {
            MR::sendMsgPush(pReceiver, pSender);
        }
    } else {
        MR::sendMsgPush(pReceiver, pSender);
    }
}

bool JumpSpider::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(GET_NERVE(JumpSpider, HostTypeNrvHitBlow)) || isNerve(GET_NERVE(JumpSpider, HostTypeNrvStampDeath)) ||
        isNerve(GET_NERVE(JumpSpider, HostTypeNrvStampFall)) || isNerve(GET_NERVE(JumpSpider, HostTypeNrvEaten))) {
        return false;
    }

    if (MR::isMsgLockOnStarPieceShoot(msg)) {
        return true;
    }

    if (MR::isMsgStarPieceAttack(msg)) {
        if (!isNerve(GET_NERVE(JumpSpider, HostTypeNrvStarPieceHit))) {
            setNerve(GET_NERVE(JumpSpider, HostTypeNrvStarPieceHit));
        }

        return true;
    }

    if (MR::isMsgPlayerHipDrop(msg) || MR::isMsgPlayerTrample(msg)) {
        MR::startActionSound(this, "EmStompedS");
        MR::startActionSound(this, "EvJumpspiderDead");

        if (MR::isOnGround(this)) {
            setNerve(GET_NERVE(JumpSpider, HostTypeNrvStampDeath));
        } else {
            setNerve(GET_NERVE(JumpSpider, HostTypeNrvStampFall));
        }

        return true;
    }

    if (MR::isMsgPlayerHitAll(msg)) {
        mSpinHitController->start(this, *MR::getSensorPos(pSender), *MR::getSensorPos(pReceiver));
        setNerve(GET_NERVE(JumpSpider, HostTypeNrvHitBlow));
        return true;
    }

    return false;
}

bool JumpSpider::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgPlayerKick(msg) &&
        (isNerve(GET_NERVE(JumpSpider, HostTypeNrvStarPieceHit)) || isNerve(GET_NERVE(JumpSpider, HostTypeNrvStarPieceHitLand)) ||
         isNerve(GET_NERVE(JumpSpider, HostTypeNrvStarPieceHitWait)))) {
        mSpinHitController->start(this, *MR::getSensorPos(pSender), *MR::getSensorPos(pReceiver));
        setNerve(GET_NERVE(JumpSpider, HostTypeNrvHitBlow));
        return true;
    }

    if (isEatable()) {
        if (MR::isMsgYoshiStartTongue(msg)) {
            setNerve(GET_NERVE(JumpSpider, HostTypeNrvEaten));
            return true;
        }
    }

    if (isNerve(GET_NERVE(JumpSpider, HostTypeNrvEaten))) {
        if (MR::isMsgYoshiLoopTongue(msg)) {
            return true;
        }

        if (MR::isMsgYoshiReleaseEaten(msg)) {
            if (MR::isValidSwitchDead(this)) {
                MR::onSwitchDead(this);
            }

            makeActorDead();
            return true;
        }

        if (MR::isMsgYoshiCancelEaten(msg)) {
            if (MR::isValidSwitchDead(this)) {
                MR::onSwitchDead(this);
            }

            makeActorDead();
            return true;
        }
    }

    return false;
}

void JumpSpider::calcAndSetBaseMtx() {
    TPos3f pos;
    MR::calcMtxFromGravityAndZAxis(&pos, this, mGravity, mFront);
    MR::setBaseTRMtx(this, pos);
    MR::updateBaseScale(this, mScaleController);
}
