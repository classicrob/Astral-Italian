#pragma auto_inline off
#include "Game/Enemy/KoteBug.hpp"
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/Enemy/AnimStampController.hpp"
#include "Game/Enemy/SpinHitController.hpp"
#include "Game/Enemy/WalkerStateBindStarPointer.hpp"
#include "Game/LiveActor/Binder.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/ActorItemGenerator.hpp"
#include "Game/NPC/ScoreAttackCounter.hpp"
#include "Game/Util/YoshiLockOnUtil.hpp"
#include "Game/Util/JointController.hpp"

namespace {
    const f32 hCommonGravity = 1.5f;
    const f32 hGroundDamp = 0.8f;
    const f32 hAirDampH = 0.96f;
    const f32 hAirDampV = 0.93f;
    const f32 hPushedAcc = 2.0f;
    const f32 hSearchDistance = 1000.0f;
    const f32 hFlyDistance = 2000.0f;
    const f32 hFlyPursueVel = 0.2f;
    const f32 hFlyPursueLastSubVel = 0.01f;
    const f32 hFlyHeight = 100.0f;
    const s32 hFlyStartTime = 20;
    const s32 hFlyPursueTime = 200;
    const f32 hKeepAcc = 1.8f;
    const f32 hFreqRange = 40.0f;
    const f32 hOverturnFallJumpVelV = 30.0f;
    const s32 hOverturnTime = 240;
    const f32 hRecoverJumpVelV = 24.0f;
    const volatile f32 hDistNoCalcWait = 1000.0f;
    const s32 hSeachTime = 50;
    const s32 hEscapeSeachTime = 20;
    const f32 hPreEscapeTurnLimitDegree = 9.0f;
    const s32 hFlyPursueLastTime = 90;
    const f32 hFlyJitabataHeightBase = 60.0f;
    const f32 hFlyJitabataHeight = 150.0f;
    const f32 hJitabataPostureRate = 0.02f;
    const f32 hJitabataAngle = -1.8f;
    const f32 hFlyPursueLastAddRate = 1.0f;
    const f32 hHitToPlayerJumpVelV = 10.0f;
    const f32 hHitToPlayerJumpVelH = -10.0f;
    const f32 hHitScaleYSpeed = 0.1f;
    const s32 hStampDeathTime = 20;
    const s32 hBlowStopSceneTime = 3;
    const f32 hSearchTurnLimitDegree = 5.7f;
    const f32 hFlyTurnLimitDegree = 5.7f;
    const f32 hFlyEscapeTurnLimitDegree = 11.5f;
    const f32 hOverturnRotateRate = 0.15f;
    const f32 hRecoverRotateRate = 0.2f;
    const f32 hBlowRotateAngle = -1.0f;
};  // namespace

namespace NrvKoteBug {
    NEW_NERVE_ONEND(HostTypeNrvNoCalcWait, KoteBug, NoCalcWait, NoCalcWait);
    NEW_NERVE_ONEND(HostTypeNrvWait, KoteBug, Wait, Wait);
    class HostTypeNrvAppear : public Nerve {
    public:
        virtual void execute(Spine* spine) const { static_cast<KoteBug*>(spine->mExecutor)->fn_801B6650(); }
        static HostTypeNrvAppear sInstance;
    };
    HostTypeNrvAppear HostTypeNrvAppear::sInstance ATTRIBUTE_WEAK;
    NEW_NERVE(HostTypeNrvSearch, KoteBug, Search);
    NEW_NERVE(HostTypeNrvEscapeSearch, KoteBug, EscapeSearch);
    NEW_NERVE(HostTypeNrvPreEscape, KoteBug, PreEscape);
    NEW_NERVE(HostTypeNrvFlyStart, KoteBug, FlyStart);
    NEW_NERVE(HostTypeNrvFlyPursue, KoteBug, FlyPursue);
    NEW_NERVE(HostTypeNrvFlyEscape, KoteBug, FlyPursue);
    NEW_NERVE(HostTypeNrvFlyPursueLast, KoteBug, FlyPursueLast);
    NEW_NERVE(HostTypeNrvOverturnFall, KoteBug, OverturnFall);
    NEW_NERVE(HostTypeNrvOverturn, KoteBug, Overturn);
    NEW_NERVE(HostTypeNrvPreRecover, KoteBug, PreRecover);
    NEW_NERVE(HostTypeNrvRecover, KoteBug, Recover);
    NEW_NERVE(HostTypeNrvHitToPlayer, KoteBug, HitToPlayer);
    NEW_NERVE(HostTypeNrvBlow, KoteBug, Blow);
    NEW_NERVE(HostTypeNrvStampDeath, KoteBug, StampDeath);
    NEW_NERVE_ONEND(HostTypeNrvBindStarPointer, KoteBug, BindStarPointer, BindStarPointer);
    class HostTypeNrvEaten : public Nerve {
    public:
        virtual void execute(Spine*) const {}
        static HostTypeNrvEaten sInstance;
    };
    HostTypeNrvEaten HostTypeNrvEaten::sInstance ATTRIBUTE_WEAK;
};  // namespace NrvKoteBug

KoteBug::KoteBug(const char* pName)
    : LiveActor(pName), mScaleController(), mStampController(), mSpinHitController(), mStarPointer(), mFront(0, 0, 1), mPosture(0, 0, 0, 1),
      mYoshiLockOn(), _C0(0), _C4(false), mJointController() {
}

void KoteBug::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, "KoteBug", false);
    TPos3f mtx;
    mtx.set(getBaseMtx());
    mtx.getZDir(mFront);
    initNerve(GET_NERVE(KoteBug, HostTypeNrvWait), 1);
    mSpinHitController = new SpinHitController(this);
    mScaleController = new AnimScaleController(nullptr);
    mStampController = new AnimStampController();
    mStarPointer = new WalkerStateBindStarPointer(this, mScaleController);
    MR::initActorState(this, mStarPointer, GET_NERVE(KoteBug, HostTypeNrvBindStarPointer), "SupportFreeze");
    MR::onCalcGravity(this);
    MR::validateClipping(this);
    mJointController = MR::createJointDelegatorWithNullChildFunc(this, &KoteBug::calcJoint, "Center");
    mYoshiLockOn = MR::createYoshiLockOnTarget(this, (MtxPtr)nullptr, TVec3f(0.0f));
    if (MR::useStageSwitchSyncAppear(this, rIter)) setNerve(GET_NERVE(KoteBug, HostTypeNrvAppear));
}

void KoteBug::appear() {
    LiveActor::appear();
    MR::emitEffect(this, "Appear");
}

void KoteBug::kill() {
    LiveActor::kill();
    MR::emitEffect(this, "Death");
}

void KoteBug::control() {
    if (MR::isInDeath(this, TVec3f(0.0f, 0.0f, 0.0f)) || MR::isInWater(mPosition)) {
        kill();
        return;
    }

    mScaleController->updateNerve();
    mStampController->updateNerve();
    if (_C0 > 0) _C0--;
    if (fn_801B7B40()) {
        TQuat4f rotation;
        rotation.setRotate(TVec3f(1, 0, 0), 3.1415927f);
        mPosture.slerp(rotation, ::hOverturnRotateRate);
    } else if (isNerve(GET_NERVE(KoteBug, HostTypeNrvFlyPursueLast))) {
        TQuat4f rotation;
        rotation.setRotate(TVec3f(1, 0, 0), ::hJitabataAngle);
        mPosture.slerp(rotation, ::hJitabataPostureRate);
    } else {
        mPosture.slerp(TQuat4f(0, 0, 0, 1), ::hRecoverRotateRate);
    }

    if (fn_801B7D60()) return;
}

void KoteBug::fn_801B6650() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "PreEscape");
        MR::startActionSound(this, "EmKotebugFind");
    }
    MR::turnDirectionToPlayerDegreeHorizon(this, &mFront, 5.7f);
    MR::zeroVelocity(this);
    if (MR::isActionEnd(this)) {
        bool escape = !MR::isPlayerElementModeBee();
        if (escape) { setNerve(GET_NERVE(KoteBug, HostTypeNrvFlyEscape)); return; }
        setNerve(GET_NERVE(KoteBug, HostTypeNrvSearch));
    }
}

void KoteBug::exeNoCalcWait() {
    if (MR::isFirstStep(this)) {
        MR::validateClipping(this);
        MR::offBind(this);
        MR::offCalcShadow(this, nullptr);
        MR::offCalcGravity(this);
    }

    MR::zeroVelocity(this);
    if (MR::calcDistanceToPlayer(mPosition) < ::hDistNoCalcWait) {
        setNerve(GET_NERVE(KoteBug, HostTypeNrvWait));
    }
}

void KoteBug::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::validateClipping(this);
        MR::startAction(this, "Wait");
    }
    YoshiFunction::tryYoshiEat(mYoshiLockOn);
    f32 distance = MR::calcDistanceToPlayer(this);
    if (MR::isOnGround(this) && distance < hSearchDistance) {
        bool escape = !MR::isPlayerElementModeBee();
        if (escape) { setNerve(GET_NERVE(KoteBug, HostTypeNrvEscapeSearch)); return; }
        setNerve(GET_NERVE(KoteBug, HostTypeNrvSearch));
        return;
    }
    if (MR::isOnGround(this)) MR::zeroVelocity(this);
    MR::applyVelocityDampAndGravity(this, hCommonGravity, hGroundDamp, hAirDampH, hAirDampV, 1.0f);
}

void KoteBug::exeSearch() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Syaka");
    }

    MR::turnDirectionToPlayerDegreeHorizon(this, &mFront, ::hSearchTurnLimitDegree);
    YoshiFunction::tryYoshiEat(mYoshiLockOn);
    if (MR::isGreaterStep(this, ::hSeachTime)) {
        setNerve(GET_NERVE(KoteBug, HostTypeNrvFlyPursue));
    } else if (::hSearchDistance < MR::calcDistanceToPlayer(this)) {
        setNerve(GET_NERVE(KoteBug, HostTypeNrvWait));
    } else {
        MR::applyVelocityDampAndGravity(this, ::hCommonGravity, ::hGroundDamp, ::hAirDampH, ::hAirDampV, 1.0f);
    }
}

void KoteBug::exeEscapeSearch() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Syaka");
    }

    MR::turnDirectionToPlayerDegreeHorizon(this, &mFront, ::hPreEscapeTurnLimitDegree);
    YoshiFunction::tryYoshiEat(mYoshiLockOn);
    if (MR::isGreaterStep(this, ::hEscapeSeachTime)) {
        bool escape = !MR::isPlayerElementModeBee();
        if (escape) {
            setNerve(GET_NERVE(KoteBug, HostTypeNrvPreEscape));
        } else {
            setNerve(GET_NERVE(KoteBug, HostTypeNrvFlyPursue));
        }
    } else if (::hSearchDistance < MR::calcDistanceToPlayer(this)) {
        setNerve(GET_NERVE(KoteBug, HostTypeNrvWait));
    } else {
        MR::applyVelocityDampAndGravity(this, ::hCommonGravity, ::hGroundDamp, ::hAirDampH, ::hAirDampV, 1.0f);
    }
}

void KoteBug::exePreEscape() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "PreEscape");
        MR::startActionSound(this, "EmKotebugFind");
    }

    MR::applyVelocityDampAndGravity(this, ::hCommonGravity, ::hGroundDamp, ::hAirDampH, ::hAirDampV, 1.0f);
    MR::turnDirectionToPlayerDegreeHorizon(this, &mFront, ::hPreEscapeTurnLimitDegree);
    YoshiFunction::tryYoshiEat(mYoshiLockOn);
    if (MR::isActionEnd(this)) {
        setNerve(GET_NERVE(KoteBug, HostTypeNrvFlyEscape));
    }
}

void KoteBug::exeFlyStart() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Wait");
    }

    MR::startActionSound(this, "EmLvKotebugFly");
    MR::applyVelocityDampAndGravity(this, ::hCommonGravity, ::hGroundDamp, ::hAirDampH, ::hAirDampV, 1.0f);
    MR::turnDirectionToPlayerDegreeHorizon(this, &mFront, ::hFlyTurnLimitDegree);
    YoshiFunction::tryYoshiEat(mYoshiLockOn);
    if (MR::isGreaterStep(this, ::hFlyStartTime)) {
        setNerve(GET_NERVE(KoteBug, HostTypeNrvFlyPursue));
    }
}

void KoteBug::exeFlyPursue() {
    if (MR::isFirstStep(this)) {
        if (isNerve(GET_NERVE(KoteBug, HostTypeNrvFlyEscape))) {
            MR::startAction(this, "Jitabata");
        } else {
            MR::startAction(this, "Fly");
        }
    }

    MR::startActionSound(this, "EmLvKotebugFly");
    MR::applyVelocityDampAndGravity(this, ::hCommonGravity, ::hGroundDamp, ::hAirDampH, ::hAirDampV, 1.0f);
    if (isNerve(GET_NERVE(KoteBug, HostTypeNrvFlyEscape))) {
        MR::turnDirectionFromPlayerDegreeHorizon(this, &mFront, ::hFlyEscapeTurnLimitDegree);
    } else {
        MR::turnDirectionToPlayerDegreeHorizon(this, &mFront, ::hFlyTurnLimitDegree);
    }
    YoshiFunction::tryYoshiEat(mYoshiLockOn);

    mVelocity.add(mFront * ::hFlyPursueVel);
    if (MR::isShadowProjected(this, nullptr)) {
        MR::addVelocityKeepHeightUseShadow(this, ::hFlyHeight, 0.0f, ::hKeepAcc, ::hFreqRange, nullptr);
    }

    bool escape = !MR::isPlayerElementModeBee();
    if (escape) {
        if (isNerve(GET_NERVE(KoteBug, HostTypeNrvFlyPursue))) {
            setNerve(GET_NERVE(KoteBug, HostTypeNrvFlyEscape));
            return;
        }
    } else if (isNerve(GET_NERVE(KoteBug, HostTypeNrvFlyEscape))) {
        setNerve(GET_NERVE(KoteBug, HostTypeNrvFlyPursue));
        return;
    }

    if (::hFlyDistance < MR::calcDistanceToPlayer(this)) {
        setNerve(GET_NERVE(KoteBug, HostTypeNrvWait));
    } else if (isNerve(GET_NERVE(KoteBug, HostTypeNrvFlyEscape)) && MR::isGreaterStep(this, ::hFlyPursueTime)) {
        setNerve(GET_NERVE(KoteBug, HostTypeNrvFlyPursueLast));
    }
}

void KoteBug::exeFlyPursueLast() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Jitabata");
    }

    MR::startActionSound(this, "EmLvKotebugPursueLast");
    MR::applyVelocityDampAndGravity(this, ::hCommonGravity, ::hGroundDamp, ::hAirDampH, ::hAirDampV, 1.0f);
    f32 rate = getNerveStep() / f32(::hFlyPursueLastTime);
    MR::setBckRate(this, ::hFlyPursueLastAddRate + rate);
    mVelocity.add(mFront * (::hFlyPursueVel - ::hFlyPursueLastSubVel * rate));
    if (MR::isShadowProjected(this, nullptr)) {
        MR::addVelocityKeepHeightUseShadow(this, ::hFlyJitabataHeightBase + ::hFlyJitabataHeight * rate, 0.0f, ::hKeepAcc, ::hFreqRange, nullptr);
    }

    YoshiFunction::tryYoshiEat(mYoshiLockOn);
    if (::hFlyDistance < MR::calcDistanceToPlayer(this)) {
        setNerve(GET_NERVE(KoteBug, HostTypeNrvWait));
    } else if (MR::isGreaterStep(this, ::hFlyPursueLastTime)) {
        setNerve(GET_NERVE(KoteBug, HostTypeNrvOverturnFall));
    }
}

void KoteBug::exeOverturnFall() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Pote");
        MR::startActionSound(this, "EmKotebugOverturnFall");
        MR::addVelocityJump(this, ::hOverturnFallJumpVelV);
    }

    MR::applyVelocityDampAndGravity(this, ::hCommonGravity, ::hGroundDamp, ::hAirDampH, ::hAirDampV, 1.0f);
    YoshiFunction::tryYoshiEat(mYoshiLockOn);
    if (MR::isOnGroundCos(this, 0.75f)) {
        mScaleController->startAndAddScaleVelocityY(::hHitScaleYSpeed);
        mScaleController->startAnim();
        setNerve(GET_NERVE(KoteBug, HostTypeNrvOverturn));
        MR::emitEffect(this, "PoteSmoke");
        MR::startActionSound(this, "EmKotebugFallLand");
    }
}

void KoteBug::exeOverturn() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Jitabata2");
    }

    MR::startActionSound(this, "EmLvKotebugOverturn");
    MR::applyVelocityDampAndGravity(this, ::hCommonGravity, ::hGroundDamp, ::hAirDampH, ::hAirDampV, 1.0f);
    MR::killVelocityOnGroundH(this);
    mBinder->_1EC._5 = true;
    YoshiFunction::tryYoshiEat(mYoshiLockOn);
    if (MR::isGreaterStep(this, ::hOverturnTime)) {
        setNerve(GET_NERVE(KoteBug, HostTypeNrvPreRecover));
    }
}

void KoteBug::exePreRecover() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Gu");
    }

    MR::reboundVelocityFromCollision(this, 0.0f, 0.0f, 1.0f);
    MR::applyVelocityDampAndGravity(this, ::hCommonGravity, ::hGroundDamp, ::hAirDampH, ::hAirDampV, 1.0f);
    MR::killVelocityOnGroundH(this);
    YoshiFunction::tryYoshiEat(mYoshiLockOn);
    if (MR::isActionEnd(this)) {
        setNerve(GET_NERVE(KoteBug, HostTypeNrvRecover));
    }
}

void KoteBug::exeRecover() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Pon");
        MR::startActionSound(this, "EmKotebugRecover");
        MR::addVelocityJump(this, ::hRecoverJumpVelV);
    }

    MR::reboundVelocityFromCollision(this, 0.0f, 0.0f, 1.0f);
    YoshiFunction::tryYoshiEat(mYoshiLockOn);
    MR::applyVelocityDampAndGravity(this, ::hCommonGravity, ::hGroundDamp, ::hAirDampH, ::hAirDampV, 1.0f);
    if (MR::isOnGround(this)) {
        MR::startActionSound(this, "EmKotebugLand");
        mScaleController->startAndAddScaleVelocityY(::hHitScaleYSpeed);
        mScaleController->startAnim();
        setNerve(GET_NERVE(KoteBug, HostTypeNrvWait));
    }
}

void KoteBug::exeHitToPlayer() {
    if (MR::isFirstStep(this)) {
        mScaleController->startAndAddScaleVelocityY(::hHitScaleYSpeed);
        mScaleController->startAnim();
        MR::startAction(this, "Wait");
        MR::addVelocityJump(this, ::hHitToPlayerJumpVelV);
        mVelocity.add(mFront * ::hHitToPlayerJumpVelH);
    }

    MR::reboundVelocityFromCollision(this, 0.0f, 0.0f, 1.0f);
    MR::applyVelocityDampAndGravity(this, ::hCommonGravity, ::hGroundDamp, ::hAirDampH, ::hAirDampV, 1.0f);
    YoshiFunction::tryYoshiEat(mYoshiLockOn);
    if (MR::isOnGround(this)) {
        setNerve(GET_NERVE(KoteBug, HostTypeNrvFlyStart));
    }
}

void KoteBug::exeBlow() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Fly");
        MR::startBlowHitSound(this);
        MR::stopScene(::hBlowStopSceneTime);
        MR::onCalcShadow(this, nullptr);
        MR::onCalcGravity(this);
    }

    TQuat4f rotation;
    rotation.setRotate(TVec3f(1, 0, 0), ::hBlowRotateAngle);
    PSQUATMultiply(mPosture, rotation, mPosture);

    if (mSpinHitController->execute(this)) {
        MR::emitItemHit(this);
        MR::startActionSound(this, "EmExplodeS");
    }
}

void KoteBug::exeStampDeath() {
    if (MR::isFirstStep(this)) {
        MR::stopBck(this);
        MR::invalidateClipping(this);
        MR::zeroVelocity(this);
        mStampController->startAnim(mGravity);
        MR::setBinderRadius(this, 10.0f);
        MR::forceBindOnGround(this, 0.0f, 1.0f);
    }

    mBinder->_1EC._5 = true;
    if (MR::isGreaterStep(this, ::hStampDeathTime)) {
        MR::startActionSound(this, "EmExplodeS");
        MR::emitItemStomp(this);
        kill();
    }
}

void KoteBug::exeBindStarPointer() {
    YoshiFunction::tryYoshiEat(mYoshiLockOn);

    if (MR::updateActorState(this)) {
        if (_C4) {
            setNerve(GET_NERVE(KoteBug, HostTypeNrvRecover));
        } else {
            setNerve(GET_NERVE(KoteBug, HostTypeNrvWait));
        }
    }
}

bool KoteBug::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(GET_NERVE(KoteBug, HostTypeNrvAppear)) || isNerve(GET_NERVE(KoteBug, HostTypeNrvBlow)) || isNerve(GET_NERVE(KoteBug, HostTypeNrvStampDeath))) {
        return false;
    }

    if (MR::isMsgPlayerHipDrop(msg)) {
        MR::startActionSound(this, "EmStompedS");
        ScoreAttackFunction::scoreByAttack(this, msg, nullptr);
        setNerve(GET_NERVE(KoteBug, HostTypeNrvStampDeath));
        return true;
    }

    if (MR::isMsgPlayerTrample(msg)) {
        if (isNerve(GET_NERVE(KoteBug, HostTypeNrvOverturnFall))) {
            return false;
        }

        if (isNerve(GET_NERVE(KoteBug, HostTypeNrvOverturn)) || isNerve(GET_NERVE(KoteBug, HostTypeNrvRecover)) || (isNerve(GET_NERVE(KoteBug, HostTypeNrvBindStarPointer)) && _C4)) {
            MR::startActionSound(this, "EmStompedS");
            ScoreAttackFunction::scoreByAttack(this, msg, nullptr);
            setNerve(GET_NERVE(KoteBug, HostTypeNrvStampDeath));
            return true;
        }

        MR::startActionSound(this, "EmKotebugTrample");
        mVelocity.add(mFront * -10.0f);
        setNerve(GET_NERVE(KoteBug, HostTypeNrvOverturnFall));
        return true;
    }

    if (MR::isMsgLockOnStarPieceShoot(msg)) {
        if (isNerve(GET_NERVE(KoteBug, HostTypeNrvOverturnFall)) || isNerve(GET_NERVE(KoteBug, HostTypeNrvOverturn))) {
            return false;
        }

        return true;
    }

    if (MR::isMsgStarPieceAttack(msg)) {
        if (isNerve(GET_NERVE(KoteBug, HostTypeNrvOverturnFall)) || isNerve(GET_NERVE(KoteBug, HostTypeNrvOverturn))) {
            MR::addVelocityJump(this, ::hOverturnFallJumpVelV);
            return true;
        }

        setOverturnFall(pSender, pReceiver);
        return true;
    }

    if (MR::isMsgInvincibleAttack(msg) || MR::isMsgFireBallAttack(msg) || MR::isMsgYoshiProjectile(msg) || MR::isMsgYoshiEat(msg)) {
        mSpinHitController->start(this, (*MR::getSensorPos(pSender)), (*MR::getSensorPos(pReceiver)));
        ScoreAttackFunction::scoreByAttack(this, msg, nullptr);
        setNerve(GET_NERVE(KoteBug, HostTypeNrvBlow));
        return true;
    }

    if (MR::isMsgPlayerHitAll(msg)) {
        if (isNerve(GET_NERVE(KoteBug, HostTypeNrvOverturnFall)) || isNerve(GET_NERVE(KoteBug, HostTypeNrvOverturn)) || (isNerve(GET_NERVE(KoteBug, HostTypeNrvBindStarPointer)) && _C4)) {
            if (_C0 == 0) {
                mSpinHitController->start(this, (*MR::getSensorPos(pSender)), (*MR::getSensorPos(pReceiver)));
                ScoreAttackFunction::scoreByAttack(this, msg, nullptr);
                setNerve(GET_NERVE(KoteBug, HostTypeNrvBlow));
                return true;
            }
            return false;
        }

        _C0 = 30;
        setOverturnFall(pSender, pReceiver);
        return true;
    }

    return false;
}

bool KoteBug::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(GET_NERVE(KoteBug, HostTypeNrvAppear)) || isNerve(GET_NERVE(KoteBug, HostTypeNrvBlow)) || isNerve(GET_NERVE(KoteBug, HostTypeNrvStampDeath))) return false;

    if (MR::isMsgInhaleBlackHole(msg)) {
        kill();
        return true;
    }

    if (MR::isMsgPlayerKick(msg)) {
        if (MR::isPlayerExistUp(this, MR::getSensorRadius(this, "Body"), 0.25f)) {
            return false;
        }

        if (isNerve(GET_NERVE(KoteBug, HostTypeNrvOverturn)) || isNerve(GET_NERVE(KoteBug, HostTypeNrvPreRecover)) ||
            isNerve(GET_NERVE(KoteBug, HostTypeNrvRecover)) || (isNerve(GET_NERVE(KoteBug, HostTypeNrvBindStarPointer)) && _C4)) {
            mSpinHitController->start(this, (*MR::getSensorPos(pSender)), (*MR::getSensorPos(pReceiver)));
            ScoreAttackFunction::scoreByAttack(this, msg, nullptr);
            setNerve(GET_NERVE(KoteBug, HostTypeNrvBlow));
            return true;
        }
    }

    if (MR::isMsgYoshiStartTongue(msg)) { setNerve(GET_NERVE(KoteBug, HostTypeNrvEaten)); return true; }
    if (MR::isMsgYoshiLoopTongue(msg)) return true;
    if (MR::isMsgYoshiReleaseEaten(msg)) { makeActorDead(); return true; }
    if (MR::isMsgYoshiCancelEaten(msg)) { MR::startActionSound(this, "EmExplodeS"); makeActorDead(); return true; }
    return false;
}

bool KoteBug::fn_801B7B40() const {
    if (isNerve(GET_NERVE(KoteBug, HostTypeNrvOverturn)) || isNerve(GET_NERVE(KoteBug, HostTypeNrvOverturnFall)) || isNerve(GET_NERVE(KoteBug, HostTypeNrvPreRecover)) || isNerve(GET_NERVE(KoteBug, HostTypeNrvStampDeath))) return true;
    if (isNerve(GET_NERVE(KoteBug, HostTypeNrvBindStarPointer))) return _C4;
    return false;
}

bool KoteBug::fn_801B7BE0() const {
    if (isNerve(GET_NERVE(KoteBug, HostTypeNrvNoCalcWait)) || isNerve(GET_NERVE(KoteBug, HostTypeNrvWait)) || isNerve(GET_NERVE(KoteBug, HostTypeNrvSearch)) || isNerve(GET_NERVE(KoteBug, HostTypeNrvEscapeSearch)) || isNerve(GET_NERVE(KoteBug, HostTypeNrvPreEscape)) || isNerve(GET_NERVE(KoteBug, HostTypeNrvFlyStart)) || isNerve(GET_NERVE(KoteBug, HostTypeNrvFlyPursue)) || isNerve(GET_NERVE(KoteBug, HostTypeNrvFlyEscape)) || isNerve(GET_NERVE(KoteBug, HostTypeNrvOverturn)) || isNerve(GET_NERVE(KoteBug, HostTypeNrvPreRecover)) || isNerve(GET_NERVE(KoteBug, HostTypeNrvRecover))) return true;
    return false;
}

bool KoteBug::fn_801B7CF0() const {
    if (isNerve(GET_NERVE(KoteBug, HostTypeNrvOverturn)) || isNerve(GET_NERVE(KoteBug, HostTypeNrvPreRecover)) || isNerve(GET_NERVE(KoteBug, HostTypeNrvRecover))) return true;
    return false;
}

bool KoteBug::fn_801B7D60() {
    if (fn_801B7BE0() && mStarPointer->tryStartPointBind()) {
        _C4 = fn_801B7CF0();
        setNerve(GET_NERVE(KoteBug, HostTypeNrvBindStarPointer));
        return true;
    }
    return false;
}

void KoteBug::setOverturnFall(HitSensor* pSender, HitSensor* pReceiver) {
    MR::startActionSound(this, "EmKotebugTrample");

    TVec3f direction((*MR::getSensorPos(pReceiver)));
    direction -= (*MR::getSensorPos(pSender));
    MR::vecKillElement(direction, mGravity, &direction);
    MR::normalizeOrZero(&direction);
    mVelocity.add(direction * 10.0f);

    setNerve(GET_NERVE(KoteBug, HostTypeNrvOverturnFall));
}

bool KoteBug::receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(GET_NERVE(KoteBug, HostTypeNrvAppear)) || isNerve(GET_NERVE(KoteBug, HostTypeNrvBlow)) || isNerve(GET_NERVE(KoteBug, HostTypeNrvStampDeath))) {
        return false;
    }

    if (MR::isMsgExplosionAttack(msg)) {
        mSpinHitController->start(this, (*MR::getSensorPos(pSender)), (*MR::getSensorPos(pReceiver)));
        setNerve(GET_NERVE(KoteBug, HostTypeNrvBlow));
        return true;
    }

    if (MR::isMsgToEnemyAttackBlow(msg)) {
        mSpinHitController->start(this, (*MR::getSensorPos(pSender)), (*MR::getSensorPos(pReceiver)));
        setNerve(GET_NERVE(KoteBug, HostTypeNrvBlow));
        return true;
    }

    if (MR::isMsgToEnemyAttackTrample(msg)) {
        setNerve(GET_NERVE(KoteBug, HostTypeNrvStampDeath));
        return true;
    }

    return false;
}

// NONMATCHING: compiler folds the target eaten-state branch into the common exit.
void KoteBug::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (!isNerve(GET_NERVE(KoteBug, HostTypeNrvAppear)) && !isNerve(GET_NERVE(KoteBug, HostTypeNrvBlow)) && !isNerve(GET_NERVE(KoteBug, HostTypeNrvStampDeath))) {
        if (isNerve(GET_NERVE(KoteBug, HostTypeNrvEaten))) return;
        if (pSender != getSensor("Body")) {
            return;
        }

        if (MR::isSensorPlayer(pReceiver)) {
            if (MR::isPlayerHipDropFalling() || MR::isPlayerHipDropLand()) {
                return;
            }

            if (isNerve(GET_NERVE(KoteBug, HostTypeNrvFlyStart)) || isNerve(GET_NERVE(KoteBug, HostTypeNrvFlyPursue))) {
                if (MR::isPlayerExistUp(this, MR::getSensorRadius(this, "Body"), 0.25f)) {
                    MR::sendMsgPush(pReceiver, pSender);
                    return;
                }

                if (MR::sendMsgEnemyAttack(pReceiver, pSender) == true) {
                    setNerve(GET_NERVE(KoteBug, HostTypeNrvHitToPlayer));
                    return;
                }
            }

            MR::sendMsgPushAndKillVelocityToTarget(this, pReceiver, pSender);
        } else if (MR::isSensorEnemy(pReceiver) || MR::isSensorYoshi(pReceiver)) {
            MR::sendMsgPush(pReceiver, pSender);
        }
    }
}

bool KoteBug::receiveMsgPush(HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(GET_NERVE(KoteBug, HostTypeNrvAppear)) || isNerve(GET_NERVE(KoteBug, HostTypeNrvBlow)) || isNerve(GET_NERVE(KoteBug, HostTypeNrvStampDeath)) ||
        isNerve(GET_NERVE(KoteBug, HostTypeNrvBindStarPointer))) {
        return false;
    }

    if (MR::isSensorEnemy(pSender) || MR::isSensorMapObj(pSender)) {
        MR::addVelocityFromPushHorizon(this, ::hPushedAcc, pSender, pReceiver);
        return true;
    }

    return false;
}

void KoteBug::calcAndSetBaseMtx() {
    TPos3f mtx;
    MR::calcMtxFromGravityAndZAxis(&mtx, this, mGravity, mFront);
    TPos3f stampMtx(mStampController->_14);
    mtx.concat(stampMtx, mtx);
    mtx.setTrans(mPosition);
    MR::setBaseTRMtx(this, mtx);

    MR::updateBaseScale(this, mScaleController);
    mJointController->registerCallBack();
}

bool KoteBug::calcJoint(TPos3f* pMtx, const JointControllerInfo& rInfo) {
    TPos3f rotation;
    rotation.makeQuat(mPosture);
    pMtx->concat(*pMtx, rotation);
    return true;
}

void KoteBug::endNoCalcWait() {
    MR::onBind(this);
    MR::onCalcShadow(this, nullptr);
    MR::onCalcGravity(this);
}

void KoteBug::endWait() {
    MR::invalidateClipping(this);
}

void KoteBug::endBindStarPointer() {
    mStarPointer->kill();
}

