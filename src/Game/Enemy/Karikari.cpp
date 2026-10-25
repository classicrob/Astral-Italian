#include "Game/Enemy/Karikari.hpp"
#include "Game/Util.hpp"
#include "Game/Enemy/KarikariDirector.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/Util/ActorMovementUtil.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/ActorShadowUtil.hpp"
#include "Game/Util/ActorSwitchUtil.hpp"
#include "Game/Util/CameraUtil.hpp"
#include "Game/Util/EffectUtil.hpp"
#include "Game/Util/FixedPosition.hpp"
#include "Game/Util/GamePadUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include "Game/Util/NativeActorObserve.hpp"
#include "Game/Util/StarPointerUtil.hpp"
#include "Game/Util/SupportPlayUtil.hpp"
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/Enemy/WalkerStateBindStarPointer.hpp"
#include <JSystem/JGeometry/TUtil.hpp>

namespace {
    volatile static const f32 sSize = 30.0f;  // TODO: is this really volatile or is this just a hack

    static const f32 sGravity = 2.4f;
    static const f32 sTurnRatio = 0.25f;
    // static const f32 sTornadoPower = ; // unused
    // static const f32 sTornadoUpPower = ; // unused
    // static const f32 sTornadoTangentPower = ; // unused
    static const f32 sVelocityDampGround = 0.8f;
    static const f32 sVelocityDampAir = 0.98f;
    static const f32 sStopVelocitySquared = 100.0f;
    static const s32 sJumpAgainTime = 11;
    static const s32 sStopSceneTime = 5;
    static const f32 sScratchStrongVel = 18.0f;
    static const f32 sFromCursorVelScale = 5.0f;
    static const f32 sJumpToPrePursueVel = 50.0f;
    static const f32 sJumpToPursueVel = 33.0f;
    static const f32 sPursueVel = 10.0f;
    static const f32 sJumpAnimRate = 1.0f;
    static const f32 sDistToPursue = 1000.0f;
    static const f32 sWatchForGravity = 1.4f;
    static const f32 sWatchForJumpVel = 35.0f;
    static const f32 sWatchForDistance = 500.0f;
    static const s32 sWatchForJumpTime = 7;
    static const s32 sWatchForLoopInterval = 45;
    static const s32 sPreClingTime = 3;
    static const f32 sDistToCling = 80.0f;
    static const s32 sSpinAttackedTime = 8;
    static const f32 sVelocityDampSpinAttacked = 0.9f;
    // static const f32 sSpinAttackedVel = ;
    // static const f32 sSpinAttackedVerticalVel = ;
    static const s32 sBlowOutTime = 60;
    static const f32 sBlowOutGravity = 1.3f;
    static const f32 sBlowOutVel = 10.0f;
    static const f32 sBlowOutVerticalVel = 38.0f;
    static const f32 sDPDAttackedGravity = 2.4f;
    static const f32 sVelocityDampAttacked = 0.9f;
    static const f32 sAttackedVel = 43.0f;
    static const f32 sAttackedVerticalVel = 33.0f;
    static const s32 sFrozenLimitTime = 180;
    // static const f32 sSpinFrozenAttackedVel = ;
    // static const f32 sFrozenAttackedBySpinRadius = ;
    // static const f32 sDeathEffectScale = ;
    // static const f32 sIceShadowSize = ;
    static const f32 sFrozenRecoverGravity = 2.4f;
    static const f32 sJumpToFrozenRecoverVel = 40.0f;
    static const s32 sPreWaitTime = 90;
    static const s32 sBeginWaitMotionFrame = 20;
    static const s32 sPressTime = 40;
    static const f32 sExplosionBlowOutVel = 30.0f;
    static const f32 sEnemyAttackHitVel = 30.0f;
    static const f32 sSpinXAnimRate = 2.0f;

    static const f32 sGravitySpinAttacked = 0.0f;
    static const s32 sIceAppearTime = 0;

    KarikariDirector* getKarikariDirector() {
        return MR::getSceneObj< KarikariDirector >(SceneObj_KarikariDirector);
    }
};  // namespace

namespace NrvKarikari {
    NEW_NERVE(HostTypeNrvFall, Karikari, Fall);
    NEW_NERVE(HostTypeNrvLand, Karikari, Land);
    NEW_NERVE(HostTypeNrvNoCalcWait, Karikari, NoCalcWait);
    NEW_NERVE(HostTypeNrvWait, Karikari, Wait);
    NEW_NERVE(HostTypeNrvPrePursue, Karikari, PrePursue);
    NEW_NERVE(HostTypeNrvPursue, Karikari, Pursue);
    NEW_NERVE(HostTypeNrvWatchFor, Karikari, WatchFor);
    NEW_NERVE(HostTypeNrvPreCling, Karikari, PreCling);
    NEW_NERVE(HostTypeNrvCling, Karikari, Cling);
    NEW_NERVE(HostTypeNrvRelease, Karikari, Release);
    NEW_NERVE(HostTypeNrvSpinAttacked, Karikari, SpinAttacked);
    NEW_NERVE(HostTypeNrvDPDAttacked, Karikari, DPDAttacked);
    NEW_NERVE(HostTypeNrvFrozen, Karikari, Frozen);
    NEW_NERVE(HostTypeNrvFrozenRecover, Karikari, FrozenRecover);
    NEW_NERVE(HostTypeNrvBlowOut, Karikari, BlowOut);
    NEW_NERVE(HostTypeNrvBlowOutStarPiece, Karikari, BlowOut);
    NEW_NERVE(HostTypeNrvInTornado, Karikari, InTornado);
    NEW_NERVE(HostTypeNrvPress, Karikari, Press);
};  // namespace NrvKarikari

Karikari::Karikari(const char* pName)
    : LiveActor(pName), mFront(0.0f, 0.0f, 1.0f), mPoseQuat(0.0f, 0.0f, 0.0f, 1.0f), mClingPosition(nullptr), mAnimScale(nullptr), mStarPointer(nullptr),
      mIsAttackEnabled(true), mIsPushable(true), mFrozenTime(0) {
}

void Karikari::init(const JMapInfoIter& rIter) {
    MR::createSceneObj(SceneObj_KarikariDirector);
    MR::joinToGroup(this, "カリカリディレクター");
    MR::initActor(this, rIter, "Karipon", false);
    MR::makeQuatAndFrontFromRotate(&mPoseQuat, &mFront, this);
    initNerve(GET_NERVE(Karikari, HostTypeNrvFall), 1);

    mAnimScale = new AnimScaleController(nullptr);
    mStarPointer = new WalkerStateBindStarPointer(this, mAnimScale);
    MR::initActorState(this, mStarPointer, GET_NERVE(Karikari, HostTypeNrvPress), "SupportFreeze");
    MR::declareStarPiece(this, 3);
    MR::useStageSwitchSyncAppear(this, rIter);
}

void Karikari::applyEnvironmentInfluenceToVelocity() {
    if (MR::isOnGround(this)) {
        MR::reboundVelocityFromCollision(this, 0.0f, 0.0f, 1.0f);
        mVelocity.mult(::sVelocityDampGround);
        TVec3f norm(MR::getGroundNormal(this));
        TVec3f velH(mVelocity);
        MR::vecKillElement(velH, norm, &velH);
        if (velH.squared() < ::sStopVelocitySquared) {
            mVelocity.scale(norm.dot(mVelocity), norm);
        }
        return;
    }

    if (isNerve(GET_NERVE(Karikari, HostTypeNrvSpinAttacked))) {
        mVelocity.mult(::sVelocityDampSpinAttacked);
        return;
    }

    mVelocity.mult(::sVelocityDampAir);
}

// NONMATCHING: vector temporary stack-slot order.
namespace {
    inline TVec3f calcKarikariStarPiecePosition(const TVec3f& up, const TVec3f& position) {
        TVec3f pos(up);
        pos.scale(80.0f);
        pos += position;
        return pos;
    }
}

void Karikari::killedInFrozenState() {
    MR::deleteEffect(this, "KarikariCrystalLight");
    MR::emitEffect(this, "KarikariCrystalBreak");
    MR::startActionSound(this, "EmKarikariBlow");
    MR::appearStarPiece(this, calcKarikariStarPiecePosition(-mGravity, mPosition), 3, 10.0f, 40.0f, false);
    makeActorDead();
    MR::stopScene(::sStopSceneTime);
}

void Karikari::control() {
    mAnimScale->updateNerve();
    MR::blendQuatFromGroundAndFront(&mPoseQuat, this, mFront, 0.05f, 0.5f);

    if (MR::isInDeath(this, TVec3f(0.0f, 0.0f, 0.0f)) || MR::isInWater(mPosition)) {
        kill();
    }
}

void Karikari::exeFall() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Wait");
        mIsAttackEnabled = false;
        mIsPushable = true;
        MR::onBind(this);
        MR::onCalcShadow(this, nullptr);
        MR::onCalcGravity(this);
        MR::startBtp(this, "Bite");
        MR::setBtpFrameAndStop(this, 0.0f);
    }

    tryStarPointerBind();
    applyEnvironmentInfluenceToVelocity();
    TVec3f gravity(mGravity);
    gravity.scale(::sGravity);
    mVelocity += gravity;

    if (MR::isOnGround(this)) {
        MR::emitEffect(this, "SmokeLand");
        setNerve(GET_NERVE(Karikari, HostTypeNrvLand));
    }
}

void Karikari::exeLand() {
    applyEnvironmentInfluenceToVelocity();
    mVelocity += mGravity * ::sGravity;

    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Land");
        MR::startBtp(this, "Land");
        mIsAttackEnabled = false;
        mIsPushable = true;
        MR::onBind(this);
        MR::onCalcShadow(this, nullptr);
        MR::onCalcGravity(this);
        mVelocity.zero();
    }

    tryStarPointerBind();

    if (MR::isGreaterStep(this, ::sPreWaitTime)) {
        TVec3f diff(*MR::getPlayerPos());
        diff.sub(mPosition);
        if (diff.squared() < ::sDistToPursue * ::sDistToPursue) {
            setNerve(GET_NERVE(Karikari, HostTypeNrvPursue));
            return;
        } else {
            setNerve(GET_NERVE(Karikari, HostTypeNrvWait));
            return;
        }
    }

    if (MR::isStep(this, ::sBeginWaitMotionFrame)) {
        MR::startAction(this, "Wait");
        MR::startBtp(this, "Bite");
        MR::setBtpFrameAndStop(this, 0.0f);
        return;
    }

    if (MR::isGreaterStep(this, ::sBeginWaitMotionFrame)) {
        TVec3f toPlayerH(*MR::getPlayerPos() - mPosition);
        MR::vecKillElement(toPlayerH, mGravity, &toPlayerH);
        MR::normalizeOrZero(&toPlayerH);
        tryTurnToDirection(toPlayerH, ::sTurnRatio);
    }
}

void Karikari::exeNoCalcWait() {
    if (MR::isFirstStep(this)) {
        MR::offBind(this);
        MR::offCalcShadow(this, nullptr);
        MR::offCalcGravity(this);
        mIsAttackEnabled = false;
        mIsPushable = false;
    }

    mVelocity.zero();
    if (tryStarPointerBind()) {
        return;
    }

    if (MR::calcDistanceToPlayer(mPosition) < ::sDistToPursue) {
        setNerve(GET_NERVE(Karikari, HostTypeNrvWait));
    }
}

void Karikari::exeWait() {
    applyEnvironmentInfluenceToVelocity();
    mVelocity += mGravity * ::sGravity;

    if (MR::isFirstStep(this)) {
        MR::onBind(this);
        MR::onCalcShadow(this, nullptr);
        MR::onCalcGravity(this);
        MR::validateHitSensors(this);
        MR::startAction(this, "Wait");
        MR::startBtp(this, "Bite");
        MR::setBtpFrameAndStop(this, 0.0f);
        mIsAttackEnabled = true;
        mIsPushable = true;
    }

    if (tryStarPointerBind()) {
        return;
    }

    TVec3f diff(*MR::getPlayerPos());
    diff.sub(mPosition);
    if (diff.squared() < ::sDistToPursue * ::sDistToPursue) {
        setNerve(GET_NERVE(Karikari, HostTypeNrvPrePursue));
        return;
    }

    if (MR::isOnGround(this)) {
        MR::zeroVelocity(this);
    }
}

void Karikari::exePrePursue() {
    if (MR::isFirstStep(this)) {
        MR::onBind(this);
        MR::onCalcShadow(this, nullptr);
        MR::onCalcGravity(this);
        MR::validateHitSensors(this);
        MR::startActionSound(this, "EmKarikariFind");
        MR::startAction(this, "Search");
        MR::startBtp(this, "Bite");
        MR::setBtpFrameAndStop(this, 0.0f);
        mIsAttackEnabled = true;
        mIsPushable = false;
    }

    applyEnvironmentInfluenceToVelocity();
    mVelocity += mGravity * ::sGravity;

    tryStarPointerBind();

    if (MR::isLessStep(this, ::sJumpAgainTime)) {
        TVec3f toPlayer(*MR::getPlayerPos() - mPosition);
        tryTurnToDirection(toPlayer, ::sTurnRatio);
        mIsPushable = true;
        return;
    }

    if (MR::isStep(this, ::sJumpAgainTime)) {
        mVelocity.sub(mGravity * ::sJumpToPrePursueVel);
        mIsPushable = false;
        return;
    }

    if (MR::isOnGround(this)) {
        setNerve(GET_NERVE(Karikari, HostTypeNrvPursue));
        MR::emitEffect(this, "SmokeLand");
    }
}

void Karikari::exePursue() {
    if (MR::isFirstStep(this)) {
        MR::onBind(this);
        MR::onCalcShadow(this, nullptr);
        MR::onCalcGravity(this);
        MR::validateHitSensors(this);
        MR::startAction(this, "Jump");
        MR::setBckRate(this, ::sJumpAnimRate);
        MR::startBtp(this, "Bite");
        MR::setBtpFrameAndStop(this, 0.0f);
        mIsAttackEnabled = true;
        mIsPushable = true;
    }

    applyEnvironmentInfluenceToVelocity();
    mVelocity += mGravity * ::sGravity;

    tryStarPointerBind();

    TVec3f toPlayerH(*MR::getPlayerPos() - mPosition);
    f32 dist = toPlayerH.squared();
    MR::vecKillElement(toPlayerH, mGravity, &toPlayerH);
    MR::normalizeOrZero(&toPlayerH);
    if (MR::isLessStep(this, ::sJumpAgainTime)) {
        mIsPushable = true;
        tryTurnToDirection(toPlayerH, ::sTurnRatio);
        return;
    }

    if (MR::isStep(this, ::sJumpAgainTime)) {
        MR::startActionSound(this, "EmKarikariJump2");
        mVelocity.sub(mGravity * ::sJumpToPursueVel);
        toPlayerH.scale(::sPursueVel);
        mVelocity += toPlayerH;
        return;
    }

    if (MR::isOnGround(this)) {
        MR::emitEffect(this, "SmokeLand");
        if (::getKarikariDirector()->isMaxNumCling()) {
            setNerve(GET_NERVE(Karikari, HostTypeNrvWatchFor));
            return;
        }

        if (::sDistToPursue * ::sDistToPursue < dist) {
            setNerve(GET_NERVE(Karikari, HostTypeNrvWait));
            return;
        }

        tryTurnToDirection(toPlayerH, ::sTurnRatio);
        setNerve(GET_NERVE(Karikari, HostTypeNrvPursue));

    } else {
        mIsPushable = false;
    }
}

void Karikari::exeWatchFor() {
    if (MR::isFirstStep(this)) {
        MR::onBind(this);
        MR::onCalcShadow(this, nullptr);
        MR::onCalcGravity(this);
        MR::validateHitSensors(this);
        MR::startAction(this, "Glad");
        MR::startBtp(this, "Bite");
        MR::setBtpFrameAndStop(this, 0.0f);
        mIsAttackEnabled = false;
        mIsPushable = true;
    }

    applyEnvironmentInfluenceToVelocity();
    mVelocity += mGravity * ::sWatchForGravity;

    tryStarPointerBind();

    if (!MR::isOnGround(this)) {
        return;
    }

    if (MR::isLessStep(this, ::sWatchForJumpTime)) {
        TVec3f toPlayer(*MR::getPlayerPos() - mPosition);
        tryTurnToDirection(toPlayer, ::sTurnRatio);

        if (!::getKarikariDirector()->isMaxNumCling() || ::sWatchForDistance * ::sWatchForDistance < toPlayer.squared()) {
            setNerve(GET_NERVE(Karikari, HostTypeNrvPrePursue));
        }
        return;
    }

    if (MR::isStep(this, ::sWatchForJumpTime)) {
        mVelocity.sub(mGravity * ::sWatchForJumpVel);
        return;
    }

    if (MR::isGreaterStep(this, ::sWatchForLoopInterval)) {
        setNerve(GET_NERVE(Karikari, HostTypeNrvWatchFor));
    }
}

void Karikari::exePreCling() {
    if (MR::isFirstStep(this)) {
        MR::offBind(this);
        MR::onCalcShadow(this, nullptr);
        MR::onCalcGravity(this);
        MR::validateHitSensors(this);
        MR::startAction(this, "Wait");
        MR::startBtp(this, "Bite");
        MR::setBtpFrameAndStop(this, 0.0f);
        mIsAttackEnabled = false;
        mIsPushable = false;
    }

    MR::attachSupportTicoToTarget(this);

    TVec3f toClingPos;
    f32 t = getNerveStep() / static_cast< f32 >(::sPreClingTime);
    mClingPosition->copyTrans(&toClingPos);
    toClingPos.sub(mPosition);
    mVelocity = mVelocity * (1.0f - t) + toClingPos * t;
    MR::startActionSound(this, "EmLvKarikariCling");

    if (MR::isGreaterStep(this, ::sPreClingTime)) {
        MR::onBind(this);
        setNerve(GET_NERVE(Karikari, HostTypeNrvCling));
    }
}

void Karikari::exeCling() {
    if (MR::isFirstStep(this)) {
        MR::offBind(this);
        MR::offCalcShadow(this, nullptr);
        MR::invalidateShadow(this, nullptr);
        MR::offCalcGravity(this);
        MR::validateHitSensors(this);
        mIsAttackEnabled = false;
        mIsPushable = false;
        MR::setBckRate(this, 1.0f);
        MR::startAction(this, "Bite");
        MR::startBtp(this, "Bite");
        MR::startActionSound(this, "EmKarikariTouch");
        mVelocity.zero();
    }

    MR::attachSupportTicoToTarget(this);
    mClingPosition->copyTrans(&mPosition);
    MR::startActionSound(this, "EmLvKarikariCling");
}

void Karikari::exeRelease() {
    if (MR::isFirstStep(this)) {
        MR::startBtp(this, "Bite");
        MR::setBtpFrameAndStop(this, 0.0f);
    }

    TVec3f away(mPosition);
    away.sub(*MR::getPlayerPos());
    MR::vecKillElement(away, mGravity, &away);
    away.setLength(::sAttackedVel);
    away.sub(mGravity * ::sAttackedVerticalVel);
    mVelocity.set(away);
    setNerve(GET_NERVE(Karikari, HostTypeNrvSpinAttacked));
}

void Karikari::exeSpinAttacked() {
    if (MR::isFirstStep(this)) {
        MR::onBind(this);
        MR::onCalcShadow(this, nullptr);
        MR::onCalcGravity(this);
        MR::validateShadow(this, nullptr);
        MR::validateHitSensors(this);
        MR::startAction(this, "SpinX");
        MR::setBckRate(this, ::sSpinXAnimRate);
        MR::startBtp(this, "Bite");
        MR::setBtpFrameAndStop(this, 0.0f);
        MR::startSystemME("ME_MAGIC");
        mIsAttackEnabled = false;
        mIsPushable = true;
    }

    applyEnvironmentInfluenceToVelocity();
    mVelocity += mGravity * ::sGravitySpinAttacked;

    if (MR::isGreaterStep(this, ::sSpinAttackedTime)) {
        setNerve(GET_NERVE(Karikari, HostTypeNrvFrozen));
    }
}

void Karikari::exeDPDAttacked() {
    if (MR::isFirstStep(this)) {
        MR::onBind(this);
        MR::onCalcShadow(this, nullptr);
        MR::onCalcGravity(this);
        MR::validateShadow(this, nullptr);
        MR::validateHitSensors(this);
        MR::startAction(this, "SpinX");
        MR::setBckRate(this, ::sSpinXAnimRate);
        MR::startBtp(this, "Bite");
        MR::setBtpFrameAndStop(this, 0.0f);
        mIsAttackEnabled = false;
        mIsPushable = true;
        MR::startActionSound(this, "EmKarikariRelease");
        MR::emitEffect(this, "Touch");
    }

    applyEnvironmentInfluenceToVelocity();
    mVelocity += mGravity * ::sDPDAttackedGravity;

    if (MR::isOnGround(this)) {
        MR::emitEffect(this, "SmokeLand");
        setNerve(GET_NERVE(Karikari, HostTypeNrvLand));
    }
}

void Karikari::exeFrozen() {
    if (MR::isFirstStep(this)) {
        MR::offBind(this);
        MR::offCalcShadow(this, nullptr);
        MR::offCalcGravity(this);
        MR::validateHitSensors(this);
        mIsAttackEnabled = true;
        mIsPushable = false;
        MR::startBtp(this, "Bite");
        MR::setBtpFrameAndStop(this, 0.0f);
        mVelocity.zero();
        MR::startAction(this, "Wait");
        MR::setBckFrameAndStop(this, 0.0f);
        MR::emitEffect(this, "KarikariCrystalLight");
        mFrozenTime = ::sFrozenLimitTime;
    }

    if (MR::isStep(this, ::sIceAppearTime)) {
        MR::invalidateShadow(this, nullptr);
    }

    if (--mFrozenTime <= 0) {
        MR::onBind(this);
        setNerve(GET_NERVE(Karikari, HostTypeNrvFrozenRecover));
    }
}

void Karikari::exeFrozenRecover() {
    if (MR::isFirstStep(this)) {
        MR::onBind(this);
        MR::onCalcShadow(this, nullptr);
        MR::onCalcGravity(this);
        MR::validateShadow(this, nullptr);
        MR::validateHitSensors(this);
        mIsAttackEnabled = true;
        mIsPushable = true;

        mVelocity = (-mGravity) * ::sJumpToFrozenRecoverVel;

        MR::deleteEffect(this, "KarikariCrystalLight");
        MR::emitEffect(this, "KarikariCrystalBreak");
        MR::startAction(this, "SpinX");
        MR::setBckRate(this, ::sSpinXAnimRate);
        MR::startBtp(this, "Bite");
        MR::setBtpFrameAndStop(this, 0.0f);
    }

    applyEnvironmentInfluenceToVelocity();
    mVelocity += mGravity * ::sFrozenRecoverGravity;

    if (MR::isOnGround(this)) {
        MR::emitEffect(this, "KarikariSmokeLand");
        setNerve(GET_NERVE(Karikari, HostTypeNrvLand));
    }
}

// NONMATCHING: vector temporary stack-slot order.
void Karikari::exeBlowOut() {
    if (MR::isFirstStep(this)) {
        MR::onBind(this);
        MR::onCalcShadow(this, nullptr);
        MR::onCalcGravity(this);
        MR::validateShadow(this, nullptr);
        MR::validateHitSensors(this);
        mIsAttackEnabled = false;
        mIsPushable = true;
        MR::startActionSound(this, "EmKarikariBlow");
        MR::startAction(this, "SpinX");
        MR::setBckRate(this, ::sSpinXAnimRate);
        MR::startBtp(this, "Bite");
        MR::setBtpFrameAndStop(this, 0.0f);
        MR::startBlowHitSound(this);
    }

    applyEnvironmentInfluenceToVelocity();
    mVelocity += mGravity * ::sBlowOutGravity;

    if (MR::isBinded(this) || MR::isGreaterStep(this, ::sBlowOutTime)) {
        if (isNerve(GET_NERVE(Karikari, HostTypeNrvBlowOut))) {
            MR::appearStarPiece(this, calcKarikariStarPiecePosition(-mGravity, mPosition), 1, 10.0f, 40.0f, false);
        } else {
            MR::appearStarPiece(this, calcKarikariStarPiecePosition(-mGravity, mPosition), 3, 10.0f, 40.0f, false);
        }
        kill();
    }
}

void Karikari::exeInTornado() {
    if (MR::isFirstStep(this)) {
        MR::onBind(this);
        MR::onCalcShadow(this, nullptr);
        MR::onCalcGravity(this);
        MR::invalidateHitSensors(this);
        MR::startAction(this, "Press");
        MR::startBtp(this, "Bite");
        MR::setBtpFrameAndStop(this, 0.0f);
        MR::startActionSound(this, "EmKarikariStomped", -1, -1, -1);
    }

    applyEnvironmentInfluenceToVelocity();
    TVec3f gravity(mGravity);
    gravity.scale(2.4f);
    mVelocity += gravity;

    if (MR::isGreaterStep(this, 40)) {
        kill();
    }
}

bool Karikari::tryStarPointerBind() {
    if (MR::attachSupportTicoToTarget(this)) {
        setNerve(GET_NERVE(Karikari, HostTypeNrvPress));
        return true;
    }

    return false;
}

void Karikari::exePress() {
    if (MR::isFirstStep(this)) {
        MR::onBind(this);
        MR::onCalcShadow(this, nullptr);
        MR::offCalcGravity(this);
        MR::validateHitSensors(this);
        mIsAttackEnabled = false;
        mIsPushable = true;
    }

    if (MR::updateActorStateAndNextNerve(this, GET_NERVE(Karikari, HostTypeNrvWait))) {
        MR::onCalcGravity(this);
    }
}

bool Karikari::inClingNerve() const {
    return isNerve(GET_NERVE(Karikari, HostTypeNrvPreCling)) || isNerve(GET_NERVE(Karikari, HostTypeNrvCling));
}

bool Karikari::tryBlowOut(const TVec3f& rPos, bool spawnMultipleStarPieces) {
    if (isNerve(GET_NERVE(Karikari, HostTypeNrvBlowOut)) || isNerve(GET_NERVE(Karikari, HostTypeNrvBlowOutStarPiece)) ||
        isNerve(GET_NERVE(Karikari, HostTypeNrvRelease)) || isNerve(GET_NERVE(Karikari, HostTypeNrvSpinAttacked)) ||
        isNerve(GET_NERVE(Karikari, HostTypeNrvDPDAttacked)) || isNerve(GET_NERVE(Karikari, HostTypeNrvInTornado))) {
        return false;
    }

    TVec3f toPosition(mPosition);
    toPosition.sub(rPos);
    MR::vecKillElement(toPosition, mGravity, &toPosition);
    toPosition.setLength(::sBlowOutVel);
    TVec3f vertical(mGravity);
    vertical.scale(::sBlowOutVerticalVel);
    toPosition.sub(vertical);
    mVelocity = toPosition;

    MR::emitEffect(this, "Hit");

    ::getKarikariDirector()->removeClingingKarikari(this);

    if (spawnMultipleStarPieces) {
        setNerve(GET_NERVE(Karikari, HostTypeNrvBlowOutStarPiece));
    } else {
        setNerve(GET_NERVE(Karikari, HostTypeNrvBlowOut));
    }

    return true;
}

bool Karikari::tryElectricKill() {
    if (!inClingNerve()) {
        return false;
    }

    MR::emitEffect(this, "ElectricDeath");
    makeActorDead();
    return true;
}

bool Karikari::tryHipDropRelease() {
    TVec3f away(mPosition);
    away.sub(*MR::getPlayerPos());
    MR::vecKillElement(away, mGravity, &away);
    away.setLength(::sAttackedVel);
    away.sub(mGravity * ::sAttackedVerticalVel);
    mVelocity.set(away);
    setNerve(GET_NERVE(Karikari, HostTypeNrvDPDAttacked));
    ::getKarikariDirector()->removeClingingKarikari(this);
    return true;
}

void Karikari::makeActorDead() {
    LiveActor::makeActorDead();
    ::getKarikariDirector()->removeClingingKarikari(this);
}

void Karikari::kill() {
    MR::emitEffect(this, "Death");
    MR::startActionSound(this, "EmExplodeS");
    LiveActor::kill();
}



void Karikari::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(GET_NERVE(Karikari, HostTypeNrvCling)) || isNerve(GET_NERVE(Karikari, HostTypeNrvPreCling)) ||
        isNerve(GET_NERVE(Karikari, HostTypeNrvBlowOut)) || isNerve(GET_NERVE(Karikari, HostTypeNrvBlowOutStarPiece)) ||
        isNerve(GET_NERVE(Karikari, HostTypeNrvSpinAttacked))) {
        return;
    }

    if (!MR::isSensorType(pSender, 0x20 /* ATYPE_KARIKARI */)) {
        return;
    }

    if (mIsPushable && MR::isSensorEnemy(pReceiver)) {
        MR::sendMsgPush(pReceiver, pSender);
        return;
    }

    if (mIsAttackEnabled && MR::isSensorPlayer(pReceiver)) {
        if (isNerve(GET_NERVE(Karikari, HostTypeNrvFrozen))) {
            killedInFrozenState();
            return;
        }

        TVec3f diff(*MR::getSensorPos(pSender));
        diff.sub(*MR::getSensorPos(pReceiver));
        f32 dist = diff.squared();
        f32 clingDist = ::sDistToCling;
        if (dist < clingDist * clingDist) {
            if (::getKarikariDirector()->registCling(this)) {
                setNerve(GET_NERVE(Karikari, HostTypeNrvPreCling));
                mVelocity.zero();
            }
        }
    }
}

bool Karikari::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgHitmarkEmit(msg)) {
        return true;
    }

    if (MR::isMsgInhaleBlackHole(msg)) {
        kill();
        return true;
    }

    if (MR::isMsgPlayerKick(msg) && isNerve(GET_NERVE(Karikari, HostTypeNrvPress))) {
        return tryBlowOut(*MR::getSensorPos(pSender), false);
    }

    return false;
}

bool Karikari::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgLockOnStarPieceShoot(msg)) {
        return true;
    }

    if (MR::isMsgStarPieceAttack(msg)) {
        return tryBlowOut(*MR::getSensorPos(pSender), true);
    }

    if (MR::isMsgSupportTicoSpin(msg)) {
        return tryBlowOut(*MR::getSensorPos(pSender), false);
    }

    if (inClingNerve()) {
        return false;
    }

    if (MR::isMsgPlayerHitAll(msg)) {
        return tryBlowOut(*MR::getSensorPos(pSender), false);
    }

    return false;
}

bool Karikari::receiveMsgPush(HitSensor* pSender, HitSensor* pReceiver) {
    if (!MR::isSensorType(pReceiver, 0x20 /* ATYPE_KARIKARI */)) {
        return false;
    }

    if (inClingNerve() || isNerve(GET_NERVE(Karikari, HostTypeNrvBlowOut)) || isNerve(GET_NERVE(Karikari, HostTypeNrvBlowOutStarPiece)) ||
        isNerve(GET_NERVE(Karikari, HostTypeNrvFrozen))) {
        return false;
    }

    if (MR::isSensorEnemy(pSender) || MR::isSensorMapObj(pSender)) {
        TVec3f diff(mPosition);
        diff.sub(MR::getSensorHost(pSender)->mPosition);
        MR::normalizeOrZero(&diff);
        mVelocity += diff;
        return true;
    }

    return false;
}

bool Karikari::receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(GET_NERVE(Karikari, HostTypeNrvRelease)) || isNerve(GET_NERVE(Karikari, HostTypeNrvSpinAttacked)) ||
        isNerve(GET_NERVE(Karikari, HostTypeNrvBlowOut)) || isNerve(GET_NERVE(Karikari, HostTypeNrvBlowOutStarPiece)) ||
        isNerve(GET_NERVE(Karikari, HostTypeNrvInTornado))) {
        return false;
    }

    if (isNerve(GET_NERVE(Karikari, HostTypeNrvFrozen))) {
        killedInFrozenState();
        return true;
    }

    if (MR::isMsgToEnemyAttackBlow(msg)) {
        bool isCling = false;
        if (isNerve(GET_NERVE(Karikari, HostTypeNrvPreCling)) || isNerve(GET_NERVE(Karikari, HostTypeNrvCling))) {
            isCling = true;
        }
        if (isCling) {
            ::getKarikariDirector()->unregistCling(this);
        }
        // blow away from enemy
        TVec3f vel(*MR::getSensorPos(pReceiver));
        vel.sub(*MR::getSensorPos(pSender));
        MR::normalizeOrZero(&vel);
        vel.scale(::sEnemyAttackHitVel);
        mVelocity.set(vel);
        setNerve(GET_NERVE(Karikari, HostTypeNrvBlowOut));
        return true;
    }

    if (MR::isMsgToEnemyAttackTrample(msg)) {
        ::getKarikariDirector()->removeClingingKarikari(this);
        setNerve(GET_NERVE(Karikari, HostTypeNrvInTornado));
        return true;
    }

    if (MR::isMsgExplosionAttack(msg)) {
        if (isNerve(GET_NERVE(Karikari, HostTypeNrvFrozen))) {
            killedInFrozenState();
            return true;
        }

        ::getKarikariDirector()->removeClingingKarikari(this);
        // blow away from explosion
        TVec3f vel(*MR::getSensorPos(pReceiver));
        vel.sub(*MR::getSensorPos(pSender));
        vel.setLength(::sExplosionBlowOutVel);
        mVelocity = vel;
        setNerve(GET_NERVE(Karikari, HostTypeNrvBlowOut));
        return true;
    }

    return false;
}

void Karikari::tryTurnToDirection(const TVec3f& rDir, f32 turnRatio) {
    TQuat4f rot;
    rot.setRotate(mFront, rDir, turnRatio);
    rot.transform(mFront);
}

void Karikari::calcAndSetBaseMtx() {
    MR::updateBaseScale(this, mAnimScale);

    if (isNerve(GET_NERVE(Karikari, HostTypeNrvCling))) {
        MR::setBaseTRMtx(this, mClingPosition->mMtx);
        return;
    }

    MR::setBaseTRMtx(this, mPoseQuat);
}

Karikari::~Karikari() {
}
