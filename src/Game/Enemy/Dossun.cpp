#include "Game/Enemy/Dossun.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/ActorStateParamScale.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util/ActorCameraUtil.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/ActorShadowUtil.hpp"
#include "Game/Util/ActorSwitchUtil.hpp"
#include "Game/Util/EffectUtil.hpp"
#include "Game/Util/EventUtil.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/ModelUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/SoundUtil.hpp"

namespace {
    // sShadowBoxSize
    const f32 sUpperHeight = 1000.0f;
    const s32 sUpperFrame = 180;
    const f32 sFallingSpeed = 30.0f;
    const s32 sOnGroundFrame = 120;
    const f32 sRisingSpeed = 10.0f;
    const f32 sQuickRatio = 2.5f;
    const f32 sCamShakeDistanceStrong = 2000.0f;
    const f32 sCamShakeDistanceWeak = 3500.0f;
    const f32 sShadowDropDepth = 0.0f;
};  // namespace

namespace NrvDossun {
    NEW_NERVE(DossunNrvReady, Dossun, Ready);
    NEW_NERVE(DossunNrvUpper, Dossun, Upper);
    NEW_NERVE(DossunNrvFallSign, Dossun, FallSign);
    NEW_NERVE(DossunNrvFalling, Dossun, Falling);
    NEW_NERVE(DossunNrvOnGround, Dossun, OnGround);
    NEW_NERVE(DossunNrvRising, Dossun, Rising);
};  // namespace NrvDossun

Dossun::Dossun(const char* pName) : LiveActor(pName) {
    mState = nullptr;
    _94.x = 0.0f;
    _94.y = 0.0f;
    _94.z = 0.0f;
    _A0.x = 0.0f;
    _A0.y = 0.0f;
    _A0.z = 0.0f;
    mFallingTime = 0;
    mHoldTime = 0;
    mMovementDist = ::sUpperHeight;
    mRisenWaitTime = ::sUpperFrame;
    mOnGroundTime = ::sOnGroundFrame;
    mShadowType = -1;
    mHasShadow = false;
}

void Dossun::init(const JMapInfoIter& rIter) {
    MR::initUseStageSwitchReadA(this, rIter);
    MR::useStageSwitchAwake(this, rIter);
    initMapToolInfo(rIter);
    MR::initActor(this, rIter, false);
    MR::startBva(this, "Wait");
    if (mHasShadow) {
        MR::connectToSceneCollisionEnemyStrongLight(this);
    } else {
        MR::connectToSceneCollisionEnemyNoShadowedMapObjStrongLight(this);
    }

    MR::initLightCtrl(this);
    initCollision(rIter);
    initEffectKeeper(0, nullptr, false);
    initSound(4, nullptr, nullptr, TVec3f(0.0f));
    initShadow();
    mState = new ActorStateParamScale(this);
    mState->init(rIter);
    initNerve(GET_NERVE(Dossun, DossunNrvUpper), 0);
    MR::setClippingTypeSphereContainsModelBoundingBox(this, 100.0f);
    s16 frameMax = MR::getBckFrameMax(this, "FallStart");
    mRisenWaitTime = (mRisenWaitTime - frameMax <= 0) ? 0 : (mRisenWaitTime - frameMax);
    appear();
}

void Dossun::initCollision(const JMapInfoIter& rIter) {
    initHitSensor(1);
    HitSensor* pSensor = MR::addBodyMessageSensorMapObjPress(this);
    const char* pObjName = nullptr;
    MR::getObjectName(&pObjName, rIter);
    MR::initCollisionParts(this, pObjName, pSensor, nullptr);
}

void Dossun::control() {
    mState->calcHostVelocity();
}

void Dossun::movement() {
    mState->update();
    LiveActor::movement();
    mState->resetHostVelocity();
}

void Dossun::appear() {
    LiveActor::appear();
    MR::invalidateHitSensor(this, "Body");
    calcParameters();
    MR::setPosition(this, _94);
    MR::setSensorType(this, nullptr, 113);
    if (MR::isValidSwitchA(this)) {
        setNerve(GET_NERVE(Dossun, DossunNrvReady));
    } else {
        setNerve(GET_NERVE(Dossun, DossunNrvUpper));
    }
}

void Dossun::initMapToolInfo(const JMapInfoIter& rIter) {
    MR::initDefaultPos(this, rIter);
    MR::getJMapInfoArg0NoInit(rIter, &mMovementDist);
    MR::getJMapInfoArg1NoInit(rIter, &mRisenWaitTime);
    MR::getJMapInfoArg2NoInit(rIter, &mShadowType);
    MR::getJMapInfoArg4NoInit(rIter, &mOnGroundTime);
    s32 shadowType = -1;
    MR::getJMapInfoArg3NoInit(rIter, &shadowType);
    mHasShadow = shadowType != -1;
    MR::setGroupClipping(this, rIter, 16);
    _A0.set(mPosition);
}

void Dossun::initShadow() {
    if (mShadowType != 1) {
        calcParameters();
        if (mShadowType == 0) {
            MR::initShadowFromCSVWithInitShadowVolumeSphere(this, "Shadow2D");
        } else {
            MR::initShadowFromCSVWithInitShadowVolumeSphere(this, "Shadow");
            MR::excludeCalcShadowToMyCollision(this, nullptr);
        }
    }
}

void Dossun::calcParameters() {
    TPos3f mtx;
    mtx.set(getBaseMtx());
    TVec3f offset(0.0f, mMovementDist, 0.0f);
    mtx.mult33(offset);
    _94.add(_A0, offset);

    mFallingTime =
        static_cast< s16 >(mMovementDist / (MR::isGalaxyQuickCometAppearInCurrentStage() ? ::sFallingSpeed * ::sQuickRatio : ::sFallingSpeed));
    mHoldTime = static_cast< s16 >(mMovementDist / (MR::isGalaxyQuickCometAppearInCurrentStage() ? ::sRisingSpeed * ::sQuickRatio : ::sRisingSpeed));
}

s32 Dossun::getStepUntilRising() {
    if (MR::isGalaxyQuickCometAppearInCurrentStage()) {
        return mOnGroundTime / ::sQuickRatio;
    }

    return mOnGroundTime;
}

s32 Dossun::getUpperFrame() const {
    if (MR::isGalaxyQuickCometAppearInCurrentStage()) {
        f32 frame = MR::getBckFrameMax(this, "FallStart") / ::sQuickRatio;
        s32 waitFrames = mRisenWaitTime / ::sQuickRatio;
        if (waitFrames - static_cast< s32 >(frame) <= 0) {
            return 0;
        } else {
            return waitFrames - static_cast< s32 >(frame);
        }
    }

    return mRisenWaitTime;
}

void Dossun::exeReady() {
    if (MR::isOnSwitchA(this)) {
        setNerve(GET_NERVE(Dossun, DossunNrvFallSign));
    }
}

void Dossun::exeUpper() {
    if (MR::isFirstStep(this)) {
        MR::setPosition(this, _94);
        MR::setSensorType(this, nullptr, 113);
    }

    if (MR::isStep(this, getUpperFrame())) {
        setNerve(GET_NERVE(Dossun, DossunNrvFallSign));
    }
}

void Dossun::exeFallSign() {
    if (mState->isFirstStep()) {
        MR::startAction(this, "FallStart");
        MR::startBva(this, "Attack");

        if (MR::isGalaxyQuickCometAppearInCurrentStage()) {
            MR::setBckRate(this, ::sQuickRatio);
            MR::setBvaRate(this, ::sQuickRatio);
        }

        MR::startActionSound(this, "OjDossunFallSign");
    }

    if (MR::isActionEnd(this)) {
        setNerve(GET_NERVE(Dossun, DossunNrvFalling));
    }
}

void Dossun::exeFalling() {
    f32 ease = MR::getEaseInValue(mState->getNerveStepRate(), 0.0f, 1.0f, mFallingTime);
    mPosition.lerp(_94, _A0, ease);
    MR::startActionSound(this, "OjLvDossunFall");
    if (mState->isStep(mFallingTime)) {
        setNerve(GET_NERVE(Dossun, DossunNrvOnGround));
    }
}

void Dossun::exeOnGround() {
    if (MR::isFirstStep(this)) {
        MR::setPosition(this, _A0);
        MR::startRumbleWithShakeCameraNormalWeak(this, "中", "弱", ::sCamShakeDistanceStrong, ::sCamShakeDistanceWeak);
        MR::startActionSound(this, "OjDossunLand");
        MR::emitEffect(this, "Land");
    }

    if (MR::isStep(this, getStepUntilRising())) {
        setNerve(GET_NERVE(Dossun, DossunNrvRising));
    }
}

void Dossun::exeRising() {
    if (mState->isFirstStep()) {
        MR::startBva(this, "Wait");
        MR::tryEmitEffect(this, "Rise");
        if (MR::isValidSwitchParam(this) && MR::isOnSwitchParam(this)) {
            MR::setSensorType(this, nullptr, 64);
        }
    }

    f32 ease = MR::getEaseInOutValue(mState->getNerveStepRate(), 0.0f, 1.0f, mHoldTime);
    mPosition.lerp(_A0, _94, ease);
    MR::startActionSound(this, "OjLvDossunUpper");
    if (mState->isStep(mHoldTime)) {
        MR::startActionSound(this, "OjDossunStop");
        setNerve(GET_NERVE(Dossun, DossunNrvUpper));
    }
}
