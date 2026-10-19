#define JGEOMETRY_QUAT_ASSIGN_NOINLINE
#pragma auto_inline off

#include "Game/MapObj/SpinDriver.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/MapObj/SpinDriverCamera.hpp"
#include "Game/MapObj/SpinDriverShootPath.hpp"
#include "Game/MapObj/SpinDriverUtil.hpp"
#include "Game/Util/ActorMovementUtil.hpp"
#include "Game/Util/BaseMatrixFollowTargetHolder.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/ActorShadowUtil.hpp"
#include "Game/Util/ActorSwitchUtil.hpp"
#include "Game/Util/AreaObjUtil.hpp"
#include "Game/Util/DemoUtil.hpp"
#include "Game/Util/EffectUtil.hpp"
#include "Game/Util/GamePadUtil.hpp"
#include "Game/Util/GravityUtil.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/JointUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/ModelUtil.hpp"
#include "Game/Util/MtxUtil.hpp"
#include "Game/Util/NerveUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include "Game/Util/SupportPlayUtil.hpp"
#include "Inline.hpp"
#include "Game/Util/ActionSoundUtil.hpp"
#include <cmath>
#include <cstring>
#include "Game/Util/PlayerUtil.hpp"

#include "Game/Util/StringUtil.hpp"





namespace {
    // static const f32 sShadowMaxHeight = _;
    // static const f32 sShadowHeightMargin = _;
    // static const f32 sRotateYCanJumpAccel = _;
    // static const f32 sRotateYActiveAccel = _;
    // static const f32 sRotateYSwingSignAccel = _;
    // static const f32 sMaxSwingSignRotateYSpeed = _;
    // static const f32 sRotateYFreq = _;
    // static const f32 sMaxBindStartSpeed = _;
    // static const f32 sCaptureRange = _;
    // static const f32 sCaptureAgainRange = _;
    // static const f32 sCaptureTime = _;
    // static const f32 sCaputureEndDistance = _;
    // static const f32 sCaptureToCenterAccel = _;
    // static const f32 sCaptureSpeedDownRange = _;
    // static const f32 sCaptureFreq = _;
    // static const f32 sRotateYCaptureAccel = _;
    // static const f32 sJumpReadyTime = _;
    // static const f32 sCoolDownTime = _;
    // static const f32 sTakeBackDistance = _;
    // static const f32 sLandRotateStartRate = _;
    // static const f32 sLandRotateEndRate = _;
    // static const f32 sFallMotionStartRate = _;
    // static const f32 sFlyMotionInterFrame = _;
    // static const f32 sPreLandMotionInterFrame = _;
    // static const f32 sFallMotionInterFrame = _;
    static s32 cSpaceFlyStartFrame = 10;
    static s32 cSpaceFlyEndFrame = 50;
    // static const f32 sJumpWaitTime = _;
};  // namespace

namespace NrvSpinDriver {
    NEW_NERVE(SpinDriverNrvTryDemo, SpinDriver, TryDemo);
    NEW_NERVE(SpinDriverNrvNonActive, SpinDriver, NonActive);
    NEW_NERVE(SpinDriverNrvAppear, SpinDriver, Appear);
    NEW_NERVE(SpinDriverNrvWait, SpinDriver, Wait);
    NEW_NERVE(SpinDriverNrvCapture, SpinDriver, Capture);
    NEW_NERVE(SpinDriverNrvShootStart, SpinDriver, ShootStart);
    NEW_NERVE(SpinDriverNrvShoot, SpinDriver, Shoot);
    NEW_NERVE(SpinDriverNrvCoolDown, SpinDriver, CoolDown);
};  // namespace NrvSpinDriver

SpinDriver::SpinDriver(const char* pName)
    : LiveActor(pName), mBindActor(), mShootPath(), mSpinDriverCamera(), mBindStartQuat(0, 0, 0, 1), mBindQuat(0, 0, 0, 1), mBindPos(0, 0, 0), mBindVelocity(0, 0, 0), mFrontDir(0, 0, 1),
      mSideDir(0, 0, 0), mUpDir(0, 1, 0), mBindStartPos(0, 0, 0), mRotateYAngle(), mJumpSpeed(40.0f), mRotateYSpeed(), mShadowCenter(0, 0, 0), mBindRate(), mShadowLength(-1.0f), mShootFrames(300), mFlyLoopFrame(-1), mFlyEndFrame(240),
      mFallFrame(270), mInitPoseParam(), mIsCaptureEnabled(true), mRushTimer(), mIsRushActive(), mIsCaptureArmed(true), mIsSilentAppear() {
    mFollowMtx.identity();
}

void SpinDriver::init(const JMapInfoIter& rIter) {
    MR::initDefaultPos(this, rIter);
    MR::initActor(this, rIter, "SpinDriver", false);

    MR::connectToSceneNoSilhouettedMapObjStrongLight(this);

    initShootPath(rIter);

    MR::offCalcGravity(this);

    TVec3f gravityVector;
    MR::calcGravityVectorOrZero(this, &gravityVector, nullptr, 0);

    if (MR::isNearZero(gravityVector)) {
        gravityVector.set< f32 >(0.0f, -1.0f, 0.0f);
    }

    mGravity.set(gravityVector);

    initHitSensor(1);
    MR::addHitSensor(this, "Body", ATYPE_SPIN_DRIVER_BIND, 16, 300.0f, TVec3f(0.0f, 0.0f, 0.0f));

    initEffectKeeper(0, nullptr, false);

    initSound(6, "SpinDriver", nullptr, TVec3f(0.0f));

    initEventCamera(rIter);

    initNerve(GET_NERVE(SpinDriver, SpinDriverNrvWait));

    MR::initShadowFromCSVWithInitShadowVolumeSphere(this, "SpinDriverShadow");

    initParamFromJMapInfo(rIter);

    MR::setClippingFar200m(this);

    MR::makeMtxTR(mFollowMtx, this);
    MR::addBaseMatrixFollowTarget(this, rIter, &mFollowMtx,
        MR::createBaseMatrixFollowValidateDelegator(this, &SpinDriver::isValidFollow));

    if (MR::useStageSwitchSyncAppear(this, rIter)) {
        mSpinDriverCamera->initAppearCamera(rIter, this);
    }

    MR::useStageSwitchAwake(this, rIter);
}

void SpinDriver::initAfterPlacement() {
    SpinDriverUtil::setShadowAndClipping(this, &mShadowCenter, mShadowLength, 300.0f, &mClippingRadius);
}

void SpinDriver::initParamFromJMapInfo(const JMapInfoIter& rIter) {
    if (mShootPath == nullptr) {
        MR::getJMapInfoArg0NoInit(rIter, &mJumpSpeed);
    } else {
        MR::getJMapInfoArg0NoInit(rIter, &mShootFrames);
    }

    mShadowLength = -1.0f;
    MR::getJMapInfoArg1NoInit(rIter, &mShadowLength);

    s32 arg2 = 0;

    if (MR::getJMapInfoArg2NoInit(rIter, &arg2)) {
        mIsCaptureEnabled = arg2 != 0;
    }

    s32 arg4 = 0;

    if (MR::getJMapInfoArg4NoInit(rIter, &arg4)) {
        mIsSilentAppear = !(arg4 - 1);
    }
}

void SpinDriver::initShootPath(const JMapInfoIter& rIter) {
    if (MR::isConnectedWithRail(rIter)) {
        mShootPath = new SpinDriverShootPath();

        mInitPoseParam = 0.0f;
        MR::getJMapInfoArg5NoInit(rIter, &mInitPoseParam);

        mShootPath->initUsingParabolic(rIter, mPosition);
        mShootPath->calcInitPose(&mUpDir, &mFrontDir, &mSideDir, mInitPoseParam);
    } else {
        TPos3f rotateMtx;
        MR::makeMtxRotate(rotateMtx, mRotation);
        rotateMtx.getXYZDir(mSideDir, mUpDir, mFrontDir);
    }
}

void SpinDriver::initEventCamera(const JMapInfoIter& rIter) {
    mSpinDriverCamera = new SpinDriverCamera();
    mSpinDriverCamera->init(rIter, this);

    if (MR::isConnectedWithRail(rIter)) {
    }
}

void SpinDriver::appear() {
    LiveActor::appear();

    MR::invalidateClipping(this);

    if (mSpinDriverCamera->isUseAppearCamera(this)) {
        MR::requestStartDemo(this, "出現", GET_NERVE(SpinDriver, SpinDriverNrvAppear), GET_NERVE(SpinDriver, SpinDriverNrvTryDemo));
    } else {
        setNerve(GET_NERVE(SpinDriver, SpinDriverNrvAppear));
    }
}

void SpinDriver::makeActorAppeared() {
    LiveActor::makeActorAppeared();
    setNerve(GET_NERVE(SpinDriver, SpinDriverNrvWait));
}

void SpinDriver::makeActorDead() {
    if (mBindActor != nullptr) {
        cancelBind();
    }

    LiveActor::makeActorDead();
}

void SpinDriver::control() {
    if (!mIsCaptureArmed && mPosition.squared(*MR::getPlayerPos()) > 122500.0f) {
        mIsCaptureArmed = true;
    }

    if (mRushTimer > 0) {
        mRushTimer--;
    }

    mIsRushActive = mRushTimer > 0;

    mRotateYAngle += mRotateYSpeed;
    mRotateYAngle = MR::repeat(mRotateYAngle, -MR::pi(), 2 * MR::pi());

    mRotateYSpeed *= 0.95f;
}

void SpinDriver::calcAndSetBaseMtx() {
    TPos3f baseMtx;
    MR::makeMtxUpFrontPos(&baseMtx, mUpDir, mFrontDir, mPosition);

    TPos3f yMtx;
    yMtx.identity();
    yMtx.setEulerY(mRotateYAngle);

    baseMtx.concat(baseMtx, yMtx);
    MR::setBaseTRMtx(this, baseMtx);
}

bool SpinDriver::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgIsRushTakeOver(msg)) {
        mRushTimer = 60;
        return canBind(pSender);
    }

    if (MR::isMsgAutoRushBegin(msg)) {
        mRushTimer = 60;

        if (startBind(pSender)) {
            return true;
        }
    }

    if (MR::isMsgRushCancel(msg)) {
        if (isNerve(GET_NERVE(SpinDriver, SpinDriverNrvShootStart))) {
            return false;
        }

        if (mBindActor != nullptr) {
            mBindActor = nullptr;

            cancelCamera();

            return true;
        }
    }

    if (MR::isMsgRushForceCancel(msg) && mBindActor != nullptr) {
        mBindActor = nullptr;

        cancelCamera();

        return true;
    }

    if (MR::isMsgUpdateBaseMtx(msg) && mBindActor != nullptr) {
        calcBindActorMatrix();
        return true;
    }

    return false;
}

bool SpinDriver::tryStartShoot() {
    bool isSwingOr2P = MR::isPadSwing(WPAD_CHAN0) || MR::isPlayerPointedBy2POnTriggerButton();

    if (isSwingOr2P) {
        MR::startLevelSoundPlayer("SpinAttack", -1, -1);

        if (MR::isInAreaObj("Water", mPosition)) {
            MR::startActionSound(this, "PmSpinDrvInWater1");
        }

        setNerve(GET_NERVE(SpinDriver, SpinDriverNrvShootStart));

        return true;
    }

    return false;
}

bool SpinDriver::tryEndCapture() {
    if (MR::isGreaterStep(this, 40) && mBindPos.distance(mPosition) < 15.0f) {
        cancelBind();

        mIsCaptureArmed = 0;

        setNerve(GET_NERVE(SpinDriver, SpinDriverNrvWait));

        return true;
    }

    return false;
}

bool SpinDriver::tryShoot() {
    if (MR::isGreaterStep(this, 20)) {
        setNerve(GET_NERVE(SpinDriver, SpinDriverNrvShoot));
        return true;
    }

    return false;
}

bool SpinDriver::tryEndShoot() {
    if (mShootPath == nullptr || MR::isGreaterStep(this, mShootFrames)) {
        MR::endBindAndSpinDriverJump(this, mBindVelocity);

        mBindActor = 0;

        endCamera();

        setNerve(GET_NERVE(SpinDriver, SpinDriverNrvCoolDown));

        return true;
    }

    return false;
}

bool SpinDriver::tryForceCancel() {
    if (mBindActor == nullptr) {
        setNerve(GET_NERVE(SpinDriver, SpinDriverNrvCoolDown));
        return true;
    }

    return false;
}

bool SpinDriver::tryEndCoolDown() {
    if (MR::isGreaterStep(this, 60)) {
        setNerve(GET_NERVE(SpinDriver, SpinDriverNrvWait));
        return true;
    }

    return false;
}

bool SpinDriver::trySwitchOff() {
    if (MR::isValidSwitchAppear(this) && !MR::isOnSwitchAppear(this)) {
        kill();
        return true;
    }

    return false;
}

void SpinDriver::exeTryDemo() {
}

void SpinDriver::exeNonActive() {
    if (MR::isFirstStep(this)) {
        MR::validateClipping(this);

        MR::startAction(this, "Wait");
        MR::startBpk(this, "Wait");
    }

    bool isSwingOr2P = MR::isPadSwing(WPAD_CHAN0) || MR::isPlayerPointedBy2POnTriggerButton();

    if (isSwingOr2P) {
        mRotateYSpeed += 0.05f;

        if (mRotateYSpeed > 0.15f) {
            mRotateYSpeed = 0.15f;
        }
    }
}

void SpinDriver::exeAppear() {
    if (MR::isFirstStep(this)) {
        if (!mIsSilentAppear) {
            MR::startActionSound(this, "SySpinDriverAppear");
            MR::startActionSound(this, "OjSpinDrvAppear");
        }

        MR::startAction(this, "Appear");
        mSpinDriverCamera->startAppearCamera(this, mUpDir, mFrontDir, mPosition);
    }

    if (MR::isActionEnd(this) && MR::isGreaterStep(this, mSpinDriverCamera->getAppearCameraFrames())) {
        setNerve(GET_NERVE(SpinDriver, SpinDriverNrvWait));

        if (mSpinDriverCamera->isUseAppearCamera(this)) {
            MR::endDemoWaitCameraInterpolating(this, "出現");
            mSpinDriverCamera->endAppearCamera(this);
        }
    }
}

void SpinDriver::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::validateClipping(this);

        MR::startAction(this, "Wait");
        MR::startBpk(this, "Wait");
    }

    bool isSwingOr2P = MR::isPadSwing(WPAD_CHAN0) || MR::isPlayerPointedBy2POnTriggerButton();

    if (isSwingOr2P) {
        mRotateYSpeed += 0.05f;
    }

    if (mRushTimer > 0) {
        mRotateYSpeed += 0.008f;

        MR::startActionSound(this, "OjLvSpinDrvShine");

        if (!mIsRushActive) {
            MR::emitEffect(this, "SpinDriverLight");
            MR::startCSSound("CS_SPIN_BIND", nullptr, 0, 0, 1.0f);
        }
    } else {
        MR::deleteEffect(this, "SpinDriverLight");
    }

    if (trySwitchOff()) {
        return;
    }
}

void SpinDriver::exeCapture() {
    if (tryForceCancel()) {
        MR::deleteEffect(this, "SpinDriverLight");
        return;
    }

    if (MR::isFirstStep(this)) {
        MR::startBckPlayer("SpinDriverWait", "SpinDriverCapture");
        MR::startAction(this, "Active");

        MR::emitEffect(this, "SpinDriverLight");
    }

    MR::startActionSound(this, "OjLvSpinDrvShine");
    MR::startActionSound(this, "OjLvSpinDrvCapture");

    moveBindPosToCenter();

    mBindRate = MR::calcNerveRate(this, 40);

    updateBindActorMatrix(mBindRate);

    mRotateYSpeed += 0.008f;

    MR::tryRumblePadWeak(this, WPAD_CHAN0);

    mRushTimer = 60;

    if (!tryStartShoot() && tryEndCapture()) {
        return;
    }
}

void SpinDriver::exeShootStart() {
    if (tryForceCancel()) {
        return;
    }

    if (MR::isFirstStep(this)) {
        mBindStartPos = mBindPos;

        MR::deleteEffect(this, "SpinDriverLight");
        MR::emitEffect(this, "SpinDriverStart");

        MR::startBckPlayer("Spin", "SpinDriverShoot");
        MR::startAction(this, "Active");
        MR::startBpk(this, "Active");

        startCamera();
    }

    if (MR::isStep(this, 4)) {
        MR::startCSSound("CS_SPIN_DRIVE_SHORT", "SE_SY_CS_SPIN_DRV_START", 0, 0, 1.0f);
    }

    f32 progress = MR::clamp(getNerveStep() / 20.0f, 0.0f, 1.0f);
    f32 half = 2.0f * (progress - 0.5f);
    f32 square = half * half;
    f32 val = square * square;

    TVec3f upVec;
    MR::calcUpVec(&upVec, this);

    TVec3f vec50;
    vec50.set(mPosition - upVec * 200.0f * (1.0f - val));

    f32 val2 = MR::clamp(2.0f * progress, 0.0f, 1.0f);
    mBindPos.set(mBindStartPos * (1.0f - val2) + vec50 * val2);

    updateBindActorMatrix(val2 + mBindRate * (1.0f - val2));

    mRotateYSpeed += 0.04f;

    MR::tryRumblePadMiddle(this, 0);

    if (tryShoot()) {
        return;
    }
}

void SpinDriver::exeShoot() {
    if (tryForceCancel()) {
        return;
    }

    if (MR::isFirstStep(this)) {
        calcShootMotionTime();

        if (MR::hasME()) {
            MR::startSystemME("ME_MAGIC");
        } else {
            MR::startActionSound(this, "SySSpinDrvMeAlt");
        }

        MR::startActionSound(this, "OjSpinDrvJump");
        if (MR::isPlayerElementModeYoshi()) {
            MR::startActionSoundPlayer("StarRingStart", -1, -1);
        } else {
            MR::startLevelSoundPlayer("JumpObj", -1, -1);
        }
        if (MR::isSupportPlayActive()) {
            MR::startLevelSoundPlayer("SupportTicoSpinDriverShoot", -1, -1);
        }

        if (MR::isInAreaObj("Water", mPosition)) {
            MR::startActionSound(this, "PmSpinDivInWater2");
        }

        MR::startBckPlayer("SpaceFlyShortStart", "SpinDriverFlyStart");

        MR::shakeCameraNormal();
        MR::tryRumblePadVeryStrong(this, WPAD_CHAN0);
    }

    if (mShootPath != nullptr) {
        updateBindPosition();
        updateCamera();

        calcParabolicBindPose();

        if (MR::isStep(this, mFlyLoopFrame)) {
            MR::startBckPlayer("SpaceFlyShort", "SpinDriverFlyLoop");
        }

        if (MR::isStep(this, mFlyEndFrame)) {
            MR::startBckPlayer("SpaceFlyEnd", "SpinDriverFlyEnd");
        }

        if (MR::isLessStep(this, mFallFrame)) {
            MR::startLevelSoundPlayer("SE_PM_LV_SPIN_DRV_FLY", -1);
        }

        if (MR::isStep(this, mFallFrame)) {
            MR::startBckPlayer("Fall", "SpinDriverFall");
        }
    } else {
        mBindVelocity = mUpDir * mJumpSpeed;
    }

    if (tryEndShoot()) {
        return;
    }
}

void SpinDriver::exeCoolDown() {
    if (MR::isFirstStep(this)) {
        MR::startBpk(this, "Wait");
    }

    if (!tryEndCoolDown() && trySwitchOff()) {
        return;
    }
}

// Target retains the quaternion assignment call; the shared operator currently inlines it.
bool SpinDriver::startBind(HitSensor* pSensor) {
    if (!canStartBind()) {
        return false;
    }

    bool isSwingOr2P = MR::isPadSwing(WPAD_CHAN0) || MR::isPlayerPointedBy2POnTriggerButton();

    if (isSwingOr2P) {
        if (MR::hasME()) {
            MR::startSystemME("ME_MAGIC");
        } else {
            MR::startActionSound(this, "SySSpinDrvMeAlt");
        }

        MR::startLevelSoundPlayer("SpinAttack", -1, -1);

        if (MR::isInAreaObj("Water", mPosition)) {
            MR::startActionSound(this, "PmSpinDrvInWater1");
        }

        setNerve(GET_NERVE(SpinDriver, SpinDriverNrvShootStart));
    } else {
        if (mIsCaptureEnabled && mIsCaptureArmed && mPosition.squared(*MR::getSensorPos(pSensor)) < 57600.0f) {
            setNerve(GET_NERVE(SpinDriver, SpinDriverNrvCapture));
        } else {
            return false;
        }
    }

    mBindActor = MR::getSensorHost(pSensor);
    mBindPos = mBindActor->mPosition;

    mBindVelocity = *MR::getPlayerLastMove();

    f32 mag = mBindVelocity.length();

    if (mag > 40.0f) {
        mBindVelocity *= 40.0f / mag;
    }

    mBindRate = 0.0f;

    TRot3f rotation;
    rotation.set(mBindActor->getBaseMtx());
    rotation.getQuat(mBindStartQuat);

    mBindQuat = mBindStartQuat;

    ::cSpaceFlyStartFrame = MR::getBckFrameMax(mBindActor, "SpaceFlyStart");
    ::cSpaceFlyEndFrame = MR::getBckFrameMax(mBindActor, "SpaceFlyEnd");

    MR::invalidateClipping(this);
    return true;
}

void SpinDriver::cancelBind() {
    if (mBindActor != nullptr) {
        MR::endBindAndPlayerJump(this, mBindVelocity, 0);

        mBindActor = nullptr;

        endCamera();
    }
}

void SpinDriver::updateBindPosition() {
    f32 progress = static_cast< f32 >(getNerveStep()) / mShootFrames;

    TVec3f oldPos(mBindPos);

    TVec3f position;
    mShootPath->calcPosition(&position, progress);

    mBindPos = position;
    mBindVelocity = position - oldPos;
}

void SpinDriver::moveBindPosToCenter() {
    mBindPos += mBindVelocity;

    f32 mag;
    TVec3f dir(mPosition - mBindPos);
    MR::separateScalarAndDirection(&mag, &dir, dir);

    mBindVelocity += dir * 1.5f * (mag / 120.0f);
    mBindVelocity.x *= 0.8f;
    mBindVelocity.y *= 0.8f;
    mBindVelocity.z *= 0.8f;
}

void SpinDriver::updateBindActorMatrix(f32 a1) {
    TPos3f makeMtx;
    MR::makeMtxUpFrontPos(&makeMtx, mUpDir, mFrontDir, mPosition);

    TQuat4f quat;
    makeMtx.getQuat(quat);

    mBindQuat.slerp(mBindStartQuat, quat, a1);
    PSMTXCopy(makeMtx.toMtxPtr(), mFollowMtx);
}

void SpinDriver::calcBindActorMatrix() {
    TPos3f baseMtx;
    baseMtx.setQT(mBindQuat, mBindPos);
    LiveActor* actor = mBindActor;

    if (strcmp(actor->mName, "マリオアクター") == 0) {
        MR::setPlayerBaseMtx(baseMtx.toMtxPtr());
    } else {
        MR::setBaseTRMtx(actor, baseMtx);
    }
}

void SpinDriver::calcParabolicBindPose() {
    if (!MR::isNearZero(mBindVelocity)) {
        TVec3f direction;
        MR::normalize(mBindVelocity, &direction);

        if (mShootFrames > 0) {
            f32 rate = MR::calcNerveRate(this, mShootFrames);
            rate = MR::normalize(rate, static_cast< f32 >(mFlyEndFrame) / mShootFrames, static_cast< f32 >(mFallFrame) / mShootFrames);

            if (rate > 0.0f) {
                TQuat4f rotation;
                const f32 angle = PI * rate;
                rotation.setRotate(mSideDir, angle);
                rotation.rotate(direction);
            }
        }

        turnBindHead(direction, 0.4f);
    }
}

void SpinDriver::turnBindHead(const TVec3f& rVec, f32 f1) {
    TVec3f yDir;
    mBindQuat.getYDir(yDir);

    TQuat4f q;
    q.setRotate(yDir, rVec, f1);
    mBindQuat.mult(q);
    mBindQuat.normalize();
}

void SpinDriver::calcShootMotionTime() {
    if (mShootFrames < ::cSpaceFlyEndFrame + 20) {
        mFlyLoopFrame = -1;
        mFlyEndFrame = 0;
        mFallFrame = mShootFrames;
    } else {
        mFlyEndFrame = mShootFrames - (::cSpaceFlyEndFrame + 20);
        mFallFrame = mShootFrames - 20;
        mFlyLoopFrame = mFlyEndFrame / 5;
    }
}

void SpinDriver::startCamera() {
    if (mSpinDriverCamera != nullptr) {
        mSpinDriverCamera->start(mUpDir, mFrontDir, mPosition);
    }
}

void SpinDriver::updateCamera() {
    if (mSpinDriverCamera != nullptr) {
        mSpinDriverCamera->update(mBindVelocity, mBindPos);
    }
}

void SpinDriver::cancelCamera() NO_INLINE {
    if (mSpinDriverCamera != nullptr) {
        mSpinDriverCamera->cancel();
    }
}

void SpinDriver::endCamera() NO_INLINE {
    if (mSpinDriverCamera != nullptr) {
        mSpinDriverCamera->end();
    }
}

bool SpinDriver::canStartBind() const NO_INLINE {
    return isNerve(GET_NERVE(SpinDriver, SpinDriverNrvWait));
}

bool SpinDriver::canBind(HitSensor* pSensor) const {
    if (!canStartBind()) {
        return false;
    }

    bool isSwingOr2P = MR::isPadSwing(WPAD_CHAN0) || MR::isPlayerPointedBy2POnTriggerButton();

    if (isSwingOr2P) {
        return true;
    }

    if (mIsCaptureEnabled && mIsCaptureArmed && mPosition.squared(*MR::getSensorPos(pSensor)) < 57600.0f) {
        return true;
    }

    return false;
}


bool SpinDriver::isValidFollow(s32) const {
    bool valid = false;
    if (isNerve(GET_NERVE(SpinDriver, SpinDriverNrvShootStart)) || isNerve(GET_NERVE(SpinDriver, SpinDriverNrvShoot))) {
        valid = true;
    }
    return valid;
}

