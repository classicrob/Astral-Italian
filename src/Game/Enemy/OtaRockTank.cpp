#define JGEOMETRY_VEC3_MI_NOINLINE
#pragma auto_inline off
#include "Game/Enemy/OtaRockTank.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/BreakModel.hpp"
#include "Game/LiveActor/ActorItemGenerator.hpp"
#include "Game/Enemy/CocoNutBall.hpp"
#include "Game/Enemy/FireBall.hpp"
#include "Game/Enemy/OtaRockChief.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util/FixedPosition.hpp"
#include <cstdio>

#include "Game/Enemy/ScatterFireBall.hpp"
namespace NrvOtaRockTank {
    NEW_NERVE(OtaRockTankNrvRunAccel, OtaRockTank, RunAccel);
    NEW_NERVE(OtaRockTankNrvRunKeep, OtaRockTank, RunKeep);
    NEW_NERVE(OtaRockTankNrvBrakeA, OtaRockTank, BrakeA);
    NEW_NERVE(OtaRockTankNrvRunRail, OtaRockTank, RunRail);

    // Empty nerve (no executor).
    class OtaRockTankNrvNone : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {}
        static OtaRockTankNrvNone sInstance;
    };
    OtaRockTankNrvNone OtaRockTankNrvNone::sInstance ATTRIBUTE_WEAK;

    NEW_NERVE(OtaRockTankNrvStartDemo, OtaRockTank, StartDemo);
    NEW_NERVE(OtaRockTankNrvRunStart, OtaRockTank, RunStart);
    NEW_NERVE(OtaRockTankNrvRun, OtaRockTank, Run);
    NEW_NERVE(OtaRockTankNrvFireAttack, OtaRockTank, FireAttack);
    NEW_NERVE(OtaRockTankNrvBrakeB, OtaRockTank, BrakeB);
    NEW_NERVE(OtaRockTankNrvTurnAttack, OtaRockTank, TurnAttack);
    NEW_NERVE(OtaRockTankNrvRunFukashi, OtaRockTank, RunFukashi);
    NEW_NERVE(OtaRockTankNrvMeteor, OtaRockTank, Meteor);
    NEW_NERVE(OtaRockTankNrvFallDash, OtaRockTank, Dash);
    NEW_NERVE(OtaRockTankNrvFallDashStart, OtaRockTank, Dash);
    NEW_NERVE(OtaRockTankNrvDashStart, OtaRockTank, DashStart);
    NEW_NERVE(OtaRockTankNrvFallDashBrake, OtaRockTank, DashBrake);
    NEW_NERVE(OtaRockTankNrvBackDashStart, OtaRockTank, Dash);
    NEW_NERVE(OtaRockTankNrvBackDash, OtaRockTank, Dash);
    NEW_NERVE(OtaRockTankNrvBackDashBrake, OtaRockTank, DashBrake);
    NEW_NERVE(OtaRockTankNrvRunAfterDash, OtaRockTank, Run);
    NEW_NERVE(OtaRockTankNrvDamageA, OtaRockTank, Damage);
    NEW_NERVE(OtaRockTankNrvDamageB, OtaRockTank, Damage);
    NEW_NERVE(OtaRockTankNrvDownDemo, OtaRockTank, DownDemo);
    NEW_NERVE(OtaRockTankNrvExplosion, OtaRockTank, Explosion);
};  // namespace NrvOtaRockTank

#define NT(n) (&NrvOtaRockTank::OtaRockTankNrv##n::sInstance)

namespace {
    extern const s32 sAngleDivisor;
    const f32 cWheelBrake = 0.15f;
    const char* sOtaRockPosName = "OtaRockPos";
    const char* sWheelLName = "WheelL";
    const char* sWheelRName = "WheelR";
    const char* sBattery01Name = "Battery01";
    const char* sBattery02Name = "Battery02";
    const char* sBattery03Name = "Battery03";
    const char* sBattery04Name = "Battery04";
    const char* sBattery05Name = "Battery05";
    const char* sBattery06Name = "Battery06";
    const char* sMouthName = "Mouth";
    const char* sThrowEffects[] = {
        "OtaRockTankThrowLeftFront", "OtaRockTankThrowCenterFront", "OtaRockTankThrowRightFront",
        "OtaRockTankThrowLeftBack", "OtaRockTankThrowCenterBack", "OtaRockTankThrowRightBack", "Throw"
    };
    extern const f32 sLowLifeSpeed;
    extern const f32 sLowLifeStopRadius;
    TVec3f sBatteryOffset(0.0f, -20.0f, 500.0f);
    TVec3f sMouthOffset(180.0f, 0.0f, 0.0f);
    TVec3f sBatteryRotate(0.0f, 180.0f, 0.0f);
    TVec3f sCameraOffset(0.0f, 500.0f, 0.0f);
};

OtaRockTank::OtaRockTank(const char* pName) : LiveActor(pName) {
    mBreakModel = nullptr;
    mActorCameraInfo = nullptr;
    mStartDemoCameraInfo = nullptr;
    mCameraRegisterPosition.set(0.0f, 0.0f, 0.0f);
    mHasPowerStar = false;
    mLife = 3;
    mChief = nullptr;
    mLeftWheelJointController = nullptr;
    mRightWheelJointController = nullptr;
    mWheelRot = 0.0f;
    mShotIndex = 1;
    mMeteorCount = 0;
    mMeteorRandom = 0;
    mCocoNutBalls = nullptr;
    mFireBalls = nullptr;
    mFixedPositions = nullptr;
    mRailClipPos.set(0.0f, 0.0f, 0.0f);
    mRailDirection.set(0.0f, 0.0f, 0.0f);
    mPrevRailDirection.set(0.0f, 0.0f, 0.0f);
    mWheelSpeed = 10.0f;
    mDamageMask = 0xFF;
    mCurrentRailPointNo = 0;
    mTargetRailPointNo = 0;
    mSpeed = 16.0f;
    mCanFireAtRailPoint = 0;
    mVolleyStarted = 0;
    mUnusedFlag = 0;
    mFireTimer = 0;
    mThrowNum = 2;
    mMeteorStep = 0;
    mSpeedStart = 0.0f;
}

JointController* OtaRockTank::createTankJointController(const char* pJointName, bool (OtaRockTank::*calcFunc)(TPos3f*, const JointControllerInfo&)) {
    JointControlDelegator<OtaRockTank>* pController = new JointControlDelegator<OtaRockTank>(this, calcFunc, nullptr);
    MR::setJointControllerParam(pController, this, pJointName);
    return pController;
}

#pragma dont_inline on
// NONMATCHING: array assignment evaluation order and saved-register allocation differ
void OtaRockTank::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, false);
    mChief = new OtaRockChief(this, "\x83\x49\x83\x5E\x83\x8D\x83\x62\x83\x4E\x83\x60\x81\x5B\x83\x74");
    mChief->initWithoutIter();
    MR::startBrk(mChief, "Normal");
    mBreakModel = new BreakModel(this, "タンク壊れモデル", "OtaRockTankBreak", getBaseMtx(), "Break");
    mBreakModel->initWithoutIter();
    mStartDemoCameraInfo = MR::createActorCameraInfo(rIter);
    MR::initAnimCamera(this, mStartDemoCameraInfo, "StartDemo");
    mDownDemoCameraInfo = MR::createActorCameraInfo(rIter);
    MR::initAnimCamera(this, mDownDemoCameraInfo, "DownDemo");
    MR::initActorCamera(this, rIter, &mActorCameraInfo);
    MR::declareCameraRegisterVec(this, 0, &mCameraRegisterPosition);
    mCocoNutBalls = new CocoNutBall[8];
    for (s32 i = 0; i < 8; i++) {
        TVec3f& scale = mCocoNutBalls[i].mScale;
        scale = TVec3f(1.1f, 1.1f, 1.1f);
        mCocoNutBalls[i].initWithoutIter();
        mCocoNutBalls[i].mHost = this;
        mCocoNutBalls[i].mIsTankBall = true;
    }
    mFireBalls = new FireBall[48];
    for (s32 i = 0; i < 48; i++) {
        TVec3f& scale = mFireBalls[i].mScale;
        scale = TVec3f(1.5f, 1.5f, 1.5f);
        mFireBalls[i].mUnusedB4 = 2.0f;
        mFireBalls[i].initWithoutIter();
        mFireBalls[i].mHost = this;
    }
    mScatterBalls = new ScatterFireBall[24];
    for (s32 i = 0; i < 24; i++) {
        TVec3f& scale = mScatterBalls[i].mScale;
        scale = TVec3f(1.5f, 1.5f, 1.5f);
        mScatterBalls[i]._B4 = 2.0f;
        mScatterBalls[i].initWithoutIter();
        mScatterBalls[i].mHost = this;
    }
    mTargetPoses = new TVec3f[11];
    for (s32 i = 0; i < 11; i++) {
        TVec3f& pose = mTargetPoses[i];
        pose.x = 0.0f;
        pose.y = 0.0f;
        pose.z = 0.0f;
        TVec3f def(0.0f);
        char name[128];
        snprintf(name, 128, "着弾点%d", i);
        MR::tryFindNamePos(name, &mTargetPoses[i], &def);
    }
    TVec3f rot(0.0f);
    TVec3f offs = -sBatteryOffset;
    offs.y = -offs.y;
    mFixedPositions = new FixedPosition*[7];
    mFixedPositions[0] = new FixedPosition(this, sBattery03Name, offs, sBatteryRotate);
    mFixedPositions[1] = new FixedPosition(this, sBattery01Name, sBatteryOffset, rot);
    mFixedPositions[2] = new FixedPosition(this, sBattery02Name, sBatteryOffset, rot);
    mFixedPositions[3] = new FixedPosition(this, sBattery05Name, sBatteryOffset, rot);
    mFixedPositions[4] = new FixedPosition(this, sBattery04Name, offs, sBatteryRotate);
    mFixedPositions[5] = new FixedPosition(this, sBattery06Name, offs, sBatteryRotate);
    mFixedPositions[6] = new FixedPosition(mChief, sMouthName, sMouthOffset, TVec3f(0.0f, 90.0f, 0.0f));
    mLeftWheelJointController = createTankJointController(sWheelLName, &OtaRockTank::calcTankLeftWheelJoint);
    mRightWheelJointController = createTankJointController(sWheelRName, &OtaRockTank::calcTankRightWheelJoint);
    mChiefJointController = createTankJointController(sOtaRockPosName, &OtaRockTank::calcTankChiefJoint);
    initNerve(NT(RunAccel), 0);
    MR::initAndSetRailClipping(&mRailClipPos, this, 100.0f, 500.0f);
    MR::moveCoordToStartPos(this);
    MR::moveTransXZToCurrentRailPos(this);
    MR::calcRailDirection(&mRailDirection, this);
    mPrevRailDirection.set(mRailDirection);
    mCurrentRailPointNo = MR::getCurrentRailPointNo(this);
    s32 arg0 = -1;
    MR::getJMapInfoArg0NoInit(rIter, &arg0);
    if (arg0 != -1) {
        mHasPowerStar = true;
        MR::declarePowerStar(this);
    }
    startTankRunAnimations();
    makeActorAppeared();
}

#pragma dont_inline reset
void OtaRockTank::appear() {
    LiveActor::appear();
}

// NONMATCHING: loop register allocation differs
void OtaRockTank::control() {
    TVec3f trans[6];
    TVec3f dir[6];
    for (s32 i = 0; i < 6; i++) {
        FixedPosition* pFixedPos = mFixedPositions[i];
        pFixedPos->calc();
        pFixedPos->mMtx.getTrans(trans[i]);
        pFixedPos->mMtx.getZDir(dir[i]);
    }
    if (MR::isEffectValid(mChief, "AngrySmoke")) {
        MR::startActionSound(mChief, "Angry", -1, -1, -1);
    }
}

void OtaRockTank::kill() {
    LiveActor::kill();
    MR::endActorCamera(this, mActorCameraInfo, false, -1);
    if (mHasPowerStar) {
        MR::requestAppearPowerStar(this, mPosition);
    }
    if (MR::isValidSwitchDead(this)) {
        MR::onSwitchDead(this);
    }
}

void OtaRockTank::calcAndSetBaseMtx() {
    MR::calcAndSetBaseTRMtxFromGravityAndZAxis(this, mRailDirection);
    MR::extractMtxTrans(getBaseMtx(), &mPosition);
    TVec3f sp14(0.0f);
    PSMTXMultVecSR(getBaseMtx(), &sCameraOffset, &sp14);
    TVec3f sp8(mPosition);
    sp8 += sp14;
    mCameraRegisterPosition = sp8;
    mLeftWheelJointController->registerCallBack();
    mRightWheelJointController->registerCallBack();
    mChiefJointController->registerCallBack();
}

void OtaRockTank::exeRunAccel() {
    if (MR::isFirstStep(this)) {
        syncTankRailCoordinate();
    }
    if (MR::isLessEqualStep(this, 5)) {
        moveTankAlongRail();
        accelerateTankMove();
        accelerateTankWheels();
    } else {
        tryReachTankRailPoint();
    }
    mWheelRot += mWheelSpeed;
    if (MR::isGreaterStep(this, 10)) {
        mSpeed = getTankMaxSpeed();
        setNerve(NT(RunKeep));
    }
    tryStartTankDemo();
}

void OtaRockTank::exeRunKeep() {
    if (MR::isFirstStep(this)) {
        syncTankRailCoordinate();
    }
    if (tryReachTankRailPoint()) {
        MR::tryStartActionNotPlaying(mChief, "Wait");
        setNerve(NT(BrakeA));
    }
    mWheelRot += mWheelSpeed;
    tryStartTankDemo();
}

// NONMATCHING: animation-name address caching and saved-register allocation differ
void OtaRockTank::exeBrakeA() {
    if (MR::isFirstStep(this)) {
        mSpeedStart = mSpeed;
        MR::emitEffect(this, "BrakeL");
        MR::emitEffect(this, "BrakeR");
        const char* action = "Brake";
        MR::tryStartAllAnim(this, action);
        MR::tryStartAllAnim(mChief, action);
    }
    updateTankBrake(mSpeedStart);
    moveTankAlongRail();
    decelerateTankWheels();
    mWheelRot += mWheelSpeed;
    if (MR::isGreaterEqualStep(this, 15)) {
        startTankRunAnimations();
        setNerve(NT(RunRail));
        MR::tryDeleteEffect(this, "BrakeL");
        MR::tryDeleteEffect(this, "BrakeR");
    }
    tryStartTankDemo();
}

void OtaRockTank::exeRunRail() {
    if (!tryStartTankDemo()) {
        if (MR::isFirstStep(this)) {
            mSpeedStart = mSpeed;
        }
        TVec3f dir(mRailDirection);
        MR::calcRailDirectionAtNextPoint(&dir, this);
        moveTankTowardRailDirection(dir, 0.8f);
        mWheelRot += mWheelSpeed;
        if (MR::isGreaterEqualStep(this, 65)) {
            if (mCurrentRailPointNo != MR::getCurrentRailPointNo(this)) {
                mCurrentRailPointNo = MR::getCurrentRailPointNo(this);
                setNerve(NT(RunAccel));
            }
        }
    }
}

void OtaRockTank::exeStartDemo() {
    if (MR::isFirstStep(this)) {
        TPos3f mtx;
        mtx.identity();
        MR::tryFindNamePos("\x83\x7B\x83\x58\x90\xED\x83\x66\x83\x82\x8A\x4A\x8E\x6E\x88\xCA\x92\x75", mtx.toMtxPtr());
        mtx.getTrans(mPosition);
        mtx.getZDir(mRailDirection);
        mPrevRailDirection.set(mRailDirection);
        MR::forceDeleteEffectAll(this);
        MR::forceDeleteEffectAll(mChief);
        MR::startAction(this, "StartDemo");
        MR::startAction(mChief, "StartDemo");
        MR::hidePlayer();
        MR::stopStageBgmForDemo();
    }
    if (MR::tryEndDemoAndAnimCamera(this, mStartDemoCameraInfo, "StartDemo", 0)) {
        MR::moveCoordToStartPos(this);
        MR::moveTransXZToCurrentRailPos(this);
        MR::calcRailDirection(&mRailDirection, this);
        mPrevRailDirection.set(mRailDirection);
        mCurrentRailPointNo = MR::getCurrentRailPointNo(this);
        MR::showPlayer();
        MR::startActorCameraTargetPlayer(this, mActorCameraInfo, -1);
        setNerve(NT(RunStart));
    }
}

void OtaRockTank::exeRunStart() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "RunStart");
        mSpeed = getTankMaxSpeed();
        if (mLife == 1) {
            MR::startAction(mChief, "AngryRunStart");
            MR::startBrk(mChief, "Angry");
            if (!MR::isEffectValid(mChief, "AngrySmoke")) {
                MR::emitEffect(mChief, "AngrySmoke");
            }
        } else {
            MR::startAction(mChief, "RunStart");
        }
        mWheelSpeed = 0.0f;
        mCanFireAtRailPoint = 0;
    }
    if (MR::isGreaterStep(this, 60)) {
        accelerateTankWheels();
        mWheelRot += mWheelSpeed;
    }
    if (MR::isActionEnd(this)) {
        startTankRunAnimations();
        if (MR::isRailReachedNearNextPoint(this, getTankRailBrakeDistance())) {
            mCurrentRailPointNo = MR::getNextRailPointNo(this);
            MR::moveRailToNextPoint(this);
        }
        setNerve(NT(FireAttack));
    }
}

void OtaRockTank::exeRun() {
    if (MR::isFirstStep(this)) {
        mCanFireAtRailPoint = 0;
        mFireTimer = 30;
        syncTankRailCoordinate();
    }
    MR::startActionSound(this, "Run", -1, -1, -1);
    if (MR::isLessEqualStep(this, 5)) {
        moveTankAlongRail();
        accelerateTankMove();
        accelerateTankWheels();
    } else {
        tryReachTankRailPoint();
    }
    mWheelRot += mWheelSpeed;
    bool isRunStart = isNerve(NT(Run));
    if (MR::isStep(this, 10) && isRunStart) {
        tryFireTankVolley();
    }
    if (MR::isGreaterStep(this, 10)) {
        mCanFireAtRailPoint = canTankFireAtRailPoint();
        mSpeed = getTankMaxSpeed();
        setNerve(NT(FireAttack));
    }
}

void OtaRockTank::exeFireAttack() {
    if (MR::isFirstStep(this)) {
        mVolleyStarted = 0;
        mFireTimer = 1000;
    }
    MR::startActionSound(this, "Run", -1, -1, -1);
    if (MR::isBckOneTimeAndStopped(this)) {
        startTankRunAnimations();
        mFireTimer = getNerveStep() + 30;
        mVolleyStarted = 0;
    }
    if (tryReachTankRailPoint() && !mVolleyStarted) {
        if (mLife == 1) {
            MR::tryStartActionNotPlaying(mChief, "AngryWait");
        } else {
            MR::tryStartActionNotPlaying(mChief, "Wait");
        }
        setNerve(NT(BrakeB));
        return;
    }
    mWheelRot += mWheelSpeed;
    if (mCanFireAtRailPoint && (canTankFireAtRailPoint() || mVolleyStarted)) {
        if (MR::isStep(this, mFireTimer)) {
            MR::startBrk(this, "FireSign");
            mVolleyStarted = 1;
        }
        if (MR::isStep(this, mFireTimer + 10)) {
            MR::startAction(this, "Attack");
            MR::startAction(mChief, "Attack");
        }
        if (MR::isStep(this, mFireTimer + 50)) {
            tryFireTankVolley();
            mVolleyStarted = 0;
        }
    }
}

// NONMATCHING: animation-name address caching differs
void OtaRockTank::exeBrakeB() {
    if (MR::isFirstStep(this)) {
        mSpeedStart = mSpeed;
        MR::emitEffect(this, "BrakeL");
        MR::emitEffect(this, "BrakeR");
        const char* action = "Brake";
        MR::tryStartAllAnim(this, action);
        MR::tryStartAllAnim(mChief, action);
    }
    MR::startActionSound(this, "Brake", -1, -1, -1);
    updateTankBrake(mSpeedStart);
    moveTankAlongRail();
    decelerateTankWheels();
    mWheelRot += mWheelSpeed;
    if (MR::isGreaterEqualStep(this, 40)) {
        const Nerve* pNext = NT(TurnAttack);
        if (mLife <= 2 && MR::getNextRailPointNo(this) == 4) {
            pNext = NT(RunFukashi);
        }
        startTankRunAnimations();
        setNerve(pNext);
        MR::tryDeleteEffect(this, "BrakeL");
        MR::tryDeleteEffect(this, "BrakeR");
    }
}

void OtaRockTank::exeTurnAttack() {
    if (MR::isFirstStep(this)) {
        mSpeedStart = mSpeed;
        MR::startActionSound(this, "Turn", -1, -1, -1);
    }
    TVec3f dir(mRailDirection);
    MR::calcRailDirectionAtNextPoint(&dir, this);
    moveTankTowardRailDirection(dir, 0.8f);
    mWheelRot += mWheelSpeed;
    if (MR::isStep(this, 25)) {
        MR::startBrk(this, "FireSign");
        mVolleyStarted = 1;
    }
    if (MR::isStep(this, 35)) {
        MR::startAction(this, "Attack");
        MR::startAction(mChief, "Attack");
        MR::startActionSound(this, "RunFukashi", -1, -1, -1);
    }
    if (MR::isGreaterEqualStep(this, 65)) {
        if (mCurrentRailPointNo != MR::getCurrentRailPointNo(this)) {
            mCurrentRailPointNo = MR::getCurrentRailPointNo(this);
            setNerve(NT(Run));
        }
    }
}

void OtaRockTank::exeRunFukashi() {
    if (MR::isFirstStep(this)) {
        mSpeedStart = mSpeed;
    }
    MR::startActionSound(this, "Brake", -1, -1, -1);
    TVec3f dir(mRailDirection);
    TVec3f from(0.0f);
    TVec3f to(0.0f);
    MR::calcRailPointPos(&from, this, 4);
    MR::calcRailPointPos(&to, this, 1);
    MR::calcDirectionVec(&dir, from, to);
    moveTankTowardRailDirection(dir, 0.8f);
    mWheelRot += mWheelSpeed;
    if (MR::isGreaterEqualStep(this, 25)) {
        if (mCurrentRailPointNo != MR::getCurrentRailPointNo(this)) {
            mCurrentRailPointNo = MR::getCurrentRailPointNo(this);
            setNerve(NT(Meteor));
        }
    }
}

// NONMATCHING: animation-name address caching and saved-register allocation differ
void OtaRockTank::exeMeteor() {
    if (MR::isFirstStep(this)) {
        mWheelSpeed = 0.0f;
        mSpeed = 0.0f;
        mMeteorCount = 0;
        mMeteorRandom = MR::getRandom(static_cast< s32 >(0), static_cast< s32 >(10));
        const char* action4 = "MeteorSign";
        MR::tryStartAllAnim(this, action4);
        MR::tryStartAllAnim(mChief, action4);
    }
    MR::startActionSound(this, "FallAttack", -1, -1, -1);
    if (!MR::isAnyAnimPlaying(this, "MeteorSign")) {
        if (mMeteorCount > 10) {
            if (!MR::isActionEnd(this)) {
                return;
            }
            setNerve(NT(FallDashStart));
        } else {
            if (MR::isActionEnd(this)) {
                if (mMeteorCount < 10) {
                    const char* action5 = "MeteorAttack";
                    MR::tryStartAllAnim(this, action5);
                    MR::tryStartAllAnim(mChief, action5);
                } else {
                    const char* action6 = "MeteorEnd";
                    MR::tryStartAllAnim(this, action6);
                    MR::tryStartAllAnim(mChief, action6);
                }
                mMeteorStep = 0;
                mMeteorCount++;
            } else if (MR::isAnyAnimPlaying(this, "MeteorAttack") && mMeteorStep == 5) {
                fireTankMeteorForward(mMeteorCount - 1, mMeteorRandom);
                fireTankMeteorAtTarget(mMeteorCount - 1, mMeteorRandom);
            }
        }
        mMeteorStep++;
    }
}

// NONMATCHING: animation-name address caching and vector subtraction inlining differ
void OtaRockTank::exeDashStart() {
    if (MR::isFirstStep(this)) {
        const char* action7 = "DashStart";
        MR::tryStartAllAnim(this, action7);
        MR::tryStartAllAnim(mChief, action7);
        MR::startActionSound(this, "FallDashStart", -1, -1, -1);
        if (mLife == 1) {
            MR::startAction(mChief, "AngryRunStart");
            MR::startBrk(mChief, "Angry");
            if (!MR::isEffectValid(mChief, "AngrySmoke")) {
                MR::emitEffect(mChief, "AngrySmoke");
            }
        }
    }
    accelerateTankWheels();
    s32 target = mTargetRailPointNo;
    mWheelRot += mWheelSpeed;
    if (target == 4) {
        TVec3f dir(mRailDirection);
        TVec3f from(0.0f);
        TVec3f to(0.0f);
        MR::calcRailPointPos(&from, this, 4);
        MR::calcRailPointPos(&to, this, 1);
        MR::calcDirectionVec(&dir, to, from);
        MR::turnDirection(this, &mRailDirection, dir, 0.9989f);
    } else if (target != 1) {
        TVec3f dir(mRailDirection);
        MR::calcRailDirectionAtCoord(&dir, this, MR::getRailPointCoord(this, 4));
        MR::turnDirection(this, &mRailDirection, dir, 0.9989f);
        TVec3f p(0.0f);
        MR::calcRailPointPos(&p, this, 4);
        TVec3f d = p - mPosition;
        f32 rate = MR::calcNerveRate(this, 1200);
        mPosition.x += d.x * rate;
        mPosition.z += d.z * rate;
    }
    if (MR::isActionEnd(this)) {
        if (mTargetRailPointNo == 4) {
            setNerve(NT(BackDash));
            return;
        }
        if (mTargetRailPointNo == 1) {
            setNerve(NT(FallDash));
            return;
        }
        startTankRunAnimations();
        mCurrentRailPointNo = 4;
        MR::moveCoordToRailPoint(this, 4);
        setNerve(NT(FireAttack));
    }
}

// NONMATCHING: animation-name address caching and register allocation differ
void OtaRockTank::exeDash() {
    if (isNerve(NT(FallDashStart)) && MR::isFirstStep(this)) {
        MR::startActionSound(this, "FallDashStart", -1, -1, -1);
    }
    if (isNerve(NT(FallDashStart)) || isNerve(NT(BackDashStart))) {
        accelerateTankWheels();
        mWheelRot += mWheelSpeed;
        if (MR::isFirstStep(this)) {
            const char* action8 = "DashStart";
            MR::tryStartAllAnim(this, action8);
            MR::tryStartAllAnim(mChief, action8);
        }
        if (MR::isActionEnd(this)) {
            if (isNerve(NT(FallDashStart))) {
                setNerve(NT(FallDash));
                mTargetRailPointNo = 1;
                return;
            }
            setNerve(NT(BackDash));
            mTargetRailPointNo = 4;
        }
    } else {
        if (MR::isFirstStep(this)) {
            const char* action9 = "DashRun";
            MR::tryStartAllAnim(this, action9);
            MR::tryStartAllAnim(mChief, action9);
            mSpeed = 35.0f;
        }
        if (isNerve(NT(BackDash))) {
            MR::startActionSound(this, "BackDash", -1, -1, -1);
        } else if (isNerve(NT(FallDash))) {
            MR::startActionSound(this, "FallDash", -1, -1, -1);
        }
        mPosition.x += mRailDirection.x * mSpeed;
        mPosition.z += mRailDirection.z * mSpeed;
        mWheelRot += mWheelSpeed;
        TVec3f p(0.0f);
        MR::calcRailPointPos(&p, this, mTargetRailPointNo);
        f32 dist = MR::calcDistance(this, p);
        f32 limit = mLife == 1 ? sLowLifeStopRadius : 1200.0f;
        if (dist < limit) {
            if (isNerve(NT(FallDash))) {
                setNerve(NT(FallDashBrake));
                return;
            }
            setNerve(NT(BackDashBrake));
        }
    }
}

// NONMATCHING: animation-name address caching differs
void OtaRockTank::exeDashBrake() {
    if (MR::isFirstStep(this)) {
        mSpeedStart = mSpeed;
        MR::emitEffect(this, "BrakeL");
        MR::emitEffect(this, "BrakeR");
        if (isNerve(NT(BackDashBrake))) {
            const char* action10 = "Brake";
            MR::tryStartAllAnim(this, action10);
            MR::tryStartAllAnim(mChief, action10);
        }
    }
    MR::startActionSound(this, "Brake", -1, -1, -1);
    TVec3f dir(mRailDirection);
    TVec3f from(0.0f);
    TVec3f to(0.0f);
    MR::calcRailPointPos(&from, this, 4);
    MR::calcRailPointPos(&to, this, 1);
    MR::calcDirectionVec(&dir, to, from);
    bool isFall = isNerve(NT(FallDashBrake));
    if (isFall) {
        mTargetRailPointNo = 4;
        MR::turnDirection(this, &mRailDirection, dir, MR::cosDegree(4.0f));
        mPosition.x -= dir.x * mSpeed;
        mPosition.z -= dir.z * mSpeed;
    } else {
        mTargetRailPointNo = 5;
        TVec3f keep(dir);
        MR::calcRailDirectionAtCoord(&dir, this, MR::getRailPointCoord(this, 4));
        MR::turnDirection(this, &mRailDirection, dir, MR::cosDegree(4.0f));
        mPosition.x += keep.x * mSpeed;
        mPosition.z += keep.z * mSpeed;
    }
    mWheelRot += mWheelSpeed;
    if (MR::isGreaterEqualStep(this, 40)) {
        mSpeed *= 0.95f;
        accelerateTankWheels();
        if (!(mRailDirection.dot(dir) < 0.999f) && !MR::isLessStep(this, 100)) {
            mRailDirection.set(dir);
            MR::tryDeleteEffect(this, "BrakeL");
            MR::tryDeleteEffect(this, "BrakeR");
            if (isFall) {
                setNerve(NT(BackDash));
                return;
            }
            startTankRunAnimations();
            setNerve(NT(RunAfterDash));
            mCurrentRailPointNo = 4;
        }
    } else {
        updateTankBrakeSpeed(40, mSpeedStart, 20.0f);
        decelerateTankWheels();
        decelerateTankWheels();
        decelerateTankWheels();
    }
}

void OtaRockTank::exeDamage() {
    if (MR::isFirstStep(this)) {
        switch (mLife) {
        case 2:
            if (!MR::isEffectValid(this, "Weaken0")) {
                MR::tryEmitEffect(this, "Weaken0");
            }
            break;
        case 1:
            if (!MR::isEffectValid(this, "Weaken1")) {
                MR::tryEmitEffect(this, "Weaken1");
            }
            break;
        }
        MR::setBrkFrameAndStop(this, 0.0f);
        MR::startAction(this, "Damage");
        MR::startAction(mChief, "Damage");
        MR::stopScene(16);
        if (mLife > 0) {
            MR::emitItemBossDamage(this);
            MR::shakeCameraNormal();
            MR::startActionSound(this, "DamageHitSound", -1, -1, -1);
        } else {
            MR::shakeCameraStrong();
            MR::startActionSound(this, "DamageLastHitSound", -1, -1, -1);
            MR::stopStageBGM(10);
        }
        mShotIndex = 1;
        if (mLife <= 2) {
            mThrowNum = 3;
        }
        if (mCurrentRailPointNo != MR::getCurrentRailPointNo(this)) {
            mCurrentRailPointNo = MR::getCurrentRailPointNo(this);
        }
    }
    if (MR::isActionEnd(this)) {
        if (mLife <= 0) {
            if (MR::tryStartDemo(this, "DownDemo")) {
                setNerve(NT(DownDemo));
                MR::startAnimCameraTargetSelf(this, mDownDemoCameraInfo, "DownDemo", 0, false, 1.0f);
                MR::requestMovementOn(mChief);
            }
        } else {
            if (isNerve(NT(DamageB))) {
                setNerve(NT(DashStart));
                return;
            }
            setNerve(NT(RunStart));
        }
    }
}

void OtaRockTank::exeDownDemo() {
    if (MR::isFirstStep(this)) {
        MR::forceDeleteEffect(this, "Weaken0");
        MR::forceDeleteEffect(this, "Weaken1");
        MR::startAction(this, "DownDemo");
        MR::startAction(mChief, "DownDemo");
        mPrevRailDirection.set(mRailDirection);
        killTankProjectiles();
        MR::forceDeleteEffect(mChief, "AngrySmoke");
    }
    if (MR::isAnimCameraEnd(this, mDownDemoCameraInfo, "DownDemo") && MR::isActionEnd(this)) {
        MR::emitEffect(this, "DownDemoExplosion");
        mBreakModel->appear();
        setNerve(NT(Explosion));
    }
}

void OtaRockTank::exeExplosion() {
    if (MR::isFirstStep(this)) {
        MR::hideModel(this);
        mChief->kill();
        MR::startActionSound(this, "Explosion", -1, -1, -1);
    }
    if (MR::isDead(static_cast< LiveActor* >(mBreakModel)) || MR::isActionEnd(static_cast< LiveActor* >(mBreakModel))) {
        MR::overlayWithPreviousScreen(2);
        MR::endAnimCamera(this, mDownDemoCameraInfo, "DownDemo", 0, true);
        MR::endDemo(this, "DownDemo");
        kill();
    }
}

void OtaRockTank::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorEnemy(pReceiver)) {
        if (MR::isSensorEnemyAttack(pSender)) {
            MR::sendMsgEnemyAttack(pReceiver, pSender);
        }
    } else if (MR::isSensorPlayer(pReceiver)) {
        bool ret = false;
        if (isNerve(NT(DamageA)) || isNerve(NT(DamageB)) || isNerve(NT(DownDemo)) || isNerve(NT(Explosion))) {
            ret = true;
        }
        if (ret) {
            MR::sendMsgPush(pReceiver, pSender);
            return;
        }
        if (MR::sendMsgEnemyAttackStrongOrPush(pReceiver, pSender)) {
            return;
        }
    }
}

bool OtaRockTank::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (!MR::isMsgStarPieceReflect(msg)) {
        return false;
    }
    if (MR::isEqualSensor(pReceiver, this, "Body")) {
        mChief->startHitReaction();
        return true;
    }
    return true;
}

bool OtaRockTank::receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (!MR::isEqualString(MR::getSensorHost(pSender)->mName, "\x83\x84\x83\x56\x82\xCC\x8E\xC0\x92\x65")) {
        return false;
    }
    if (isNerve(NT(DamageA)) || isNerve(NT(DamageB))) {
        return true;
    }
    mLife--;
    MR::tryDeleteEffect(this, "BrakeL");
    MR::tryDeleteEffect(this, "BrakeR");
    if (isTankDashing()) {
        setNerve(NT(DamageB));
    } else {
        setNerve(NT(DamageA));
    }
    return true;
}

f32 OtaRockTank::getTankMinSpeed() {
    bool flag = false;
    if (isNerve(NT(RunAccel)) || isNerve(NT(RunKeep)) || isNerve(NT(RunRail)) || isNerve(NT(BrakeA))) {
        flag = true;
    }
    if (flag) {
        return 25.0f;
    }
    if (mLife == 1) {
        return 21.0f;
    }
    return 16.0f;
}

f32 OtaRockTank::getTankMaxSpeed() {
    bool flag = false;
    if (isNerve(NT(RunAccel)) || isNerve(NT(RunKeep)) || isNerve(NT(RunRail)) || isNerve(NT(BrakeA))) {
        flag = true;
    }
    if (flag) {
        return 35.0f;
    }
    if (mLife == 1) {
        return sLowLifeSpeed;
    }
    return 25.0f;
}

f32 OtaRockTank::getTankRailBrakeDistance() {
    bool flag = false;
    if (isNerve(NT(RunAccel)) || isNerve(NT(RunKeep)) || isNerve(NT(RunRail)) || isNerve(NT(BrakeA))) {
        flag = true;
    }
    if (flag) {
        return 1000.0f;
    }
    if (mLife == 1) {
        return 900.0f;
    }
    return 800.0f;
}

bool OtaRockTank::tryReachTankRailPoint() {
    if (MR::isRailReachedNearNextPoint(this, getTankRailBrakeDistance())) {
        return true;
    }
    moveTankAlongRail();
    approachTankCruiseSpeed();
    approachTankWheelSpeed();
    return false;
}

// NONMATCHING: vector subtraction inlining and temporary stack slots differ
void OtaRockTank::moveTankAlongRail() {
    TVec3f railPos;
    TVec3f railDir;
    MR::moveCoord(this, mSpeed);
    MR::calcRailPosAndDirectionAtCoord(&railPos, &railDir, this, MR::getRailCoord(this));
    TVec3f diff = railPos - mPosition;
    MR::vecKillElement(diff, mGravity, &diff);
    if (MR::normalizeOrZero(&diff, diff)) {
        diff = railDir;
    }
    TVec3f blend;
    MR::vecBlend(&blend, railDir, diff, 0.05f);
    if (MR::normalizeOrZero(&blend, blend)) {
        blend = railDir;
    }
    MR::turnDirection(this, &mRailDirection, blend, 0.9989f);
    mPosition += blend * mSpeed;
}

// NONMATCHING: temporary stack slots and instruction scheduling differ
void OtaRockTank::moveTankTowardRailDirection(const TVec3f& rDir, f32 scale) {
    if (mCurrentRailPointNo == MR::getCurrentRailPointNo(this)) {
        TVec3f railPos;
        TVec3f railDir;
        MR::calcRailPosAndDirectionAtCoord(&railPos, &railDir, this, MR::getRailCoord(this));
        f32 angle = MR::diffAngleAbs(railDir, rDir) / (f32)(*(const volatile s32*)&sAngleDivisor);  // read through a pointer: the target loads it from .sdata2 instead of folding 25
        MR::turnDirection(this, &mRailDirection, rDir, JMath::sSinCosTable.cosRadian(angle));
        TVec3f along(mRailDirection);
        MR::vecKillElement(along, railDir, &along);
        if (mRailDirection.dot(rDir) > 0.99f) {
            accelerateTankWheels();
        } else {
            decelerateTankWheels();
        }
        updateTankEaseOutSpeed(getNerveStep(), mSpeedStart);
        f32 ease = MR::getEaseInValue(mRailDirection.dot(rDir), 0.0f, 1.0f, 1.0f);
        mPosition.x += ease * (scale * (along.x * mSpeed)) + railDir.x * mSpeed;
        mPosition.z += ease * (scale * (along.z * mSpeed)) + railDir.z * mSpeed;
        MR::moveCoord(this, mSpeed);
        return;
    }
    TVec3f dirAt(0.0f);
    MR::calcRailDirectionAtCoord(&dirAt, this, MR::getRailPointCoord(this, mCurrentRailPointNo));
    accelerateTankWheels();
    updateTankEaseOutSpeed(getNerveStep(), mSpeedStart);
    mPosition.x += scale * (mRailDirection.x * mSpeed) + dirAt.x * mSpeed;
    mPosition.z += scale * (mRailDirection.z * mSpeed) + dirAt.z * mSpeed;
}

// NONMATCHING: vector subtraction is inlined into copy construction and subtraction assignment
void OtaRockTank::syncTankRailCoordinate() {
    TVec3f railPos;
    TVec3f railDir;
    MR::calcRailPosAndDirectionAtCoord(&railPos, &railDir, this, MR::getRailCoord(this));
    TVec3f diff = mPosition - railPos;
    MR::vecKillElement(diff, mGravity, &diff);
    f32 d = diff.dot(railDir);
    if (d > 0.0f) {
        MR::moveCoord(this, d);
    }
}

// NONMATCHING: integer conversion and floating-point scheduling differ
void OtaRockTank::updateTankEaseOutSpeed(s32 step, f32 target) {
    f32 base = 3.0f;
    bool flag = false;
    if (isNerve(NT(RunAccel)) || isNerve(NT(RunKeep)) || isNerve(NT(RunRail)) || isNerve(NT(BrakeA))) {
        flag = true;
    }
    if (flag) {
        base = 8.0f;
    }
    s32 totalStep = 65;
    f32 frame = step;
    f32 difference = target - base;
    f32 len = totalStep;
    if (mLife == 1) {
        len *= 0.9f;
    }
    mSpeed = base + (1.0f - MR::getEaseInValue(frame / len, 0.0f, 1.0f, 1.0f)) * difference;
    mSpeed = MR::clamp(mSpeed, base, getTankMinSpeed());
}

// NONMATCHING: saved-register allocation differs
void OtaRockTank::updateTankBrake(f32 target) {
    f32 base = 7.0f;
    s32 steps = 40;
    if (mLife == 1) {
        base = 10.0f;
    }
    bool flag = false;
    if (isNerve(NT(RunAccel)) || isNerve(NT(RunKeep)) || isNerve(NT(RunRail)) || isNerve(NT(BrakeA))) {
        flag = true;
    }
    if (flag) {
        base = 16.0f;
        steps = 15;
    }
    updateTankBrakeSpeed(steps, target, base);
}

// NONMATCHING: floating-point operand scheduling differs
void OtaRockTank::updateTankBrakeSpeed(s32 steps, f32 from, f32 to) {
    f32 diff = from - to;
    if (!(diff < 0.0f)) {
        f32 rate = MR::calcNerveRate(this, steps);
        mSpeed = to + MR::getEaseInValue(1.0f - rate, 0.0f, 1.0f, 1.0f) * diff;
    }
}

void OtaRockTank::accelerateTankMove() {
    f32 diff = getTankMaxSpeed() - 3.0f;
    s32 divisor = 5;
    f32 step = diff / divisor;
    mSpeed += step;
    mSpeed = MR::clamp(mSpeed, 3.0f, getTankMaxSpeed());
}

void OtaRockTank::approachTankCruiseSpeed() {
    f32 target = getTankMinSpeed();
    if (mSpeed > target) {
        mSpeed -= 0.2f;
        mSpeed = MR::clamp(mSpeed, target, getTankMaxSpeed());
        return;
    }
    if (mSpeed < target) {
        mSpeed += 0.2f;
        mSpeed = MR::clamp(mSpeed, 3.0f, target);
    }
}

void OtaRockTank::decelerateTankWheels() {
    mWheelSpeed -= 0.15f;
    mWheelSpeed = MR::clamp(mWheelSpeed, 3.0f, 18.0f);
}

void OtaRockTank::accelerateTankWheels() {
    mWheelSpeed += 0.4f;
    mWheelSpeed = MR::clamp(mWheelSpeed, 0.0f, 18.0f);
}

void OtaRockTank::approachTankWheelSpeed() {
    if (mWheelSpeed > 10.0f) {
        mWheelSpeed -= 0.15f;
        mWheelSpeed = MR::clamp(mWheelSpeed, 10.0f, 18.0f);
        return;
    }
    if (mWheelSpeed < 10.0f) {
        mWheelSpeed += 0.15f;
        mWheelSpeed = MR::clamp(mWheelSpeed, 0.0f, 10.0f);
    }
}

s32 OtaRockTank::countFreeTankFireBalls() {
    s32 count = 0;
    for (s32 i = 0; i < 48; i++) {
        if (MR::isDead(&mFireBalls[i])) {
            count++;
        }
    }
    return count;
}

CocoNutBall* OtaRockTank::findFreeTankCoconutBall() {
    for (s32 i = 0; i < 8; i++) {
        if (MR::isDead(&mCocoNutBalls[i])) {
            return &mCocoNutBalls[i];
        }
    }
    return nullptr;
}

FireBall* OtaRockTank::findFreeTankFireBall() {
    for (s32 i = 0; i < 48; i++) {
        if (MR::isDead(&mFireBalls[i])) {
            return &mFireBalls[i];
        }
    }
    return nullptr;
}

ScatterFireBall* OtaRockTank::findFreeTankScatterBall() {
    for (s32 i = 0; i < 24; i++) {
        if (mScatterBalls[i].isFullyDead()) {
            return &mScatterBalls[i];
        }
    }
    return nullptr;
}

bool OtaRockTank::tryFireTankVolley() {
    bool hasFreeBalls = countFreeTankFireBalls() > 0;
    if (hasFreeBalls) {
        fireTankSideBatteries();
    }
    fireTankMouth();
    mShotIndex = (mShotIndex + 1) % mThrowNum;
    MR::shakeCameraVeryWeak();
    MR::tryRumblePadWeak(this, 0);
    return true;
}

void OtaRockTank::fireTankSideBatteries() {
    TVec3f trans[6];
    TVec3f dir[6];
    for (s32 i = 0; i < 6; i++) {
        FixedPosition* pFixedPos = mFixedPositions[i];
        pFixedPos->calc();
        pFixedPos->mMtx.getTrans(trans[i]);
        pFixedPos->mMtx.getZDir(dir[i]);
    }
    MR::vecBlend(&dir[0], dir[0], dir[1], 0.5f);
    MR::vecBlend(&dir[2], dir[2], dir[1], 0.5f);
    MR::vecBlend(&dir[3], dir[3], dir[4], 0.5f);
    MR::vecBlend(&dir[5], dir[5], dir[4], 0.5f);
    for (s32 i = 0; i < 6; i++) {
        if (mDamageMask & (1 << static_cast< u16 >(i))) {
            MR::normalize(&dir[i]);
            MR::emitEffect(this, sThrowEffects[i]);
            fireTankFlame(trans[i], TVec3f(dir[i]));
        }
    }
}

// NONMATCHING: vector subtraction inlining and reuse of the 30-degree constant differ
void OtaRockTank::fireTankMouth() {
    FixedPosition* pFixedPos = mFixedPositions[6];
    pFixedPos->calc();
    TVec3f trans;
    pFixedPos->mMtx.getTrans(trans);
    TVec3f zdir;
    pFixedPos->mMtx.getZDir(zdir);
    TVec3f toPlayer = *MR::getPlayerPos() - trans;
    MR::normalizeOrZero(&toPlayer);
    TVec3f gravKilled;
    MR::vecKillElement(toPlayer, mGravity, &gravKilled);
    if (mDamageMask & 0x40) {
        if (zdir.dot(gravKilled) > 0.6f) {
            MR::vecBlend(&zdir, zdir, toPlayer, 1.0f);
        }
        MR::normalize(&zdir);
        MR::emitEffect(mChief, sThrowEffects[6]);
        f32 speed = mLife == 1 ? 34.0f : 32.0f;
        f32 angle = 5.0f + 2.5f * (f32)MR::getRandom(static_cast< s32 >(1), static_cast< s32 >(3));
        if (MR::getRandom(static_cast< s32 >(0), static_cast< s32 >(2)) == 1) {
            angle = -angle;
        }
        if (mShotIndex % mThrowNum == 0) {
            findFreeTankCoconutBall()->launchCoconutTowardPlayer(trans, speed, angle, 160.0f);
            return;
        }
        f32 offs = 30.0f;
        s32 i = 0;
        do {
            f32 a = 0.0f;
            if (i == 1) {
                a = offs;
            } else if (i == 2) {
                a = -offs;
            }
            findFreeTankScatterBall()->shootWithSpreadAngle(trans, speed, a);
            if (mLife == 1) {
                i++;
            } else {
                break;
            }
        } while (i < 3);
    }
}

bool OtaRockTank::isTankMeteorCoconut(u32 a, s32 b) {
    s32 numerator = 10;
    s32 divisor = 2;
    f32 half = (f32)numerator / (f32)divisor;
    u32 n = (u32)half;
    if ((half - (f32)n) > 0.0f) {
        n += 1;
    }
    return ((((a + 10) - b) % 10) % n) == 0;
}

void OtaRockTank::fireTankMeteorForward(u32 a, s32 b) {
    TVec3f dir(mRailDirection);
    TVec3f trans(0.0f);
    FixedPosition* pFixedPos = mFixedPositions[6];
    pFixedPos->calc();
    pFixedPos->mMtx.getTrans(trans);
    pFixedPos->mMtx.getZDir(dir);
    if (!isTankMeteorCoconut(a, b)) {
        ScatterFireBall* pBall = findFreeTankScatterBall();
        TVec3f vel = dir * 50.0f;
        pBall->shootWithVelocity(trans, vel);
        return;
    }
    CocoNutBall* pBall = findFreeTankCoconutBall();
    TVec3f vel = dir * 50.0f;
    pBall->launchCoconutWithVelocity(trans, vel);
}

#pragma dont_inline on
// NONMATCHING: vector temporary stack slots and multiply operand order differ
void OtaRockTank::fireTankMeteorAtTarget(u32 a, s32 b) {
    TVec3f vel = mGravity * (2900.0f + (f32)a * 100.0f);
    TVec3f pos = mTargetPoses[a] - vel;
    if (!isTankMeteorCoconut(a, b)) {
        ScatterFireBall* pBall = findFreeTankScatterBall();
        TVec3f v = mGravity * 8.0f;
        pBall->shootFalling(pos, v);
        return;
    }
    CocoNutBall* pBall = findFreeTankCoconutBall();
    TVec3f v = mGravity * 8.0f;
    pBall->launchFallingCoconut(pos, v);
}

void OtaRockTank::fireTankFlame(const TVec3f& rPos, const TVec3f& rDir) {
    f32 speed = mLife == 1 ? 34.0f : 32.0f;
    FireBall* pBall = findFreeTankFireBall();
    TVec3f vel = rDir * speed;
    pBall->launchFireBallWithVelocity(rPos, vel);
}

#pragma dont_inline reset
bool OtaRockTank::calcTankLeftWheelJoint(TPos3f* pMtx, const JointControllerInfo& rInfo) {
    return calcTankWheelJoint(pMtx, rInfo);
}

bool OtaRockTank::calcTankRightWheelJoint(TPos3f* pMtx, const JointControllerInfo& rInfo) {
    return calcTankWheelJoint(pMtx, rInfo);
}

bool OtaRockTank::calcTankWheelJoint(TPos3f* pMtx, const JointControllerInfo&) {
    if (isNerve(NT(StartDemo))) {
        return false;
    }
    MR::rotateMtxLocalXDegree(pMtx->toMtxPtr(), mWheelRot);
    return true;
}

// NONMATCHING: matrix-column load order differs
bool OtaRockTank::calcTankChiefJoint(TPos3f* pMtx, const JointControllerInfo&) {
    if (isTankChiefTrackingPlayer()) {
        TVec3f toPlayer;
        MR::calcVecToPlayerH(&toPlayer, this, nullptr);
        MR::vecBlend(&mPrevRailDirection, mPrevRailDirection, toPlayer, 0.05f);
    } else {
        mPrevRailDirection.set(mRailDirection);
    }
    if ((!isNerve(NT(StartDemo)) && !isNerve(NT(DownDemo))) || MR::isFirstStep(this)) {
        TVec3f up;
        up.set<f32>(pMtx->mMtx[0][1], pMtx->mMtx[1][1], pMtx->mMtx[2][1]);
        MR::makeMtxFrontUp(pMtx, mPrevRailDirection, up);
    }
    return true;
}

bool OtaRockTank::isTankDashing() {
    bool ret = false;
    if (isNerve(NT(FallDash)) || isNerve(NT(FallDashStart)) || isNerve(NT(FallDashBrake)) || isNerve(NT(BackDashStart)) || isNerve(NT(BackDash)) ||
        isNerve(NT(BackDashBrake))) {
        ret = true;
    }
    return ret;
}

bool OtaRockTank::canTankFireAtRailPoint() {
    s32 arg = -1;
    MR::getCurrentRailPointArg0NoInit(this, &arg);
    if (arg != 0) {
        f32 dist = 0.0f;
        MR::calcDistanceToNextRailPoint(this, &dist);
        f32 limit = 1200.0f;
        if (mLife == 1) {
            limit = 1500.0f;
        }
        if ((dist - getTankRailBrakeDistance()) > limit) {
            return true;
        }
    }
    return false;
}

void OtaRockTank::startTankRunAnimations() {
    MR::startAction(this, "Run");
    if (mLife == 1) {
        MR::startAction(mChief, "AngryWait");
        return;
    }
    MR::startAction(mChief, "Wait");
}

bool OtaRockTank::isTankChiefTrackingPlayer() {
    if (isNerve(NT(DamageA)) || isNerve(NT(StartDemo)) || isNerve(NT(DownDemo)) || isNerve(NT(RunAccel)) || isNerve(NT(RunKeep)) || isNerve(NT(RunRail)) ||
        isNerve(NT(BrakeA)) || isNerve(NT(Meteor)) || isNerve(NT(FallDashStart)) || isNerve(NT(FallDash)) || isNerve(NT(DashStart)) ||
        isNerve(NT(FallDashBrake)) || isNerve(NT(BackDashStart)) || isNerve(NT(BackDash)) || isNerve(NT(BackDashBrake)) || isNerve(NT(DamageB))) {
        return false;
    }
    return true;
}

bool OtaRockTank::tryStartTankDemo() {
    if (!MR::isValidSwitchA(this)) {
        return false;
    }
    if (!MR::isOnSwitchA(this)) {
        return false;
    }
    if (MR::tryStartDemo(this, "StartDemo")) {
        MR::tryForceDeleteEffectAll(this);
        MR::tryForceDeleteEffectAll(mChief);
        MR::startAnimCameraTargetSelf(this, mStartDemoCameraInfo, "StartDemo", 0, false, 1.0f);
        MR::requestMovementOn(mChief);
        setNerve(NT(StartDemo));
    }
    return false;
}

void OtaRockTank::killTankProjectiles() {
    for (s32 i = 0; i < 8; i++) {
        if (!MR::isDead(&mCocoNutBalls[i])) {
            mCocoNutBalls[i].kill();
        }
    }
    for (s32 i = 0; i < 48; i++) {
        if (!MR::isDead(&mFireBalls[i])) {
            mFireBalls[i].kill();
        }
    }
    for (s32 i = 0; i < 24; i++) {
        if (!mScatterBalls[i].isFullyDead()) {
            if (!MR::isDead(static_cast< LiveActor* >(&mScatterBalls[i]))) {
                static_cast< LiveActor* >(&mScatterBalls[i])->kill();
            }
            mScatterBalls[i].tryKillLava();
        }
    }
}

OtaRockTank::~OtaRockTank() {}

namespace {
    const f32 sLowLifeSpeed = 25.0f;
    const f32 sLowLifeStopRadius = 1200.0f;
    const s32 sAngleDivisor = 25;
}
