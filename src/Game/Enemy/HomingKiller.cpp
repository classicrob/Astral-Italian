#include "Game/Enemy/HomingKiller.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util.hpp"
#include "Game/Scene/SceneFunction.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/LiveActor/ModelObj.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/LiveActor/PartsModel.hpp"
#include "Game/Util/ActorCameraUtil.hpp"
#include "Game/Util/ActorMovementUtil.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/ActorShadowUtil.hpp"
#include "Game/Util/ActorSwitchUtil.hpp"
#include "Game/Util/CameraUtil.hpp"
#include "Game/Util/EffectUtil.hpp"
#include "Game/Util/GravityUtil.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/MtxUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include "Game/Util/StarPointerUtil.hpp"
#include "Game/LiveActor/ActorItemGenerator.hpp"

namespace {
    static const Vec cBodyHitSensorOffset = {0.0f, 0.0f, -10.0f};

    const u32 cSensorTableTarget[] = {34, 78, 76, 47, 48, 79};
    const u32 cSensorTableAttackIfBinded[] = {34, 78, 76, 79, 49, 50, 64, 86};
    const u32 cSensorTableTryExplosion[] = {34, 78, 47, 8, 48, 32, 73, 76};

    static const f32 cBodyHitSensorRadius = 70.0f;
    static const f32 cEyeHitSensorRadius = 750.0f;
    static const f32 cBinderRadius = 70.0f;
    static const f32 cExplosionDistance = 250.0f;
    static const f32 cShadowRadiusXY = 60.0f;
    static const f32 cShadowRadiusZ = 100.0f;
    static const f32 cStarWandRadius3d = 150.0f;
    static const f32 cSandColumnEmitDot = -0.75f;
    static const s32 cAppearIntervalFrame = 180;
    static const s32 cForceKillFrame = 600;
    static const f32 cCameraShakeDistance = 1000.0f;
    static const f32 cChaseStartAngle = 360.0f;
    static const f32 cChaseSpeed = 12.0f;
    static const f32 cChaseTargetHeight = 200.0f;
    static const f32 cChaseEndDistance = 3000.0f;
    static const f32 cChaseInvalidFrame = 35;  // BUG: typo, should be an s32 :)
    static const f32 cChaseRotateZSpeed = 1.0f;
    static const f32 cChaseRotateZMax = 60.0f;
    static const f32 cChaseRotateZDot = 0.95f;
    static const f32 cChaseHeightMin = 150.0f;
    static const f32 cUpVecRotateSpeed = 2.0f;
    static const f32 cMoveRotateCosineMax = 0.9995f;  // ~cos(1.8)
    static const s32 cFreezeFrame = 20;
    static const f32 cFreezeRumbleSpeed = 75.0f;
    static const f32 cFreezeRumbleWidth = 5.0f;
    static const f32 cFreezeRotateCosineMax = 0.9997f;  // ~cos(1.4)
    static const s32 cAppearMoveFrame = 20;
    static const s32 cAppearRumbleFrame = 22;
    static const s32 cAppearStopFrame = 30;
    static const f32 cAppearMoveDistance = 200.0f;
    static const f32 cAppearRumbleSpeed = 100.0f;
    static const f32 cAppearRumbleWidth = 18.0f;
    static const f32 cChaseSpeedTorpedo = 5.0f;
    static const f32 cChaseTargetHeightTorpedo = 75.0f;
    static const f32 cTorpedoLightClippingRadius = 1000.0f;
    static const f32 cNearClipDistanceMagnum = 400.0f;
    static const f32 cMagnamFlySeDistMin = 500.0f;
    static const f32 cMagnamFlySeDistMax = 1400.0f;
    static const f32 cMagnamFlySePitchMin = 0.9f;
    static const f32 cMagnamFlySePitchMax = 1.8f;
};  // namespace

namespace NrvHomingKiller {
    NEW_NERVE(HomingKillerNrvAppear, HomingKiller, Appear);
    NEW_NERVE(HomingKillerNrvMoveStart, HomingKiller, MoveStart);
    NEW_NERVE(HomingKillerNrvMove, HomingKiller, Move);
    NEW_NERVE(HomingKillerNrvChaseStart, HomingKiller, ChaseStart);
    NEW_NERVE(HomingKillerNrvChase, HomingKiller, Chase);
    NEW_NERVE(HomingKillerNrvFreeze, HomingKiller, Freeze);
    NEW_NERVE(HomingKillerNrvBreak, HomingKiller, Break);
    NEW_NERVE(HomingKillerNrvGoToTarget, HomingKiller, GoToTarget);
    NEW_NERVE(HomingKillerNrvTongue, HomingKiller, Tongue);
    NEW_NERVE_ONEND(HomingKillerNrvEaten, HomingKiller, Eaten, Eaten);
    NEW_NERVE(HomingKillerNrvShot, HomingKiller, Shot);

};  // namespace NrvHomingKiller


namespace {
    bool isSensorType(const HitSensor* pSensor, const u32* pTypes, u32 numTypes) {
        u32 sensorType = pSensor->mType;
        for (u32 idx = 0; idx < numTypes; idx++) {
            if (sensorType == pTypes[idx]) {
                return true;
            }
        }

        return false;
    }
};  // namespace

HomingKiller::HomingKiller(const char* pName)
    : LiveActor(pName), mType(Type_HomingKiller), mSpeed(::cChaseSpeed), mChaseStartDist(2000.0f), mChaseEndDist(::cChaseEndDistance),
      mChaseRotateSpeed(0.9997f), mChaseStartAngle(::cChaseStartAngle), mFront(0.0f, 0.0f, 1.0f), mUp(0.0f, 1.0f, 0.0f), mBasePos(gZeroVec),
      mBaseFront(0.0f, 0.0f, 1.0f), mBaseUp(0.0f, 1.0f, 0.0f), mUnfreezeNerve(nullptr), mChaseInvalidTime(0), mMoveTime(0), mTargetSensor(),
      mDisableChase(), mIsLinearShot(), mUseFullSightAngle(), mPropeller(), mTorpedoLight(), mShotBindTime(0), mYoshiLockOn(nullptr),
      mMovementType(MR::MovementType_Enemy), mForceKillTime(600), mChaseInvalidFrame(35), mShotTime(10), mScaleParam(nullptr), mScaleController(nullptr) {
    mBaseMtx.identity();
    mEffectMtx.identity();
}

// NONMATCHING: vector temporary stack-slot order.
void HomingKiller::init(const JMapInfoIter& rIter) {
    const char* objectName = "HomingKiller";
    if (MR::isValidInfo(rIter)) {
        MR::getObjectName(&objectName, rIter);
        if (MR::isEqualObjectName(rIter, "Torpedo")) {
            setType(Type_Torpedo);
        } else if (MR::isEqualObjectName(rIter, "MagnumKiller")) {
            setType(Type_MagnumKiller);
        }
    } else {
        setType(mType);
        if (mType == Type_Torpedo) {
            objectName = "Torpedo";
        } else if (mType == Type_MagnumKiller) {
            objectName = "MagnumKiller";
            mIsLinearShot = true;
        }
    }

    if (MR::isValidInfo(rIter)) {
        initMapToolInfo(rIter);
    }

    MR::initActor(this, rIter, objectName, false);

    if (mType == Type_Torpedo) {
        mPropeller = MR::createPartsModelEnemyAndFix(this, "トーピードプロペラ", "TorpedoPropeller", nullptr, TVec3f(0.0f, 0.0f, 0.0f),
                                                     TVec3f(0.0f, 0.0f, 0.0f), nullptr);
        mTorpedoLight = MR::createModelObjMapObj("トーピードライト", "TorpedoLight", getBaseMtx());
        MR::invalidateClipping(mTorpedoLight);
        mTorpedoLight->makeActorDead();
        mSpeed = ::cChaseSpeedTorpedo;
    }

    MR::connectToScene(this, mMovementType, 8, MR::DrawBufferType_Enemy, -1);
    MR::initLightCtrl(this);

    initHitSensor(2);
    u32 sensorType = 45;
    if (mType == Type_MagnumKiller) {
        sensorType = 46;
    }
    f32 hitRadius = getScale();
    f32 offsetScale = getScale();

    MR::addHitSensor(this, "Body", sensorType, 8, ::cBodyHitSensorRadius * hitRadius, TVec3f(::cBodyHitSensorOffset) * offsetScale);
    MR::addHitSensorEye(this, "eye", 16, ::cEyeHitSensorRadius, TVec3f(0.0f, 0.0f, 0.0f));

    f32 binderRadius = getScale();
    initBinder(::cBinderRadius * binderRadius, 0.0f, 0);

    MR::onCalcGravity(this);
    if (MR::isValidInfo(rIter)) {
        MR::calcGravity(this);
    }

    initEffectKeeper(0, nullptr, false);

    if (mType == Type_HomingKiller) {
        MR::setEffectHostMtx(this, "SandColumn", mEffectMtx);
    }

    if (mType == Type_MagnumKiller) {
        MR::initStarPointerTarget(this, 350.0f, TVec3f(0.0f, 0.0f, 0.0f));
    } else {
        MR::initStarPointerTarget(this, ::cStarWandRadius3d, TVec3f(0.0f, 0.0f, 0.0f));
    }

    initNerve(GET_NERVE(HomingKiller, HomingKillerNrvAppear), 0);

    if (MR::isValidInfo(rIter)) {
        calcInitPosture();
    }

    if (mType == Type_Torpedo) {
        TVec3f soundOffset;
        soundOffset.setAll< f32 >(0.0f);
        initSound(12, "Torpedo", nullptr, soundOffset);
    } else if (mType == Type_MagnumKiller) {
        TVec3f soundOffset;
        soundOffset.setAll< f32 >(0.0f);
        initSound(12, "MagnumKiller", nullptr, soundOffset);
    } else {
        TVec3f soundOffset;
        soundOffset.setAll< f32 >(0.0f);
        initSound(12, "HomingKiller", nullptr, soundOffset);
    }

    if (mType == Type_MagnumKiller) {
        MR::initShadowFromCSVWithInitShadowVolumeSphere(this, "Shadow");
    } else {
        f32 scale = 1.0f;
        MR::initShadowVolumeOval(this, TVec3f(::cShadowRadiusXY, ::cShadowRadiusXY, ::cShadowRadiusZ) * scale);
        MR::setShadowDropLength(this, nullptr, 2000.0f);
    }

    MR::addToAttributeGroupSearchTurtle(this);
    MR::invalidateClipping(this);

    TVec3f lockOnOffset;
    lockOnOffset.setAll< f32 >(0.0f);
    mYoshiLockOn = MR::createYoshiLockOnTarget(this, static_cast<MtxPtr>(nullptr), lockOnOffset);

    mScaleParam = new AnimScaleParam();
    mScaleController = new AnimScaleController(mScaleParam);
    mScaleParam->_0 = 0.5f;
    mScaleParam->_4 = 0.85f;
    mScaleParam->_10 = 8.0f;

    if (mType == Type_MagnumKiller) {
        mScaleParam->_14 = 0.95f;
        mScaleParam->_18 = 0.05f;
        mScaleParam->_1C = 2.1f;
    } else {
        mScaleParam->_14 = 0.85f;
        mScaleParam->_18 = 0.2f;
        mScaleParam->_1C = 2.3f;
    }

    makeActorDead();
}

void HomingKiller::appear() {
    mPosition.set(mBasePos);
    mRotation.zero();
    mScale.set(1.0f);
    mFront.set(mBaseFront);
    mUp.set(mBaseUp);
    mChaseInvalidTime = 0;
    mMoveTime = 0;
    updateBaseMtxNoRotateZ();
    mScaleController->stopAndReset();
    MR::showModel(this);
    MR::offBind(this);
    MR::validateHitSensors(this);
    MR::invalidateShadow(this, nullptr);
    LiveActor::appear();
    setNerve(GET_NERVE(HomingKiller, HomingKillerNrvAppear));
}

void HomingKiller::appear(const TVec3f& rPos, const TVec3f& rFront) {
    mBasePos.set(rPos);
    mBaseFront.set(rFront);
    MR::calcGravityVector(this, rPos, &mBaseUp, nullptr, 0);
    mBaseUp.negate(mGravity);
    appear();
}

bool HomingKiller::isNerveAppear() const {
    return isNerve(GET_NERVE(HomingKiller, HomingKillerNrvAppear));
}

bool HomingKiller::isMoveStart() const {
    bool ret = false;
    if (isNerve(GET_NERVE(HomingKiller, HomingKillerNrvMoveStart)) && mMoveTime == 0) {
        ret = true;
    }
    return ret;
}

void HomingKiller::setChaseStartEndDistance(f32 startDist, f32 endDist) {
    mChaseStartDist = startDist;
    mChaseEndDist = endDist;
}

void HomingKiller::setTypeMagnumKiller() {
    setType(Type_MagnumKiller);
}

void HomingKiller::setChaseRotateSpeed(f32 speed) {
    mChaseRotateSpeed = speed;
}

void HomingKiller::initSpeed() {
    if (mType == Type_Torpedo) {
        mSpeed = ::cChaseSpeedTorpedo;
    } else if (mType == Type_HomingKiller) {
        mSpeed = ::cChaseSpeed;
    }
}

// NONMATCHING: local boolean registers and effect-argument scheduling.
void HomingKiller::control() {
    bool isInactive = false;
    if (isNerve(GET_NERVE(HomingKiller, HomingKillerNrvAppear)) || isNerve(GET_NERVE(HomingKiller, HomingKillerNrvBreak)) ||
        isNerve(GET_NERVE(HomingKiller, HomingKillerNrvEaten))) {
        isInactive = true;
    }

    if (!isInactive) {
        if (!isValidShowModel()) {
            MR::hideModelAndOnCalcAnimIfShown(this);
            if (mType != Type_Torpedo) {
                MR::deleteEffect(this, "Smoke");
                if (mType == Type_HomingKiller) {
                    MR::deleteEffect(this, "SmokeChase");
                }
            } else {
                MR::deleteEffect(this, "Bubble");
                MR::deleteEffect(this, "BubbleChase");
            }
        } else {
            MR::showModelIfHidden(this);
            if (mType != Type_Torpedo) {
                bool isMoving = false;
                if (isNerve(GET_NERVE(HomingKiller, HomingKillerNrvMoveStart)) || isNerve(GET_NERVE(HomingKiller, HomingKillerNrvMove)) ||
                    mType != Type_HomingKiller) {
                    isMoving = true;
                }
                MR::emitEffect(this, isMoving ? "Smoke" : "SmokeChase");
            } else {
                bool isMoving = false;
                if (isNerve(GET_NERVE(HomingKiller, HomingKillerNrvMoveStart)) || isNerve(GET_NERVE(HomingKiller, HomingKillerNrvMove))) {
                    isMoving = true;
                }
                MR::emitEffect(this, isMoving ? "Bubble" : "BubbleChase");
            }
        }

        if (mType == Type_Torpedo) {
            if (MR::isJudgedToClipFrustum(mTorpedoLight->mPosition, ::cTorpedoLightClippingRadius)) {
                MR::hideModelAndOnCalcAnimIfShown(mTorpedoLight);
            } else {
                MR::showModelIfHidden(mTorpedoLight);
            }
        }
    }

    if (!isNerve(GET_NERVE(HomingKiller, HomingKillerNrvBreak))) {
            if (!isGravityIgnored() || isChasing()) {
            TVec3f up;
            up.negate(mGravity);
            if (isNerve(GET_NERVE(HomingKiller, HomingKillerNrvChaseStart)) || isNerve(GET_NERVE(HomingKiller, HomingKillerNrvChase)) ||
                isNerve(GET_NERVE(HomingKiller, HomingKillerNrvFreeze)) || isNerve(GET_NERVE(HomingKiller, HomingKillerNrvGoToTarget))) {
                MR::turnVecToVecCos(&mUp, mUp.copy(), up, MR::cosDegree(::cUpVecRotateSpeed), mFront);
            } else {
                mUp.set(up);
            }
        }

        updateBaseMtxNoRotateZ();
    }

    if (isNormalNerve() && mType != Type_MagnumKiller) {
        YoshiFunction::tryYoshiProjectile(mYoshiLockOn);
    }

    mScaleController->update();
}

void HomingKiller::calcAndSetBaseMtx() {
    // FIXME: Regswap in inline
    // https://decomp.me/scratch/5jw7T

    TPos3f mtx2;
    TPos3f mtx;
    mtx.makeRotate(TVec3f(0.0f, 0.0f, 1.0f), MR::toRadian(mRotation.z));

    mtx2.concat(mBaseMtx, mtx);
    MR::setBaseTRMtx(this, mtx2);
    MR::updateBaseScale(this, mScaleController);
}

// NONMATCHING: unused send-message result comparison omitted.
void HomingKiller::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(GET_NERVE(HomingKiller, HomingKillerNrvBreak))) {
        return;
    }

    if (isNerve(GET_NERVE(HomingKiller, HomingKillerNrvTongue))) {
        return;
    }

    bool isShooting = false;
    if (isNerve(GET_NERVE(HomingKiller, HomingKillerNrvEaten)) && MR::isLessStep(this, 600)) {
        isShooting = true;
    }

    if (isShooting) {
        return;
    }

    if (pSender == getSensor("eye")) {
        if (isNerve(GET_NERVE(HomingKiller, HomingKillerNrvAppear))) {
            return;
        }

        if (isNerve(GET_NERVE(HomingKiller, HomingKillerNrvChase)) &&
            ::isSensorType(pReceiver, ::cSensorTableTarget, ARRAY_SIZE(::cSensorTableTarget))) {
            mTargetSensor = pReceiver;
            setNerve(GET_NERVE(HomingKiller, HomingKillerNrvGoToTarget));
        }

        return;
    }

    if (isYoshiHold()) {
        return;
    }

    if (isNerve(GET_NERVE(HomingKiller, HomingKillerNrvAppear)) || isNerve(GET_NERVE(HomingKiller, HomingKillerNrvFreeze))) {
        if (mType == Type_MagnumKiller && pReceiver->isType(45)) {
            if (MR::sendMsgEnemyAttackExplosion(pReceiver, pSender)) {
                return;
            }
        }

        MR::sendMsgPush(pReceiver, pSender);
        return;
    }

    if (MR::isSensorPlayerOrRide(pReceiver)) {
        if (MR::sendMsgEnemyAttackExplosion(pReceiver, pSender)) {
            setNerve(GET_NERVE(HomingKiller, HomingKillerNrvBreak));
            return;
        }
    } else {
        if (!pReceiver->isType(32) && tryToExplosion(pSender, pReceiver)) {
            if (mType != Type_MagnumKiller || pReceiver->isType(46)) {
                setNerve(GET_NERVE(HomingKiller, HomingKillerNrvBreak));
                return;
            }
        } else if (MR::isSensorYoshi(pReceiver)) {
            if (mType != Type_MagnumKiller) {
                if (MR::sendMsgPush(pReceiver, pSender)) {
                    setNerve(GET_NERVE(HomingKiller, HomingKillerNrvBreak));
                }
            } else {
                MR::sendMsgPush(pReceiver, pSender);
            }

            return;
        }
    }

    if (mType == Type_MagnumKiller && !MR::isSensorPlayerOrRide(pReceiver)) {
        if (MR::sendMsgToEnemyAttackTrample(pReceiver, pSender)) {
            return;
        }
    }
}

bool HomingKiller::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    bool isInactive = false;
    if (isNerve(GET_NERVE(HomingKiller, HomingKillerNrvAppear)) || isNerve(GET_NERVE(HomingKiller, HomingKillerNrvBreak)) ||
        isNerve(GET_NERVE(HomingKiller, HomingKillerNrvEaten))) {
        isInactive = true;
    }

    if (isInactive) {
        return false;
    }

    if (isNerve(GET_NERVE(HomingKiller, HomingKillerNrvTongue))) {
        return false;
    }

    if (MR::isMsgStarPieceReflect(msg)) {
        return true;
    }

    if (MR::isMsgFireBallAttack(msg)) {
        return true;
    }

    if (MR::isMsgJetTurtleAttack(msg) || MR::isMsgYoshiProjectileOrEat(msg) || MR::isMsgInvincibleAttack(msg) || MR::isMsgDrillAttack(msg)) {
        MR::emitItemHit(this);
        setNerve(GET_NERVE(HomingKiller, HomingKillerNrvBreak));
        return true;
    }

    return false;
}

bool HomingKiller::isBreakByEnemyAttack(u32 msg, HitSensor* pSender) {
    if (mType != Type_MagnumKiller) {
        return true;
    }

    if (MR::isMsgExplosionAttack(msg) && !pSender->isType(45)) {
        return true;
    }

    if (mType == Type_MagnumKiller && MR::isMsgExplosionAttack(msg) && pSender->isType(45)) {
        HomingKiller* pHost = static_cast< HomingKiller* >(MR::getSensorHost(pSender));
        bool isShotEnd = false;
        if (pHost->isNerve(GET_NERVE(HomingKiller, HomingKillerNrvShot)) && MR::isGreaterStep(pHost, pHost->mShotTime)) {
            isShotEnd = true;
        }

        if (isShotEnd) {
            return true;
        }
    }

    if (mType == Type_MagnumKiller && MR::isMsgToEnemyAttackBlow(msg)) {
        MR::tryHitReaction(this, "パンチングキノコヒット");
        return true;
    }

    return false;
}

// NONMATCHING: target loads sensor type 51 from sdata2 and retains a normalized boolean temporary.
namespace {
    inline bool isAppearAttackableSensor(HitSensor* pSensor) {
        static const u32 sSensorTypes[] = {51};
        for (s32 i = 0; i < 1; i++) {
            if (MR::isSensorType(pSensor, sSensorTypes[i])) {
                return true;
            }
        }
        return false;
    }
}  // namespace

bool HomingKiller::receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(GET_NERVE(HomingKiller, HomingKillerNrvBreak)) || isNerve(GET_NERVE(HomingKiller, HomingKillerNrvEaten))) {
        return false;
    }

    if (isNerve(GET_NERVE(HomingKiller, HomingKillerNrvAppear))) {
        if (!isAppearAttackableSensor(pSender)) {
            return false;
        }
    }

    if (isNerve(GET_NERVE(HomingKiller, HomingKillerNrvTongue))) {
        return false;
    }

    if (isBreakByEnemyAttack(msg, pSender)) {
        if (mType == Type_MagnumKiller) {
            MR::emitItemHit(this);
        }

        setNerve(GET_NERVE(HomingKiller, HomingKillerNrvBreak));
    }

    return true;
}

bool HomingKiller::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::tryForceKillIfMsgStartPowerStarGet(this, msg)) {
        return true;
    }

    if (isNormalNerve()) {
        if (MR::isMsgYoshiStartTongue(msg)) {
            setNerve(GET_NERVE(HomingKiller, HomingKillerNrvTongue));
            mVelocity.zero();
            return true;
        }
    } else if (isNerve(GET_NERVE(HomingKiller, HomingKillerNrvTongue))) {
        if (MR::isMsgYoshiLoopTongue(msg)) {
            return true;
        }

        if (MR::isMsgYoshiStartEaten(msg)) {
            setNerve(GET_NERVE(HomingKiller, HomingKillerNrvEaten));
            MR::hideModel(this);
            return true;
        }

        if (MR::isMsgYoshiCancelEaten(msg)) {
            setNerve(GET_NERVE(HomingKiller, HomingKillerNrvBreak));
            return true;
        }
    } else if (isNerve(GET_NERVE(HomingKiller, HomingKillerNrvEaten))) {
        if (MR::isMsgYoshiReleaseEaten(msg)) {
            setNerve(GET_NERVE(HomingKiller, HomingKillerNrvShot));
            return true;
        }

        if (MR::isMsgYoshiCancelEaten(msg)) {
            setNerve(GET_NERVE(HomingKiller, HomingKillerNrvBreak));
            return true;
        }
    }

    return false;
}

void HomingKiller::initMapToolInfo(const JMapInfoIter& rIter) {
    MR::initDefaultPos(this, rIter);
    MR::getJMapInfoArg0NoInit(rIter, &mChaseStartDist);

    s32 turnRadius = 3;
    MR::getJMapInfoArg1NoInit(rIter, &turnRadius);
    mChaseRotateSpeed = 1.0f - turnRadius * 0.0001f;

    MR::getJMapInfoArg2NoInit(rIter, &mIsLinearShot);
    if (mType == Type_MagnumKiller) {
        mIsLinearShot = true;
    }

    MR::getJMapInfoArg3NoInit(rIter, &mUseFullSightAngle);
    if (!mUseFullSightAngle) {
        mChaseStartAngle = ::cChaseStartAngle / 2;
    }

    MR::getJMapInfoArg6NoInit(rIter, &mForceKillTime);
    MR::getJMapInfoArg7NoInit(rIter, &mChaseInvalidFrame);
}

void HomingKiller::setType(s32 type) {
    if (type == 0) {
        mType = Type_HomingKiller;
        setName("ホーミングキラー");
    } else if (type == 1) {
        mType = Type_Torpedo;
        setName("トーピード");
    } else if (type == 2) {
        mType = Type_MagnumKiller;
        setName("マグナムキラー");
    }
}

void HomingKiller::calcInitPosture() {
    TPos3f mtx;
    MR::makeMtxRotate(mtx, mRotation);

    mBasePos.set(mPosition);
    mtx.getZDir(mBaseFront);

    if (!isGravityIgnored()) {
        MR::vecKillElement(mBaseFront, mGravity, &mBaseFront);
        MR::normalize(&mBaseFront);
    }

    if (!isGravityIgnored()) {
        mBaseUp.negate(mGravity);
    } else {
        mtx.getYDir(mBaseUp);
    }
}

void HomingKiller::updateVelocity() {
    mVelocity.set(mFront);
    f32 speed = mSpeed;
    bool isShotEnd = false;
    if (isNerve(GET_NERVE(HomingKiller, HomingKillerNrvShot)) && MR::isGreaterStep(this, mShotTime)) {
        isShotEnd = true;
    }

    if (isShotEnd) {
        speed = 17.0f;
    }

    mVelocity.scale(speed);
}

void HomingKiller::updateRotateZ(const TVec3f& rDir) {
    TVec3f grav = mGravity;
    TVec3f front;
    TVec3f side;

    bool turn = false;
    if (!MR::isSameDirection(mFront, grav) && !MR::isSameDirection(rDir, grav)) {
        MR::vecKillElement(mFront, grav, &front);
        MR::vecKillElement(rDir, grav, &side);
        if (front.dot(side) < ::cChaseRotateZDot) {
            turn = true;
        }
    }

    if (turn) {
        TVec3f up;
        up.cross(front, side);
        if (0.0f < up.dot(grav)) {
            mRotation.z = MR::clamp(mRotation.z + ::cChaseRotateZSpeed, -::cChaseRotateZMax, ::cChaseRotateZMax);
        } else {
            mRotation.z = MR::clamp(mRotation.z - ::cChaseRotateZSpeed, -::cChaseRotateZMax, ::cChaseRotateZMax);
        }
    } else {
        // TODO: this is likely MR::converge
        f32 angle = mRotation.z;
        if (angle < 0.0f) {
            angle += ::cChaseRotateZSpeed;
            if (angle > 0.0f) {
                angle = 0.0f;
            }
        } else {
            angle -= ::cChaseRotateZSpeed;
            if (angle < 0.0f) {
                angle = 0.0f;
            }
        }

        mRotation.z = angle;
    }
}

bool HomingKiller::processMove() {
    mMoveTime++;

    s32 forceKillTime;
    bool isShotEnd = false;
    if (isNerve(GET_NERVE(HomingKiller, HomingKillerNrvShot)) && MR::isGreaterStep(this, mShotTime)) {
        isShotEnd = true;
    }

    if (isShotEnd) {
        forceKillTime = 300;
    } else {
        forceKillTime = mForceKillTime;
    }

    if (tryBindedBreak() || forceKillTime < mMoveTime || (isGravityIgnored() && isWaterBreak())) {
        setNerve(GET_NERVE(HomingKiller, HomingKillerNrvBreak));
        return false;
    }

    if (!isGravityIgnored() && !isNerve(GET_NERVE(HomingKiller, HomingKillerNrvShot))) {
        TVec3f front;
        if (!MR::isSameDirection(mFront, mGravity, 0.01f)) {
            MR::vecKillElement(mFront, mGravity, &front);
            MR::normalize(&front);
        } else {
            front.set(mFront);
        }

        MR::turnVecToVecCos(&mFront, mFront.copy(), front, ::cMoveRotateCosineMax, mGravity);
    }

    updateVelocity();
    startMoveLevelSound(false);

    return true;
}

bool HomingKiller::processChase() {
    startMoveLevelSound(true);

    if (tryBindedBreak() || !MR::isNearPlayer(this, mChaseEndDist) || isWaterBreak()) {
        setNerve(GET_NERVE(HomingKiller, HomingKillerNrvBreak));
        return false;
    }

    TVec3f target;
    calcFrontVecToTarget(&target);

    if (MR::isShadowProjected(this, nullptr)) {
        TVec3f shadowPos;
        MR::getShadowProjectionPos(this, nullptr, &shadowPos);
        HitSensor* shadowSensor = MR::getShadowProjectedSensor(this, nullptr);
        if (shadowSensor->isType(76)) {
            if (MR::isNear(this, shadowPos, ::cChaseHeightMin * 2)) {
                target.sub(*MR::getSensorPos(shadowSensor), mPosition);
                MR::normalize(&target);
            }
        } else {
            if (MR::isNear(this, shadowPos, ::cChaseHeightMin)) {
                TVec3f up;
                mBaseMtx.getYDir(up);
                if (!MR::isSameDirection(target, up)) {
                    MR::vecKillElement(target, up, &target);
                    MR::normalize(&target);
                }
            }
        }
    }

    if (isUpdateChaseFrontVec(target)) {
        f32 rotateSpeed = mChaseRotateSpeed;
        MR::turnVecToVecCos(&mFront, mFront.copy(), target, rotateSpeed, mGravity);
    } else {
        target.set(mFront);
    }

    updateVelocity();
    updateRotateZ(target);
    return true;
}

bool HomingKiller::isChaseStart() const {
    if (mChaseInvalidTime < 35 || MR::isHiddenModel(this) || !MR::isNearPlayer(this, mChaseStartDist)) {
        return false;
    }

    TVec3f target;
    target.sub(*MR::getPlayerPos(), mPosition);
    if (MR::normalizeOrZero(&target)) {
        return false;
    }

    return MR::cosDegree(mChaseStartAngle * 0.5f) < target.dot(mFront);
}

bool HomingKiller::tryChaseStart() {
    mChaseInvalidTime++;
    if (mChaseInvalidFrame == mChaseInvalidTime) {
        MR::onBind(this);
    }

    if (mDisableChase || mIsLinearShot) {
        return false;
    }

    if (isChaseStart()) {
        MR::deleteEffect(this, mType == Type_Torpedo ? "Bubble" : "Smoke");
        setNerve(GET_NERVE(HomingKiller, HomingKillerNrvChaseStart));
        return true;
    }

    return false;
}

bool HomingKiller::tryFreeze(const Nerve* pUnfreezeNerve) {
    if (MR::isNoBind(this)) {
        return false;
    }

    if (MR::isStarPointerPointing2PAttachTarget(this)) {
        mVelocity.zero();
        mScaleController->startDpdHitVibration();
        MR::startDPDHitSound();
        mUnfreezeNerve = pUnfreezeNerve;
        setNerve(GET_NERVE(HomingKiller, HomingKillerNrvFreeze));
        return true;
    }

    return false;
}

bool HomingKiller::tryBindedBreak() {
    if (mShotBindTime > 0) {
        mShotBindTime--;
    }

    if (MR::isBinded(this)) {
        if (mShotBindTime > 0 && MR::isBindedGround(this)) {
            return false;
        }

        if (MR::isBindedGround(this)) {
            HitSensor* groundSensor = MR::getGroundSensor(this);
            if (::isSensorType(groundSensor, ::cSensorTableAttackIfBinded, ARRAY_SIZE(::cSensorTableAttackIfBinded))) {
                MR::sendMsgEnemyAttackExplosion(groundSensor, getSensor("Body"));
            }
        }

        if (MR::isBindedWall(this)) {
            HitSensor* wallSensor = MR::getWallSensor(this);
            if (::isSensorType(wallSensor, ::cSensorTableAttackIfBinded, ARRAY_SIZE(::cSensorTableAttackIfBinded))) {
                MR::sendMsgEnemyAttackExplosion(wallSensor, getSensor("Body"));
            }
        }

        if (MR::isBindedRoof(this)) {
            HitSensor* roofSensor = MR::getRoofSensor(this);
            if (::isSensorType(roofSensor, ::cSensorTableAttackIfBinded, ARRAY_SIZE(::cSensorTableAttackIfBinded))) {
                MR::sendMsgEnemyAttackExplosion(roofSensor, getSensor("Body"));
            }
        }

        MR::offBind(this);
        return true;
    }

    return false;
}

bool HomingKiller::tryToExplosion(HitSensor* pSender, HitSensor* pReceiver) {
    if (::isSensorType(pReceiver, ::cSensorTableTryExplosion, ARRAY_SIZE(::cSensorTableTryExplosion)) || MR::isSensorEnemy(pReceiver)) {
        bool isShotEnd = false;
        if (isNerve(GET_NERVE(HomingKiller, HomingKillerNrvShot)) && MR::isGreaterStep(this, mShotTime)) {
            isShotEnd = true;
        }

        if (isShotEnd && MR::sendArbitraryMsg(9, pReceiver, pSender)) {
            return true;
        }

        if (MR::sendMsgEnemyAttackExplosion(pReceiver, pSender)) {
            return true;
        }
    }

    return false;
}

void HomingKiller::sendMsgExplosionToNearActor() {
    HitSensor* receiver;
    HitSensor* eyeSensor = getSensor("eye");

    for (s32 idx = 0; idx < eyeSensor->mSensorCount; idx++) {
        receiver = eyeSensor->mSensors[idx];
        if (MR::isNear(eyeSensor, receiver, ::cExplosionDistance)) {
            tryToExplosion(getSensor("Body"), receiver);
        }
    }
}

bool HomingKiller::isUpdateChaseFrontVec(const TVec3f& rFront) const {
    if (!isNerve(GET_NERVE(HomingKiller, HomingKillerNrvGoToTarget))) {
        return true;
    }

    TVec3f target;
    target.sub(mTargetSensor->mPosition, mPosition);
    if (MR::normalizeOrZero(&target)) {
        return false;
    }

    return target.dot(mFront) < target.dot(rFront);
}

// NONMATCHING: vector temporary stack-slot order.
void HomingKiller::calcFrontVecToTarget(TVec3f* pFront) const {
    TVec3f playerUp;
    TVec3f targetPos;
    MR::getPlayerUpVec(&playerUp);
    MR::normalize(&playerUp);
    playerUp.scale(mType == Type_Torpedo ? ::cChaseTargetHeightTorpedo : ::cChaseTargetHeight);
    targetPos.add(*MR::getPlayerPos(), playerUp);
    MR::fn_8000E960(pFront, this, &targetPos);
}

bool HomingKiller::isValidShowModel() const {
    if (MR::isJudgedToClipFrustum(mPosition, 300.0f)) {
        return false;
    }

    if (MR::isJudgedToNearClip(mPosition, mType == Type_MagnumKiller ? ::cNearClipDistanceMagnum : 200.0f)) {
        return false;
    }

    return true;
}

bool HomingKiller::isWaterBreak() const {
    if (MR::isInWater(this, TVec3f(0.0f, 0.0f, 0.0f))) {
        return mType != Type_Torpedo;
    } else {
        return mType == Type_Torpedo;
    }
}

void HomingKiller::updateBaseMtxNoRotateZ() {
    if (MR::isSameDirection(mFront, mUp)) {
        mBaseMtx.setTrans(mPosition);
    } else {
        MR::makeMtxFrontUpPos(&mBaseMtx, mFront, mUp, mPosition);
    }
}

// NONMATCHING: arg evaluation order of the pitch multiply (f2/f0 swap)
void HomingKiller::startMoveLevelSound(bool playAlarm) {
    if (mType == Type_Torpedo) {
        MR::startActionSound(this, "EmLvTorpedoMove");
        if (playAlarm) {
            MR::startActionSound(this, "EmLvTorpedoAlarm", MR::calcDistanceToPlayer(this));
        }
    } else if (mType == Type_MagnumKiller) {
        f32 pitch = MR::getLinerValueFromMinMax(MR::calcDistanceToPlayer(this), 500.0f, 1400.0f, 1.8f, 0.9f);
        MR::startSoundInPlayerDistance(this, "EmLvMagkillerFly", 100.0f * pitch, -1, 2300.0f);
    } else {
        MR::startSoundInPlayerDistance(this, "EmLvKillerFly", -1, -1, 1500.0f);
    }
}

void HomingKiller::setBckRate(f32 rate, bool setPropellerRate) {
    if (mType != Type_MagnumKiller) {
        MR::setBckRate(this, rate);
        if (setPropellerRate && mType == Type_Torpedo) {
            MR::setBckRate(mPropeller, rate);
        }
    }
}

bool HomingKiller::isChasing() const {
    if (isNerve(GET_NERVE(HomingKiller, HomingKillerNrvChaseStart)) || isNerve(GET_NERVE(HomingKiller, HomingKillerNrvChase)) ||
        isNerve(GET_NERVE(HomingKiller, HomingKillerNrvGoToTarget))) {
        return true;
    }

    if (isNerve(GET_NERVE(HomingKiller, HomingKillerNrvFreeze))) {
        if (mUnfreezeNerve == GET_NERVE(HomingKiller, HomingKillerNrvChaseStart) || mUnfreezeNerve == GET_NERVE(HomingKiller, HomingKillerNrvChase) ||
            mUnfreezeNerve == GET_NERVE(HomingKiller, HomingKillerNrvGoToTarget)) {
            return true;
        }
    }

    return false;
}
bool HomingKiller::isNormalNerve() const {
    bool ret = false;
    if (isNerve(GET_NERVE(HomingKiller, HomingKillerNrvAppear)) || isNerve(GET_NERVE(HomingKiller, HomingKillerNrvMoveStart)) ||
        isNerve(GET_NERVE(HomingKiller, HomingKillerNrvMove)) || isNerve(GET_NERVE(HomingKiller, HomingKillerNrvChaseStart)) ||
        isNerve(GET_NERVE(HomingKiller, HomingKillerNrvChase)) || isNerve(GET_NERVE(HomingKiller, HomingKillerNrvFreeze)) ||
        isNerve(GET_NERVE(HomingKiller, HomingKillerNrvGoToTarget))) {
        ret = true;
    }
    return ret;
}

bool HomingKiller::isYoshiHold() const {
    bool ret = false;
    if (isNerve(GET_NERVE(HomingKiller, HomingKillerNrvTongue)) ||
        (isNerve(GET_NERVE(HomingKiller, HomingKillerNrvEaten)) && MR::isLessStep(this, 600)) ||
        (isNerve(GET_NERVE(HomingKiller, HomingKillerNrvShot)) && MR::isLessStep(this, mShotTime))) {
        ret = true;
    }
    return ret;
}

bool HomingKiller::isShotEnd() const {
    bool ret = false;
    if (isNerve(GET_NERVE(HomingKiller, HomingKillerNrvShot)) && MR::isGreaterStep(this, mShotTime)) {
        ret = true;
    }
    return ret;
}

bool HomingKiller::isEaten() const {
    return isNerve(GET_NERVE(HomingKiller, HomingKillerNrvEaten));
}

void HomingKiller::setMovementType(s32 type) {
    mMovementType = type;
}

#pragma opt_propagation off
void HomingKiller::exeAppear() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Start");
        bool a1 = mType == Type_MagnumKiller;
        if (!a1) {
            MR::startBpk(this, "Move");
            MR::startBrk(this, "Move");
        }

        bool a2 = mType == Type_Torpedo;
        if (a2) {
            MR::startAction(mPropeller, "RotateTorpedo");
            mTorpedoLight->appear();
            MR::startAction(mTorpedoLight, "Appear");
        }

        setBckRate(0.0f, true);
    }

    s32 step = getNerveStep();
    if (step >= ::cAppearMoveFrame) {
        step = ::cAppearMoveFrame;
    }

    f32 appearOffset = step * ::cAppearMoveDistance / ::cAppearMoveFrame;
    mPosition.add(mBasePos, mBaseFront * appearOffset);

    if (MR::isLessStep(this, ::cAppearMoveFrame)) {
        MR::startActionSound(this, "EmLvKillerStandby");
    }

    if (MR::isStep(this, ::cAppearMoveFrame)) {
        MR::startActionSound(this, "EmKillerStandbyEnd");
    }

    if (MR::isGreaterStep(this, ::cAppearMoveFrame) && MR::isLessStep(this, ::cAppearMoveFrame + ::cAppearRumbleFrame)) {
        s32 step = getNerveStep() - ::cAppearMoveFrame;
        f32 scl = MR::sinDegree(MR::repeatDegree(step * ::cAppearRumbleSpeed));
        f32 rumbleOffset = (::cAppearRumbleFrame - step) * (scl * ::cAppearRumbleWidth) / ::cAppearRumbleFrame;

        mPosition.add(mBaseFront * rumbleOffset);
    }

    mFront.set(mBaseFront);

    if (MR::isStep(this, ::cAppearMoveFrame + ::cAppearRumbleFrame + ::cAppearStopFrame)) {
        setBckRate(1.0f, true);
        MR::emitEffect(this, "Shoot");
        MR::validateShadow(this, nullptr);
        bool a3 = mType == Type_MagnumKiller;
        if (a3) {
            MR::startActionSound(this, "EmMagkillerFiring");
            setNerve(GET_NERVE(HomingKiller, HomingKillerNrvMove));
        } else {
            MR::startActionSound(this, "EmKillerFiring");
            setNerve(GET_NERVE(HomingKiller, HomingKillerNrvMoveStart));
        }
    }
}
#pragma opt_propagation on

void HomingKiller::exeMoveStart() {
    if (processMove() && !tryFreeze(GET_NERVE(HomingKiller, HomingKillerNrvMoveStart)) && !tryChaseStart() && MR::isActionEnd(this)) {
        setNerve(GET_NERVE(HomingKiller, HomingKillerNrvMove));
    }
}

void HomingKiller::exeMove() {
    if (MR::isFirstStep(this)) {
        MR::tryStartBck(this, "Move");
    }

    if (processMove() && !tryFreeze(GET_NERVE(HomingKiller, HomingKillerNrvMove)) && !tryChaseStart()) {
        return;
    }
}

void HomingKiller::exeChaseStart() {
    if (MR::isFirstStep(this) && MR::tryStartBck(this, "ChaseStart")) {
        MR::startBpk(this, "Chase");
        MR::startBrk(this, "Chase");
        MR::startActionSound(this, "EmKillerJet");
        initSpeed();
    }

    if (processChase() && !tryFreeze(GET_NERVE(HomingKiller, HomingKillerNrvChaseStart)) && MR::isActionEnd(this)) {
        setNerve(GET_NERVE(HomingKiller, HomingKillerNrvChase));
    }
}

void HomingKiller::exeChase() {
    if (MR::isFirstStep(this)) {
        MR::tryStartBck(this, "Chase");
    }

    if (processChase() && !tryFreeze(GET_NERVE(HomingKiller, HomingKillerNrvChase))) {
        return;
    }
}

void HomingKiller::exeFreeze() {
    if (MR::isFirstStep(this)) {
        MR::emitEffect(this, "Touch");
        if (mType != Type_MagnumKiller) {
            MR::setBckRate(this, 0.0f);
        }
    }

    if (tryBindedBreak()) {
        setNerve(GET_NERVE(HomingKiller, HomingKillerNrvBreak));
        return;
    }

    MR::startDPDFreezeLevelSound(this);

    if (isChasing()) {
        TVec3f target;
        calcFrontVecToTarget(&target);
        MR::turnVecToVecCos(&mFront, mFront.copy(), target, ::cFreezeRotateCosineMax, mGravity);
        updateRotateZ(target);
    }

    if (MR::isStarPointerPointing2PAttachTarget(this)) {
        setNerve(GET_NERVE(HomingKiller, HomingKillerNrvFreeze));
        return;
    }

    if (MR::isStep(this, ::cFreezeFrame)) {
        mScaleController->startAnim();
        MR::deleteEffect(this, "Touch");
        setBckRate(1.0f, true);
        setNerve(mUnfreezeNerve);
    }
}

void HomingKiller::exeBreak() {
    if (MR::isFirstStep(this)) {
        sendMsgExplosionToNearActor();
        MR::hideModel(this);
        MR::invalidateHitSensors(this);
        MR::offBind(this);
        mVelocity.zero();
        MR::deleteEffectAll(this);

        if (mTorpedoLight != nullptr) {
            mTorpedoLight->kill();
        }

        MR::emitEffect(this, "Explosion");

        if (MR::isBindedGround(this)) {
            MR::calcGravity(this);
            if (mType == Type_HomingKiller) {
                if (!MR::getGroundSensor(this)->isType(76)) {
                    if (mGravity.dot(MR::getGroundNormal(this)) < ::cSandColumnEmitDot) {
                        MR::emitEffect(this, "SandColumn");

                        TVec3f up;
                        up.negate(mGravity);
                        TVec3f shadowPos;
                        if (MR::isShadowProjected(this, nullptr)) {
                            MR::getShadowProjectionPos(this, nullptr, &shadowPos);
                        } else {
                            shadowPos.set(mPosition);
                        }

                        if (MR::isSameDirection(mFront, up)) {
                            MR::makeMtxUpNoSupportPos(&mEffectMtx, up, shadowPos);
                        } else {
                            MR::makeMtxUpFrontPos(&mEffectMtx, up, mFront, shadowPos);
                        }
                    }
                }
            }
        }

        if (mType == Type_Torpedo) {
            MR::startActionSound(this, "EmTorpedoExplosion");
            MR::releaseSoundHandle(this, "SE_EM_TORPEDO_EXPLOSION");
        } else {
            MR::startActionSound(this, "EmKillerExplosion");
            MR::releaseSoundHandle(this, "SE_EM_KILLER_EXPLOSION");
        }

        MR::startRumbleWithShakeCameraWeak(this, "強", "中", ::cCameraShakeDistance, ::cCameraShakeDistance * 2);
    }

    if (!MR::isEffectValid(this, "Explosion")) {
        if (mType != Type_HomingKiller || !MR::isEffectValid(this, "SandColumn")) {
            kill();
        }
    }
}

void HomingKiller::exeGoToTarget() {
    startMoveLevelSound(false);

    if (processChase() && !tryFreeze(GET_NERVE(HomingKiller, HomingKillerNrvGoToTarget))) {
        HitSensor* eyeSensor = getSensor("eye");
        bool foundTarget = false;
        for (s32 idx = 0; idx < eyeSensor->mSensorCount; idx++) {
            if (eyeSensor->mSensors[idx] == mTargetSensor) {
                foundTarget = true;
                break;
            }
        }

        if (!foundTarget) {
            setNerve(GET_NERVE(HomingKiller, HomingKillerNrvChase));
        }
    }
}

void HomingKiller::exeEaten() {
    if (MR::isFirstStep(this)) {
        MR::fn_80422190();
    }

    TPos3f mtx;
    MR::calcYoshiTongueTipMtx(&mtx);
    mtx.getTrans(mPosition);
    mtx.getZDir(mFront);
    mFront.negate();
    calcAnmMtx();

    if (mType == Type_HomingKiller) {
        if (MR::isGreaterStep(this, 540)) {
            MR::startActionSound(this, "AlartHigh");
        } else if (MR::isGreaterStep(this, 420)) {
            MR::startActionSound(this, "AlartMiddle");
            MR::fn_80422210();
        } else {
            MR::startActionSound(this, "AlartNormal");
        }
    }
}

// NONMATCHING: matrix/quaternion temporary stack slots.
void HomingKiller::exeShot() {
    if (MR::isFirstStep(this)) {
        MR::showModel(this);
        MR::tryStartBck(this, "chaseStart", nullptr);
        MR::startActionSound(this, "YoshiShot");
        mMoveTime = 0;

        TPos3f mtx;
        MR::calcYoshiTongueTipMtx(&mtx);
        mtx.getTrans(mPosition);
        MR::getPlayerThrowVec(&mFront);
        TVec3f negGravity;
        negGravity.negate(mGravity);
        mUp = negGravity;
        mScale.zero();
        mShotBindTime = 20;
        MR::onBind(this);
    }

    startMoveLevelSound(false);
    MR::startActionSound(this, "YoshiShotLV");
    mScale.setAll< f32 >(0.1f + 0.9f * mScale.x);

    if (mShotBindTime > 0 && MR::isBindedGround(this)) {
        TQuat4f quat;
        quat.setRotate(mUp, MR::getGroundNormal(this));
        mUp.set(MR::getGroundNormal(this));
        quat.rotate(mFront);
        TVec3f offset(mUp);
        offset.scale(10.0f);
        mPosition.add(offset);
    }

    if (processMove()) {
        mVelocity.x *= 2.0f;
        mVelocity.y *= 2.0f;
        mVelocity.z *= 2.0f;
    }
}
