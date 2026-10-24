#pragma auto_inline off
#include "Game/Ride/Pole.hpp"
#include "Game/Util/ActorAnimUtil.hpp"
#include "Game/Util/SceneUtil.hpp"
#include <JSystem/J3DGraphAnimator/J3DAnimation.hpp>
#include "Game/Util/ActionSoundUtil.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util/ActorCameraUtil.hpp"
#include "Game/Util/ActorMovementUtil.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/ActorSwitchUtil.hpp"
#include "Game/Util/DemoUtil.hpp"
#include "Game/Util/GamePadUtil.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/JointUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/ModelUtil.hpp"
#include "Game/LiveActor/ModelManager.hpp"
#include "Game/Util/MtxUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include "Game/Util/StringUtil.hpp"
#include <JSystem/JMath.hpp>
#include <cstring>

namespace {
    static const s32 sStepShakeInterrupt = 30;
    static const f32 sSensorOffsetY = 50.0f;
    // static const f32 sPlayerSpeedToRideMin =
    static const f32 sPlayerDistanceToValid = 300.0f;
    static const f32 sPlayerSpeedToCatchMin = 6.0f;
    static const f32 sPlayerSpeedToCatchMax = 10.0f;
    static const f32 sBindStartRotateSpeed = 5.0f;
    static const f32 sBindStartRotateSpeedFast = 9.0f;
    static const f32 sInvalidPlayerSpeedMaxXZ = 5.0f;
    static const f32 sInvalidPlayerSpeedMinY = 1.0f;
    static const s32 sStepInvalid = 30;
    static const f32 sClimbUpSpeedRate = 6.0f;
    static const f32 sFallDownAccelRate = 0.3f;
    static const f32 sFallDownSpeedMaxY = 15.0f;
    static const f32 sSquareFallDownAccelRate = 0.5f;
    static const f32 sSquareFallDownSpeedMaxY = 18.0f;
    static const f32 sTurnStickMin = 0.8f;
    static const f32 sRotateSpeedRate = 2.5f;
    static const f32 sClimbUpRotateSpeedRate = 0.0f;
    static const f32 sFallDownRotateSpeedRate = 0.8f;
    static const f32 sJumpSpeedXZ = 10.0f;
    static const f32 sJumpSpeedY = 25.0f;
    static const s32 sStepJumpReverseHandstandStart = 25;
    static const s32 sStepJumpReverseHandstandEnd = 25;
    static const f32 sSquareJumpPosOffset = 80.0f;
    static const s32 sStepSquareJumpChangeDir = 7;
    static const f32 sEndHandstandStickY = 0.8f;
    static const f32 sFallDownStickMin = 0.3f;
    static const f32 sLandingHeightMin = 120.0f;
    static const f32 sLandingHeightMinSquare = 50.0f;
    static const f32 sPlayerPosOffsetAtLanding = 30.0f;
    static const f32 sClimbUpAnimFrameRate = 1.75f;
    static const f32 sSquareClimbUpAnimFrameRate = 1.6f;
    static const s32 sStepDemoAppear = 60;
    static const f32 sHeightRate = 100.0f;
    static const f32 sSquareHeightOffset = 100.0f;
};  // namespace


namespace NrvPole {
    NEW_NERVE(PoleNrvDemoAppear, Pole, DemoAppear);
    NEW_NERVE(PoleNrvFree, Pole, Free);
    NEW_NERVE(PoleNrvFreeInvalid, Pole, FreeInvalid);
    NEW_NERVE(PoleNrvBindStart, Pole, BindStart);
    NEW_NERVE(PoleNrvBindStartFast, Pole, BindStart);
    NEW_NERVE(PoleNrvBindWait, Pole, BindWait);
    NEW_NERVE(PoleNrvBindTurnLeft, Pole, BindTurn);
    NEW_NERVE(PoleNrvBindTurnRight, Pole, BindTurn);
    NEW_NERVE(PoleNrvBindTurnEnd, Pole, BindWait);
    NEW_NERVE(PoleNrvBindClimbUp, Pole, BindClimbUp);
    NEW_NERVE(PoleNrvBindFallDown, Pole, BindFallDown);
    NEW_NERVE(PoleNrvBindHandstandStart, Pole, BindHandstandStart);
    NEW_NERVE(PoleNrvBindHandstandWait, Pole, BindHandstandWait);
    NEW_NERVE(PoleNrvBindHandstandEnd, Pole, BindHandstandEnd);
    NEW_NERVE(PoleNrvBindHandstandTurn, Pole, BindHandstandTurn);
};  // namespace NrvPole

Pole::Pole(const char* pName)
    : LiveActor(pName), mBasePos(0.0f, 0.0f, 0.0f), mTopPos(0.0f, 0.0f, 0.0f), mHeight(0.0f), mTopMargin(0.0f), mDisableHandstand(false),
      mIsSquare(false), m2Way(false), mNoModel(false), mInvertStick(false), mUsePole9mCollision(false), mIsTreeA(false), mIsTreeB(false),
      mIsTwisterTower(false), mClimbCoord(0.0f), mClimbSpeed(0.0f), mRider(nullptr), mCameraInfo(nullptr), mCenterPos(0.0f, 0.0f, 0.0f),
      mSide(1.0f, 0.0f, 0.0f), mUp(0.0f, 1.0f, 0.0f), mFront(0.0f, 0.0f, 1.0f), mTopMtx(nullptr), mBottomMtx(nullptr) {
    mPosMtx.identity();
    mBaseMtx.identity();
}

static inline TVec3f makeZeroVec() {
    TVec3f v;
    v.setAll< f32 >(0.0f);
    return v;
}

void Pole::init(const JMapInfoIter& rIter) {
    MR::initDefaultPos(this, rIter);
    mBasePos.set(mPosition);
    mHeight = ::sHeightRate * mScale.y;
    mScale.set(1.0f, 1.0f, 1.0f);

    MR::getJMapInfoArg0NoInit(rIter, &mDisableHandstand);
    MR::getJMapInfoArg1NoInit(rIter, &mInvertStick);
    s32 use9mCollision = -1;
    MR::getJMapInfoArg2NoInit(rIter, &use9mCollision);

    if (use9mCollision == 1) {
        mUsePole9mCollision = true;
    }

    const char* name = nullptr;
    MR::getObjectName(&name, rIter);
    if (strstr(name, "NoModel") != nullptr) {
        mNoModel = true;
    }
    if (strstr(name, "Square") != nullptr) {
        mIsSquare = true;
        mDisableHandstand = true;
    } else if (strstr(name, "2Way") != nullptr) {
        m2Way = true;
        mDisableHandstand = true;
        name = "PoleSquare";
    } else if (MR::isEqualString(name, "PoleTreeA")) {
        mIsTreeA = true;
    } else if (MR::isEqualString(name, "PoleTreeB")) {
        mIsTreeB = true;
    }

    if (MR::isEqualStageName("TwisterTowerGalaxy")) {
        mIsTwisterTower = true;
    }

    TPos3f mtx;
    mtx.identity();
    MR::makeMtxTR(mtx.toMtxPtr(), this);

    mtx.getXDir(mSide);
    mtx.getYDir(mUp);
    mtx.getZDir(mFront);

    mPosMtx.set(mtx);
    mPosMtx.invert(mPosMtx);
    mPosMtx.zeroTrans();

    if (!isTree()) {
        initHitSensor(3);
        if (m2Way) {
            MR::addHitSensorBinder(this, "bind", 8, 10.0f, TVec3f(0.0f, 0.0f, 0.0f));
        } else {
            MR::addHitSensorBinder(this, "bind", 8, 60.0f, TVec3f(0.0f, 0.0f, 0.0f));
        }
        MR::addHitSensorRide(this, "ride", 8, 100.0f, TVec3f(0.0f, ::sSensorOffsetY, 0.0f));
        MR::addHitSensorPush(this, "push", 8, 30.0f, TVec3f(0.0f, 0.0f, 0.0f));
    }

    if (!mNoModel) {
        MR::initActor(this, rIter, name, false);
        if (!m2Way) {
            if (mUsePole9mCollision) {
                MR::initCollisionParts(this, "Pole9m", getSensor("ride"), nullptr);
            } else {
                MR::initCollisionParts(this, name, getSensor("ride"), nullptr);
            }
        }
    } else {
        mBaseMtx.set(mtx);
        if (MR::isEqualString(name, "PoleNoModel")) {
            ResourceHolder* resource1 = MR::createAndAddResourceHolder("Pole.arc");
            MR::initCollisionPartsFromResourceHolder(this, "Pole", getSensor("ride"), resource1, mBaseMtx.toMtxPtr());
        } else {
            ResourceHolder* resource2 = MR::createAndAddResourceHolder("PoleSquare.arc");
            MR::initCollisionPartsFromResourceHolder(this, "PoleSquare", getSensor("ride"), resource2, mBaseMtx.toMtxPtr());
        }
    }

    if (MR::isEqualString(name, "Pole") || MR::isEqualString(name, "PoleSquare")) {
        mTopMtx = MR::getJointMtx(this, "PoleTop");
        mBottomMtx = MR::getJointMtx(this, "PoleBottom");
        if (MR::isEqualString(name, "Pole")) {
            if (mDisableHandstand) {
                mTopMargin = 170.0f;
            } else {
                MR::hideMaterial(this, "PoleTopStopMat_v");
            }
        }
        updateTopPos(mHeight);
    } else {
        if (MR::isEqualString(name, "PoleTreeA")) {
            mHeight = 800.0f;
        }

        if (MR::isEqualString(name, "PoleTreeB")) {
            mHeight = 900.0f;
        }

        mTopPos.set(mUp);
        mTopPos.scale(mHeight);
        mTopPos.add(mBasePos);

        if (!mNoModel) {
            mBottomMtx = MR::getJointMtx(this, "world_root");
        }
    }

    if (!mNoModel) {
        MR::setMtxAxisXYZ(mBottomMtx, mSide, mUp, mFront);
        MR::setMtxTrans(mBottomMtx, mBasePos);
    }

    if (!isTree()) {
        if (mNoModel) {
            MR::connectToSceneMapObjMovement(this);
        } else {
            MR::connectToSceneMapObj(this);
        }
    }

    if (isTree()) {
        initSound(4, "PoleTree", nullptr, TVec3f(0.0f, 0.0f));
    } else {
        initSound(4, "Pole", nullptr, makeZeroVec());
    }

    MR::initActorCamera(this, rIter, &mCameraInfo);
    initNerve(GET_NERVE(Pole, PoleNrvFree));

    mCenterPos.set(mUp);
    mCenterPos.scale(mHeight / 2.0f);
    mCenterPos.add(mBasePos);
    MR::setClippingTypeSphere(this, mHeight / 2.0f, &mCenterPos);
    MR::useStageSwitchAwake(this, rIter);

    if (MR::tryRegisterDemoCast(this, rIter)) {
        makeActorDead();
    } else {
        makeActorAppeared();
    }
}

void Pole::appear() {
    LiveActor::appear();
    setNerve(GET_NERVE(Pole, PoleNrvDemoAppear));
}

void Pole::calcAnim() {
    if (!mNoModel) {
        MR::updateMaterial(this);
    }

    if (isTree()) {
        mModelManager->calcAnim();
    }
}

void Pole::exeDemoAppear() {
    if (MR::isFirstStep(this)) {
        MR::invalidateHitSensors(this);
        MR::invalidateCollisionParts(this);
    }

    updateTopPos(mHeight * MR::calcNerveRate(this, ::sStepDemoAppear));

    if (MR::isDemoActive()) {
        MR::startActionSound(this, "OjLvPoleAppear", -1, -1, -1);
    }

    if (MR::isStep(this, ::sStepDemoAppear)) {
        Pole::updateTopPos(mHeight);
        MR::validateCollisionParts(this);
        MR::validateHitSensors(this);
        setNerve(GET_NERVE(Pole, PoleNrvFree));
    }
}

void Pole::exeFree() {
    TVec3f top = mTopPos - mUp.scaleInline(20.0f);
    TVec3f base = mBasePos + mUp.scaleInline(20.0f);
    MR::calcPerpendicFootToLineInside(&mPosition, *MR::getPlayerCenterPos(), top, base);
}

void Pole::exeFreeInvalid() {
    if (MR::isFirstStep(this)) {
        MR::invalidateHitSensors(this);
    }

    exeFree();

    if ((MR::isOnGroundPlayer() && (!mIsTwisterTower || MR::isGreaterStep(this, 30))) || MR::calcDistanceToPlayer(this) > ::sPlayerDistanceToValid || MR::isGreaterStep(this, ::sStepInvalid)) {
        MR::validateHitSensors(this);
        setNerve(GET_NERVE(Pole, PoleNrvFree));
    }
}

void Pole::exeBindStart() {
    if (MR::isFirstStep(this)) {
        if (isSquare()) {
            MR::startBckPlayer("SquarePoleStart", nullptr);
        } else {
            MR::startBckPlayer("TreeClimbStart", nullptr);
        }
    }

    if (!isSquare()) {
        f32 rotateSpeed = ::sBindStartRotateSpeed;
        if (isNerve(GET_NERVE(Pole, PoleNrvBindStartFast))) {
            rotateSpeed = ::sBindStartRotateSpeedFast;
        }

        s16 frame = MR::getBckCtrl(mRider)->getEnd();
        mRotation.y += rotateSpeed * MR::getEaseOutValue(1.0f - ((f32)getNerveStep() / frame), 0.0f, 1.0f, 1.0f);
    }

    if (!tryJump(false, 0.0f) && MR::isActionEnd(mRider)) {
        if (isSquare()) {
            setNerve(GET_NERVE(Pole, PoleNrvBindTurnEnd));
        } else {
            setNerve(GET_NERVE(Pole, PoleNrvBindWait));
        }
    }
}

void Pole::exeBindWait() {
    if (MR::isFirstStep(this)) {
        if (isSquare()) {
            if (!MR::isBckPlaying(mRider, "SquarePoleWait")) {
                MR::startBckPlayer("SquarePoleWait", nullptr);
            }
        } else {
            MR::startBckPlayer("TreeWait", nullptr);
        }
        mClimbSpeed = 0.0f;
    }

    if (tryJump(false, 0.0f)) {
        return;
    }

    if (!isNerve(GET_NERVE(Pole, PoleNrvBindTurnEnd))) {
        if (tryTurn()) {
            return;
        }
    } else if (!isEnableTurn()) {
        setNerve(GET_NERVE(Pole, PoleNrvBindWait));
        return;
    }

    if (!tryClimbUp()) {
        if (tryFallDown()) {
            return;
        }
    }
}

// NONMATCHING: equivalent early-return branch layout differs.
void Pole::exeBindTurn() {
    if (MR::isFirstStep(this)) {
        if (mIsSquare) {
            if (isNerve(GET_NERVE(Pole, PoleNrvBindTurnLeft))) {
                MR::startBckPlayer("SquarePoleTurnL", nullptr);
            } else {
                MR::startBckPlayer("SquarePoleTurnR", nullptr);
            }
        } else if (m2Way) {
            if (isNerve(GET_NERVE(Pole, PoleNrvBindTurnLeft))) {
                MR::startBckPlayer("SquarePole180TurnL", nullptr);
            } else {
                MR::startBckPlayer("SquarePole180TurnR", nullptr);
            }
        } else {
            MR::startBckPlayer("ClimbTurn", nullptr);
        }
    }

    if (!isSquare()) {
        mRotation.y += ::sRotateSpeedRate * getPoleSubPadStickX();
        mRotation.y = MR::repeat(mRotation.y, 0.0f, 360.0f);
        tryStartTreeAnim("Shake");
    }

    f32 jump = 0.0f;

    if (isSquare() && MR::isGreaterStep(this, ::sStepSquareJumpChangeDir)) {
        if (mIsSquare) {
            if (isNerve(GET_NERVE(Pole, PoleNrvBindTurnLeft))) {
                jump = -90.0f;
            } else {
                jump = 90.0f;
            }
        } else {
            if (isNerve(GET_NERVE(Pole, PoleNrvBindTurnLeft))) {
                jump = -180.0f;
            } else {
                jump = 180.0f;
            }
        }
    }

    if (tryJump(false, jump)) {
        return;
    }

    if (isSquare()) {
        if (MR::isActionEnd(mRider)) {
            if (mIsSquare) {
                if (isNerve(GET_NERVE(Pole, PoleNrvBindTurnRight))) {
                    mRotation.y += 90.0f;
                } else {
                    mRotation.y -= 90.0f;
                }
            } else {
                if (isNerve(GET_NERVE(Pole, PoleNrvBindTurnRight))) {
                    mRotation.y += 180.0f;
                } else {
                    mRotation.y -= 180.0f;
                }
            }

            mRotation.y = MR::repeat(mRotation.y, 0.0f, 360.0f);

            MR::startBckPlayer("SquarePoleWait", "SquarePoleTurnEnd");
            setNerve(GET_NERVE(Pole, PoleNrvBindTurnEnd));
        } else {
            return;
        }
    } else if (!isEnableTurn()) {
        setNerve(GET_NERVE(Pole, PoleNrvBindWait));
    }
}

void Pole::exeBindClimbUp() {
    if (MR::isFirstStep(this)) {
        if (isSquare()) {
            MR::startBckPlayer("SquarePoleClimb", nullptr);
            MR::setBckRate(mRider, ::sSquareClimbUpAnimFrameRate);
        } else {
            MR::startBckPlayer("TreeClimb", nullptr);
            MR::setBckRate(mRider, ::sClimbUpAnimFrameRate);
        }
        mClimbSpeed = 0.0f;
    }

    if (!tryJump(false, ::sClimbUpRotateSpeedRate) && !tryTurn() && !tryFallDown()) {
        if (MR::abs(getPoleSubPadStickY()) < 0.3f) {
            setNerve(GET_NERVE(Pole, PoleNrvBindWait));
            return;
        }

        mClimbCoord += ::sClimbUpSpeedRate;
        if (mClimbCoord > mHeight - mTopMargin) {
            mClimbCoord = mHeight - mTopMargin;

            if (!mDisableHandstand) {
                updateBindTrans();
                setNerve(GET_NERVE(Pole, PoleNrvBindHandstandStart));
                return;
            }

            if (isSquare()) {
                if (!MR::isBckPlaying(mRider, "SquarePolePushWait")) {
                    MR::startBckPlayer("SquarePolePushWait", nullptr);
                }
            }
        } else {
            MR::tryRumblePadVeryWeak(this, WPAD_CHAN0);
            tryStartTreeAnim("Shake");
        }
        updateBindTrans();
    }
}

void Pole::exeBindFallDown() {
    if (MR::isFirstStep(this)) {
        if (isSquare()) {
            MR::startBckPlayer("SquarePoleFall", nullptr);
        } else {
            MR::startBckPlayer("TreeFall", nullptr);
        }
    }

    if (isSquare()) {
        mClimbSpeed -= ::sSquareFallDownAccelRate;
        mClimbSpeed = mClimbSpeed >= -::sSquareFallDownSpeedMaxY ? mClimbSpeed : -::sSquareFallDownSpeedMaxY;
    } else {
        mClimbSpeed -= ::sFallDownAccelRate;
        mClimbSpeed = mClimbSpeed >= -::sFallDownSpeedMaxY ? mClimbSpeed : -::sFallDownSpeedMaxY;
    }

    mClimbCoord += mClimbSpeed;

    if (!isSquare()) {
        mRotation.y += ::sFallDownRotateSpeedRate * mClimbSpeed;
    }

    updateBindTrans();
    MR::tryRumblePadVeryWeak(this, WPAD_CHAN0);
    tryStartTreeAnim("Shake");

    if (isTree()) {
        MR::startLevelSoundPlayer("SE_PM_LV_SLIP_CODE_SAND", -1);
    } else {
        MR::startLevelSoundPlayer("SE_PM_LV_POLE_SLIDE", -1);
    }

    if ((!isSquare() && mClimbCoord <= ::sLandingHeightMin) || (isSquare() && mClimbCoord <= ::sLandingHeightMinSquare)) {
        TPos3f mtx;
        calcGravityMtx(&mtx);
        TVec3f front(0.0f, 0.0f, 0.0f);
        front.x = JMASinDegree(mRotation.y);
        front.z = JMACosDegree(mRotation.y);
        mtx.mult(front, front);

        TVec3f pos = front * -::sPlayerPosOffsetAtLanding;
        pos.add(mBasePos);

        MR::setPlayerPos(pos);
        MR::setPlayerFrontTargetVec(front, 1);
        MR::endActorCamera(this, mCameraInfo, true, -1);
        MR::endBindAndPlayerWait(this);

        mRider = nullptr;
        setNerve(GET_NERVE(Pole, PoleNrvFreeInvalid));
        return;
    }

    if (!tryJump(false, 0.0f) && !tryTurn() && !tryClimbUp()) {
        if (MR::abs(getPoleSubPadStickY()) < 0.3f) {
            setNerve(GET_NERVE(Pole, PoleNrvBindWait));
        }
    }
}

void Pole::exeBindHandstandStart() {
    if (MR::isFirstStep(this)) {
        MR::startBckPlayer("TreeHandstandStart", nullptr);
        MR::startSoundPlayer("SE_PM_POLE_STAND", -1, -1);
        MR::startSoundPlayer("SE_PV_LIFT_UP", -1, -1);
    }

    if (MR::isLessStep(this, ::sStepJumpReverseHandstandStart)) {
        if (Pole::tryJump(false, 0.0f)) {
            return;
        }
    } else {
        if (Pole::tryJump(true, 0.0f)) {
            return;
        }
    }

    if (!Pole::tryHandstandTurn()) {
        if (MR::isActionEnd(mRider)) {
            setNerve(GET_NERVE(Pole, PoleNrvBindHandstandWait));
        }
    }
}

void Pole::exeBindHandstandWait() {
    if (MR::isFirstStep(this)) {
        MR::startBckPlayer("TreeHandstandWait", nullptr);
    }

    if (!Pole::tryJump(true, 0.0f) && !Pole::tryHandstandTurn()) {
        f32 stickY = getPoleSubPadStickY();
        f32 stickEnd = ::sEndHandstandStickY;
        if (stickY < -stickEnd) {
            setNerve(GET_NERVE(Pole, PoleNrvBindHandstandEnd));
        }
    }
}

void Pole::exeBindHandstandEnd() {
    if (MR::isFirstStep(this)) {
        MR::startBckPlayer("TreeHandstandEnd", nullptr);
    }

    if (MR::isLessStep(this, ::sStepJumpReverseHandstandEnd)) {
        if (tryJump(true, 0.0f)) {
            return;
        }
    } else if (tryJump(false, 0.0f)) {
        return;
    }

    if (!tryTurn()) {
        if (MR::isActionEnd(mRider)) {
            setNerve(GET_NERVE(Pole, PoleNrvBindWait));
        }
    }
}

void Pole::exeBindHandstandTurn() {
    if (MR::isFirstStep(this)) {
        MR::startBckPlayer("TreeHandstandTurn", nullptr);
    }

    mRotation.y += getPoleSubPadStickX() * ::sRotateSpeedRate;
    mRotation.y = MR::repeat(mRotation.y, 0.0f, 360.0f);

    if (!tryJump(true, 0.0f) && !isEnableTurn()) {
        setNerve(GET_NERVE(Pole, PoleNrvBindHandstandWait));
    }
}

void Pole::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (mRider != nullptr && MR::isSensorRide(pSender)) {
        MR::tryGetItem(pSender, pReceiver);
        return;
    }

    if (!MR::isSensorPush(pSender) || isNerve(GET_NERVE(Pole, PoleNrvFreeInvalid))) {
        return;
    }

    if (m2Way) {
        return;
    }

    if (MR::isOnGroundPlayer()) {
        MR::sendMsgPush(pReceiver, pSender);
    }
}

bool Pole::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (!isTree()) {
        return false;
    }

    s32 interruptStep = 30;
    if (MR::isAnyAnimPlaying(this, "Shake") && MR::getBckFrame(this) < interruptStep) {
        return false;
    }

    if (MR::isMsgPlayerSpinAttackOrSupportTico(msg)) {
        if (!MR::isEqualSensor(pReceiver, this, "HitReaction") && !MR::isEqualSensor(pReceiver, this, "ride")) {
            return false;
        }

        return tryStartTreeAnim("Shake");
    }

    if (MR::isMsgStarPieceAttack(msg)) {
        if (!MR::isEqualSensor(pReceiver, this, "HitReaction")) {
            return false;
        }

        tryStartTreeAnim("Shake");
        return true;
    }

    return false;
}

bool Pole::receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (mRider != nullptr && MR::isEqualSensor(pReceiver, this, "bind")) {
        MR::endActorCamera(this, mCameraInfo, true, -1);
        MR::endBindAndPlayerDamageMsg(this, msg);

        mRider = nullptr;
        setNerve(GET_NERVE(Pole, PoleNrvFreeInvalid));
        return true;
    }
    return false;
}

bool Pole::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgIsRushTakeOver(msg)) {
        return true;
    }

    if (MR::isMsgAutoRushBegin(msg)) {
        if (MR::isOnGroundPlayer() || isNerve(GET_NERVE(Pole, PoleNrvFreeInvalid))) {
            return false;
        }

        TVec3f velXZ = *MR::getPlayerVelocity();
        MR::vecKillElement(velXZ, mUp, &velXZ);
        f32 speedXZ = velXZ.length();
        f32 speedY = mUp.dot(*MR::getPlayerVelocity());
        if (!MR::isPlayerElementModeBee() && MR::abs(speedXZ) < ::sInvalidPlayerSpeedMaxXZ && speedY > ::sInvalidPlayerSpeedMinY) {
            return false;
        }

        TVec3f front = mPosition;
        front.sub(*MR::getPlayerPos());
        MR::vecKillElement(front, mUp, &front);
        if (MR::isNearZero(front)) {
            front.set(mFront);
        } else {
            MR::normalize(&front);
        }

        mPosMtx.mult(front, front);
        front.y = 0.0f;

        if (!MR::isNearZero(front)) {
            MR::normalize(&front);
        } else {
            front.set(mFront);
        }
        mRider = MR::getSensorHost(pSender);
        mRotation.y = MR::calcRotateY(front.x, front.z);

        if (mIsSquare) {
            s32 side = (45.0f + mRotation.y) / 90.0f;
            mRotation.y = 90.0f * side;
        }

        if (m2Way) {
            s32 side = (90.0f + mRotation.y) / 180.0f;
            mRotation.y = 180.0f * side - 90.0f;
        }

        TVec3f climbProj = mPosition;
        climbProj.sub(mBasePos);
        mClimbCoord = climbProj.dot(mUp);

        if (isSquare()) {
            mClimbCoord = MR::clamp(mClimbCoord, 50.0f, mHeight - mTopMargin);
        } else {
            mClimbCoord = MR::clamp(mClimbCoord, 120.0f, mHeight - mTopMargin);
        }

        mClimbSpeed = 0.0f;
        updateBindTrans();

        MR::invalidateClipping(this);
        MR::startSoundPlayer("SE_PM_GRAB_OBJ", -1, -1);
        MR::startSoundPlayer("SE_PV_LIFT_UP", -1, -1);
        MR::startActorCameraNoTarget(this, mCameraInfo, -1);
        MR::tryRumblePadWeak(this, WPAD_CHAN0);

        if (speedXZ > ::sPlayerSpeedToCatchMax) {
            setNerve(GET_NERVE(Pole, PoleNrvBindStartFast));
        } else if (speedXZ > ::sPlayerSpeedToCatchMin) {
            setNerve(GET_NERVE(Pole, PoleNrvBindStart));
        } else {
            setNerve(GET_NERVE(Pole, PoleNrvBindWait));
        }

        tryStartTreeAnim("Land");

        if (isTree()) {
            MR::enablePlayerSilhouetteDuringRush();
        }

        return true;
    }

    if (MR::isMsgUpdateBaseMtx(msg)) {
        TPos3f posMtx;
        TRot3f rotMtx;
        posMtx.identity();
        posMtx.setXYZDir(mSide, mUp, mFront);

        rotMtx.identity();
        MR::makeMtxTransRotateY(rotMtx, this);
        MR::multMtx(posMtx, rotMtx, posMtx);
        posMtx.setTrans(mPosition);
        MR::setBaseTRMtx(mRider, posMtx);
        return true;
    }

    if (MR::isMsgRushCancel(msg)) {
        if (!MR::isDemoActive()) {
            MR::startSoundPlayer("SE_PM_JUMP_M", -1, -1);
            MR::startLevelSoundPlayer("JumpObj", -1, -1);
        }
        mRider = nullptr;
        setNerve(GET_NERVE(Pole, PoleNrvFree));
        return true;
    }

    return false;
}

bool Pole::tryJump(bool handstand, f32 rotateSpeed) {
    if (MR::testCorePadTriggerA(WPAD_CHAN0) || MR::testSystemTriggerA()) {
        TPos3f pos;
        calcGravityMtx(&pos);

        f32 frontAngle = MR::repeat(rotateSpeed + mRotation.y + 180.0f, 0.0f, 360.0f);
        TVec3f jumpFront(0.0f, 0.0f, 0.0f);
        jumpFront.x = JMASinDegree(frontAngle);
        jumpFront.z = JMACosDegree(frontAngle);

        if (handstand) {
            jumpFront.scale(-1.0f);
        }

        TVec3f jumpVec;
        if (m2Way && MR::isPlayerElementModeBee()) {
            jumpVec.x = jumpFront.x;
            jumpVec.y = ::sJumpSpeedXZ;
            jumpVec.z = ::sJumpSpeedXZ * jumpFront.z;
        } else {
            jumpVec.x = ::sJumpSpeedXZ * jumpFront.x;
            jumpVec.y = ::sJumpSpeedY;
            jumpVec.z = ::sJumpSpeedXZ * jumpFront.z;
        }
        pos.mult(jumpVec, jumpVec);
        pos.mult(jumpFront, jumpFront);
        MR::setPlayerFrontTargetVec(jumpFront, 1);

        if (handstand) {
            MR::startBckPlayer("TreeHandStandJump", nullptr);
        } else {
            MR::startBckPlayer("WallJump", nullptr);
        }

        MR::startSoundPlayer("SE_PM_JUMP_M", -1, -1);
        MR::startLevelSoundPlayer("JumpObj", -1, -1);
        MR::endActorCamera(this, mCameraInfo, true, -1);

        if (isSquare()) {
            TVec3f jumpPos = jumpFront * ::sSquareJumpPosOffset;
            jumpPos.add(*MR::getPlayerPos());
            MR::setPlayerPos(jumpPos);
        }

        MR::endBindAndPlayerJump(this, jumpVec, 0);

        if (handstand) {
            MR::becomeContinuousBckPlayer();
        }

        mRider = nullptr;
        tryStartTreeAnim("Jump");
        setNerve(GET_NERVE(Pole, PoleNrvFreeInvalid));
        return true;
    }
    return false;
}

bool Pole::tryTurn() {
    if (isEnableTurn()) {
        if (getPoleSubPadStickX() > 0.0f) {
            setNerve(GET_NERVE(Pole, PoleNrvBindTurnRight));
        } else {
            setNerve(GET_NERVE(Pole, PoleNrvBindTurnLeft));
        }
        return true;
    }
    return false;
}

bool Pole::tryClimbUp() {
    if (getPoleSubPadStickY() > 0.3f) {
        setNerve(GET_NERVE(Pole, PoleNrvBindClimbUp));
        return true;
    }
    return false;
}

bool Pole::tryFallDown() {
    f32 stickY = getPoleSubPadStickY();
    f32 stickMin = ::sFallDownStickMin;
    if (stickY < -stickMin) {
        setNerve(GET_NERVE(Pole, PoleNrvBindFallDown));
        return true;
    }
    return false;
}

bool Pole::tryHandstandTurn() {
    if (isEnableTurn()) {
        setNerve(GET_NERVE(Pole, PoleNrvBindHandstandTurn));
        return true;
    }
    return false;
}

bool Pole::isEnableTurn() const {
    if (MR::abs(getPoleSubPadStickX()) > ::sTurnStickMin) {
        if (m2Way) {
            if (getPoleSubPadStickX() < 0.0f && mRotation.y < 180.0f) {
                return false;
            }

            if (getPoleSubPadStickX() > 0.0f && mRotation.y > 180.0f) {
                return false;
            }
        }

        return true;
    }
    return false;
}

void Pole::updateBindTrans() {
    mPosition.set(mUp);
    mPosition.scale(mClimbCoord);
    mPosition.add(mBasePos);
}

void Pole::calcGravityMtx(TPos3f* pos) {
    pos->identity();
    MR::calcGravity(this);
    TVec3f up(-mGravity.x, -mGravity.y, -mGravity.z);
    MR::makeMtxUpFront(pos, up, mFront);
}

void Pole::updateTopPos(f32 top) {
    f32 topCoord = top;

    if (isSquare()) {
        topCoord += ::sSquareHeightOffset;
    }

    mTopPos.set(mUp);
    mTopPos.scale(topCoord);
    mTopPos.add(mBasePos);
    MR::setMtxAxisXYZ(mTopMtx, mSide, mUp, mFront);
    MR::setMtxTrans(mTopMtx, mTopPos);
}

f32 Pole::getPoleSubPadStickX() const {
    if (mInvertStick) {
        return -MR::getSubPadStickX(WPAD_CHAN0);
    } else {
        return MR::getSubPadStickX(WPAD_CHAN0);
    }
}

f32 Pole::getPoleSubPadStickY() const {
    if (mInvertStick) {
        return -MR::getSubPadStickY(WPAD_CHAN0);
    } else {
        return MR::getSubPadStickY(WPAD_CHAN0);
    }
}

bool Pole::tryStartTreeAnim(const char* pName) {
    if (!isTree()) {
        return false;
    }

    if (MR::isEqualString(pName, "Shake")) {
        bool isClimbing = isNerve(GET_NERVE(Pole, PoleNrvBindClimbUp)) || isNerve(GET_NERVE(Pole, PoleNrvBindFallDown)) ||
                          isNerve(GET_NERVE(Pole, PoleNrvBindTurnRight)) || isNerve(GET_NERVE(Pole, PoleNrvBindTurnLeft));

        if (isClimbing) {
            if (mIsTreeA && mClimbCoord < 300.0f) {
                return false;
            }

            if (mIsTreeB && mClimbCoord < 400.0f) {
                return false;
            }

            bool isPlaying = MR::isAnyAnimPlaying(this, "Shake") || MR::isAnyAnimPlaying(this, "Land");

            s32 interruptStep = 30;
            if (isPlaying && MR::getBckFrame(this) < interruptStep) {
                return false;
            }
        }
    }

    return MR::tryStartAllAnim(this, pName);
}
