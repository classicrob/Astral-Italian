#include "Game/MapObj/MarioLauncher.hpp"
#include "Game/Animation/BckCtrl.hpp"
#include "Game/Camera/CameraTargetArg.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Map/HitInfo.hpp"
#include "Game/Screen/LayoutActor.hpp"
#include "Game/Screen/SimpleLayout.hpp"
#include "Game/Util.hpp"
#include "Game/Util/ActorCameraUtil.hpp"
#include "Game/Util/ActorMovementUtil.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/ActorSwitchUtil.hpp"
#include "Game/Util/CameraUtil.hpp"
#include "Game/Util/EffectUtil.hpp"
#include "Game/Util/GamePadUtil.hpp"
#include "Game/Util/GravityUtil.hpp"
#include "Game/Util/JMapInfo.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/LayoutUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/MtxUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include "Game/Util/StarPointerUtil.hpp"
#include "Game/Util/TriangleFilter.hpp"
#include "revolution/wpad.h"


namespace {
    static const f32 sEjectSpeed = 40.0f;
    static const f32 sGravityScale = 0.8f;
    static const f32 sGravityRatio = 0.95f;
    static const f32 sShellRadius = 80.0f;
    // static const f32 sShellBinder =
    static const f32 sShellOffset = 180.0f;
    static const f32 sGravityRoll = 0.01f;
    static const f32 sLandingSpeedRatio = 0.4f;
    static const s32 sLoopStart = 65;
    static const s32 sDieStart = 300;
    static const s32 sDispLaunch = 90;
    static const f32 sDPDCameraYRot = 0.3f;
    static const f32 sDPDCameraXRot = 0.3f;
    static const f32 sDPDCameraElevation = 0.62f;
    static const s32 sKillFrame = 360;
    static const s32 sStartActiveFrame = 10;
    static const s32 sInterpolateTest1 = 15;
    // static const f32 sLaunchCameraX = 0.0f;
    // static const f32 sLaunchCameraY =
    // static const f32 sLaunchCameraZ =
    // static const f32 sLaunchTargetY =
    // static const f32 sLaunchTargetZ =
    static const f32 sSensorRadius = 80.0f;
    static const f32 sSensorOffsetY = 20.0f;
    // static const ___ sRequestGuidance =
    static const s32 sPrepFrame = 26;
    static const s32 sRejectFrame = 20;
    static const s32 sWaitFrame = 30;
    static const s32 sLandFrame = 1;
    static const f32 sEarthenPipeInScale = 4.0f;
    static const f32 sCameraYOffset = 150.0f;
    static const f32 sCameraZOffset = 200.0f;
    static const f32 sRejectJumpY = 10.0f;
    static const f32 sRejectJumpZ = 10.0f;
    static const s32 sInvalidCollideFrame = 0;
    static const s32 sFrontVecFrame = 1;
    // static const f32 sLaunchTargetX =
    static const f32 sReboundScale0 = 0.0f;
    static const f32 sReboundScale1 = 0.0f;
    static const f32 sReboundScale2 = 0.0f;
};  // namespace

namespace NrvMarioLauncher {
    NEW_NERVE(MarioLauncherNrvWait, MarioLauncher, Wait);
    NEW_NERVE(MarioLauncherNrvReady, MarioLauncher, Ready);
    NEW_NERVE(MarioLauncherNrvPlayerIn, MarioLauncher, PlayerIn);
    NEW_NERVE(MarioLauncherNrvPrep, MarioLauncher, Prep);
    NEW_NERVE(MarioLauncherNrvAim, MarioLauncher, Aim);
    NEW_NERVE(MarioLauncherNrvFlight, MarioLauncher, Flight);
    NEW_NERVE(MarioLauncherNrvKill, MarioLauncher, Kill);
    NEW_NERVE(MarioLauncherNrvLand, MarioLauncher, Land);
    NEW_NERVE(MarioLauncherNrvReject, MarioLauncher, Reject);
};  // namespace NrvMarioLauncher

MarioLauncher::MarioLauncher(const char* pName)
    : LiveActor(pName), mLayout(), mShell(), mHost(), mIsInAttractor(), mCameraTargetMtx("カメラターゲットダミー"), _228(false), _22C(-1) {
    mEffectMtxSmoke.identity();
    mEffectMtxGravity.identity();
    mPlayerBaseMtx.identity();
    mLauncherBaseMtx.identity();
    mPlayerMtx.identity();
}

void MarioLauncher::init(const JMapInfoIter& rIter) {
    MR::initDefaultPos(this, rIter);
    MR::initActor(this, rIter, "MarioLauncher", false);
    initNerve(GET_NERVE(MarioLauncher, MarioLauncherNrvWait));

    mShell = new MarioLauncherShell("移動用弾丸");
    mShell->init(rIter);
    mLayout = new MarioLauncherLayout();
    mLayout->init(rIter);

    MR::connectToSceneMapObjStrongLight(this);

    initHitSensor(2);
    MR::addHitSensor(this, "Bind", 0x65 /* ATYPE_MARIO_LAUNCHER_BIND */, 8, ::sSensorRadius, TVec3f(0.0f, ::sSensorOffsetY, 0.0f));
    MR::addHitSensorMtxRide(this, "Shell", 8, ::sShellRadius, mShell->mBaseMtx, TVec3f(0.0f, ::sShellOffset, 0.0f));
    MR::invalidateHitSensor(this, "Shell");

    MR::initCollisionParts(this, "MarioLauncher", getSensor("Bind"), nullptr);
    MR::validateCollisionParts(this);

    initEffectKeeper(0, nullptr, false);
    initSound(4, "MarioLauncher", nullptr, TVec3f(0.0f));

    MR::initActorCameraProgrammable(this);
    MR::declareLauncherCamera();
    MR::declareLauncherFlightCamera();
    MR::makeMtxTR(mPlayerMtx.toMtxPtr(), this);
    MR::addBaseMatrixFollowTarget(this, rIter, &mPlayerMtx, MR::createBaseMatrixFollowValidateDelegator(this, &MarioLauncher::isFollowValid));

    mLauncherType = LauncherType_Front;
    MR::getJMapInfoArg0NoInit(rIter, &mLauncherType);
    MR::getJMapInfoArg1NoInit(rIter, &_228);

    MR::startAction(this, "Wait");

    if (MR::initUseStageSwitchReadA(this, rIter)) {
        MR::listenStageSwitchOnOffA(this, MR::Functor(this, &MarioLauncher::onSwitchA), MR::Functor(this, &MarioLauncher::offSwitchA));
        MR::invalidateHitSensors(this);
    }

    if (MR::initUseStageSwitchWriteB(this, rIter)) {
    }

    MR::setBindTriangleFilter(mShell, MR::createTriangleFilterDelegator(this, &MarioLauncher::isBreak));
    makeActorAppeared();
}

void MarioLauncherShell::init(const JMapInfoIter& rIter) {
    MR::initDefaultPos(this, rIter);
    MR::connectToSceneMapObjMovement(this);
    initBinder(::sShellRadius, ::sShellOffset, 8);
    makeActorDead();
}

void MarioLauncherLayout::init(const JMapInfoIter& rIter) {
    MR::connectToSceneLayout(this);
    initLayout();
}

void MarioLauncher::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::startBckNoInterpole(this, "Wait");
        MR::validateHitSensor(this, "Bind");
    }
}

void MarioLauncher::exeReady() {
    if (MR::isFirstStep(this)) {
        MR::startBckPlayer("EarthenPipeReady", nullptr);
        MR::startActionSound(this, "PmMarioLauncherPrep");
    }

    s32 maxFrame = MR::getBckFrameMaxPlayer();
    f32 f1 = MR::calcNerveEaseInRate(this, maxFrame);

    MR::blendMtx(mPlayerBaseMtx, mLauncherBaseMtx, MR::calcNerveRate(this, maxFrame), mPlayerMtx);

    TVec3f up;
    mLauncherBaseMtx.getYDir(up);

    if (MR::isBckStoppedPlayer()) {
        setNerve(GET_NERVE(MarioLauncher, MarioLauncherNrvPlayerIn));
    }
}

void MarioLauncher::exePlayerIn() {
    if (MR::isFirstStep(this)) {
        BckCtrlData data;
        data.mLoopMode = 0;
        data.mStartFrame = 0;
        data.mEndFrame = MR::getBckFrameMaxPlayer("EarthenPipeIn");
        data.mPlayFrame = data.mEndFrame / ::sEarthenPipeInScale;

        MR::startBckPlayer("EarthenPipeIn", data);
        MR::startLevelSoundPlayer("CatchV", -1, -1);
    }

    if (MR::isBckStoppedPlayer() && doPrep()) {
        setNerve(GET_NERVE(MarioLauncher, MarioLauncherNrvPrep));
    }
}

void MarioLauncher::exePrep() {
    if (MR::isStep(this, ::sStartActiveFrame)) {
        if (mLauncherType == LauncherType_Up) {
            MR::startAction(this, "LookUp");
        } else {
            MR::startAction(this, "Active");
        }
        MR::startActionSound(this, "OjMarioLauncherIn");
    }

    TVec3f front;
    MR::calcFrontVec(&front, this);

    if (MR::isStep(this, ::sPrepFrame)) {
        MR::startActionSound(this, "OjMarioLauncherBend");
    }

    if (MR::isGreaterStep(this, ::sStartActiveFrame) && MR::isActionEnd(this)) {
        if (doAim()) {
            MR::setPlayerFrontVec(front, ::sFrontVecFrame);
            setNerve(GET_NERVE(MarioLauncher, MarioLauncherNrvAim));
        }
    }
}

// NONMATCHING: target folds the %60 multiply to mulli with immediate; ours keeps 60 in a register (mullw), tried cast form
void MarioLauncher::exeAim() {
    if (MR::isFirstStep(this)) {
        MR::hideModel(this);
    }

    if (MR::isStarPointerInScreen(WPAD_CHAN0)) {
        mLayout->appearAt(MR::getStarPointerScreenPositionOrEdge(WPAD_CHAN0));
        MR::requestMarioLauncherGuidance();
    } else {
        mLayout->mLayout->kill();
        MR::requestMarioLauncherGuidance();
    }
    MR::startSystemLevelSE("SE_SY_LV_MARIO_LAUNCHER_AIM");

    if (MR::testSystemTriggerA() || MR::testCorePadTriggerA(WPAD_CHAN0)) {
        if (doEject()) {
            setNerve(GET_NERVE(MarioLauncher, MarioLauncherNrvFlight));
        }
        return;
    }

    if (MR::testSystemTriggerB() || MR::testCorePadTriggerB(WPAD_CHAN0)) {
        if (doReject()) {
            setNerve(GET_NERVE(MarioLauncher, MarioLauncherNrvReject));
        }
        return;
    }

    if (_228) {
        if (MR::isFirstStep(this)) {
            _22C = 300;
        }

        f32 remainder = _22C % 60;
        if (MR::isStarPointerInScreen(WPAD_CHAN0)) {
            if (--_22C <= 0) {
                if (doEject()) {
                    setNerve(GET_NERVE(MarioLauncher, MarioLauncherNrvFlight));
                }
            }
        } else if (_22C > 180) {
            _22C--;
        } else {
            _22C = 180;
        }
    }
}

void MarioLauncher::exeFlight() {
    if (MR::isFirstStep(this)) {
        MR::startActionSound(this, "PmMarioLauncherLaunch");
    }

    MR::startLevelSoundPlayer("SE_PM_LV_MARIO_LAUNCHER_FLY", -1);

    if (MR::isStep(this, ::sLoopStart)) {
        MR::startLevelSoundPlayer("JumpObj", -1, -1);
        MR::startBckPlayer("CannonFlyLoop", "MarioLauncher");
    }

    if (MR::isStep(this, ::sDieStart)) {
        MR::startBckPlayer("DieCannonFly", "MarioLauncher");
    }

    if (MR::isStep(this, ::sDispLaunch)) {
        MR::showModel(this);
    }

    if (MR::isGreaterEqualStep(this, ::sKillFrame)) {
        MR::forceKillPlayerByAbyss();
        setNerve(GET_NERVE(MarioLauncher, MarioLauncherNrvKill));
        return;
    }

    if (doFlight()) {
        if (doLanding(-1)) {
        }
    }
}

void MarioLauncher::exeKill() {
    mShell->calcBaseMtx();
    mPlayerMtx.set(mShell->getBaseMtx());
    MR::calcPlayerJointMtx(&mEffectMtxSmoke, "Center");
    mEffectMtxGravity = mEffectMtxSmoke;
}

void MarioLauncherShell::calcBaseMtx() {
    TQuat4f rot;

    TVec3f vel = mVelocity;
    TVec3f grav = mGravity;
    mReboundDir.set< f32 >(mPosition.x - mBaseMtx[0][3],   //
                           mPosition.y - mBaseMtx[1][3],   //
                           mPosition.z - mBaseMtx[2][3]);  //

    if (!MR::isNearZero(vel)) {
        MR::normalize(&vel);

        TVec3f front;
        mBaseMtx.getYDir(front);
        TVec3f down;
        mBaseMtx.getZDir(down);

        if (!MR::isNearZero(grav)) {
            MR::normalize(&grav);

            if (down.dot(grav) < 0.0f) {
                rot.setRotate(-down, grav, ::sGravityRoll);
            } else {
                rot.setRotate(down, grav, ::sGravityRoll);
            }

            mRotQuat.mult(rot);
            mBaseMtx.setQuat(mRotQuat);
        }

        rot.setRotate(front, vel);
        mRotQuat.mult(rot);
        mBaseMtx.setQuat(mRotQuat);
    }

    mBaseMtx[0][3] = mPosition.x;
    mBaseMtx[1][3] = mPosition.y;
    mBaseMtx[2][3] = mPosition.z;
}

MtxPtr MarioLauncherShell::getBaseMtx() const {
    return const_cast< TPos3f& >(mBaseMtx);
}

// NONMATCHING: vector temporary stack placement and register lifetimes differ.
void MarioLauncher::exeReject() {
    if (MR::isFirstStep(this)) {
        MR::startActionSound(this, "SyMarioLauncherOut");
    }

    if (MR::isStep(this, ::sRejectFrame - 2)) {
        MR::showPlayer();
    }

    if (MR::isStep(this, ::sRejectFrame)) {
        TVec3f jumpY;
        MR::extractMtxYDir(getBaseMtx(), &jumpY);
        TVec3f jumpZ;
        MR::extractMtxZDir(getBaseMtx(), &jumpZ);
        const TVec3f& jump = jumpY * ::sRejectJumpY - jumpZ * ::sRejectJumpZ;
        MR::endBindAndPlayerJump(this, jump, 0);
        MR::startLevelSoundPlayer("JumpSV", -1, -1);
    }

    if (MR::isStep(this, ::sWaitFrame)) {
        MR::validateHitSensor(this, "Bind");
        setNerve(GET_NERVE(MarioLauncher, MarioLauncherNrvWait));
    }
}

void MarioLauncher::exeLand() {
    if (MR::isGreaterEqualStep(this, ::sLandFrame)) {
        MR::endLauncherFlightCamera();
        MR::validateClipping(this);
        MR::validateClipping(mShell);
        setNerve(GET_NERVE(MarioLauncher, MarioLauncherNrvWait));
    }
}

void MarioLauncher::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorRide(pSender)) {
        if (!MR::tryGetItem(pSender, pReceiver) && MR::isSensorEnemy(pReceiver)) {
            MR::sendArbitraryMsg(0xF4 /* ACTMES_LAUNCHER_BREAKABLE */, pReceiver, pSender);
        }
    }
}

bool MarioLauncher::receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isEqualSensor(pReceiver, this, "Shell")) {
        return doLanding(msg);
    }

    return false;
}

bool MarioLauncher::receiveMsgPush(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isEqualSensor(pReceiver, this, "Shell")) {
        MR::sendMsgPlayerPunch(pSender, pReceiver);
        return doLanding(-1);
    }

    return false;
}

bool MarioLauncher::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgAutoRushBegin(msg)) {
        if (doReady(pSender)) {
            setNerve(GET_NERVE(MarioLauncher, MarioLauncherNrvReady));
            return true;
        }
    }

    if (MR::isMsgRushCancel(msg)) {
        return doLanding(-1);
    }

    if (MR::isMsgUpdateBaseMtx(msg)) {
        MR::setPlayerBaseMtx(mPlayerMtx);
        return true;
    }

    return false;
}

bool MarioLauncher::doReady(HitSensor* pSensor) {
    if (!isNerve(GET_NERVE(MarioLauncher, MarioLauncherNrvWait))) {
        return false;
    }

    mHost = MR::getSensorHost(pSensor);
    mPlayerBaseMtx.set(MR::getPlayerBaseMtx());
    mPlayerMtx.set(mPlayerBaseMtx);
    mLauncherBaseMtx.set(getBaseMtx());

    MR::tryPlayerKillTakingActor();

    return true;
}

bool MarioLauncher::doPrep() {
    if (!isNerve(GET_NERVE(MarioLauncher, MarioLauncherNrvPlayerIn))) {
        return false;
    }

    MR::invalidateHitSensor(this, "Bind");
    MR::invalidateClipping(this);
    MR::invalidateClipping(mShell);

    TPos3f mtx;
    mtx.set(getBaseMtx());
    MR::addTransMtxLocalY(mtx, ::sCameraYOffset);
    if (mLauncherType == LauncherType_Front) {
        MR::addTransMtxLocalZ(mtx, ::sCameraZOffset);
    }
    mCameraTargetMtx.setMtx(mtx);
    mShell->mBaseMtx.set(mtx);
    MR::setPosition(mShell, mPosition);
    MR::setRotation(mShell, mRotation);
    mShell->mGravity.zero();

    MR::hidePlayer();
    MR::resetPlayerStatus();
    MR::setPlayerStateWait();

    return true;
}

bool MarioLauncher::doAim() {
    mLayout->appearAt(MR::getStarPointerScreenPositionOrEdge(WPAD_CHAN0));
    MR::requestMarioLauncherGuidance();
    mLayout->appear();

    MR::startActionSound(this, "SyMarioLauncherIn");
    MR::startLauncherCamera(CameraTargetArg(&mCameraTargetMtx));
    MR::resetCameraMan();

    mPlayerMtx.set(mCameraTargetMtx.mMatrix);

    if (mLauncherType == LauncherType_Up) {
        MR::setLauncherCameraAngle(::sDPDCameraYRot, ::sDPDCameraXRot, MR::pi() / 2.0f, -1.0f);
    } else {
        MR::setLauncherCameraAngle(::sDPDCameraYRot, ::sDPDCameraXRot + 0.5f, ::sDPDCameraElevation, -1.0f);
    }

    if (MR::isValidSwitchB(this)) {
        MR::onSwitchB(this);
    }

    MR::startStarPointerModeMarioLauncher(this);
    setNerve(GET_NERVE(MarioLauncher, MarioLauncherNrvAim));

    return true;
}

bool MarioLauncher::doReject() {
    if (MR::isValidSwitchB(this)) {
        MR::offSwitchB(this);
    }

    TPos3f mtx;
    mtx.set(getBaseMtx());
    MR::addTransMtxLocalY(mtx, ::sCameraYOffset);
    MR::addTransMtxLocalZ(mtx, 0.0f);
    mPlayerMtx.set(mtx);

    MR::showModel(this);
    MR::startBckNoInterpole(this, "Wait");
    MR::endLauncherCamera();
    MR::resetCameraMan();
    MR::endStarPointerMode(this);
    MR::invalidateHitSensor(this, "Shell");
    mLayout->kill();

    return true;
}

void MarioLauncherLayout::kill() {
    mLayout->kill();
    LayoutActor::kill();
}

bool MarioLauncher::doEject() {
    if (!MR::isStarPointerInScreen(WPAD_CHAN0)) {
        return false;
    }

    TVec3f back;
    MR::calcUpVec(&back, this);
    if (mLauncherType == LauncherType_Up) {
        MR::calcFrontVec(&back, this);
        back *= -1.0f;
    }

    TVec3f shootDir;
    MR::calcWorldRayDirectionFromScreen(&shootDir, *MR::getStarPointerScreenPosition(WPAD_CHAN0));
    MR::normalize(&shootDir);

    MR::setPosition(mShell, mCameraTargetMtx.getPosition());
    mShell->mGravity.zero();
    TVec3f velocity(shootDir * ::sEjectSpeed);
    TVec3f& shellVel = mShell->mVelocity;
    shellVel.x = velocity.x;
    shellVel.y = velocity.y;
    shellVel.z = velocity.z;
    MR::makeMtxUpFront(&mShell->mBaseMtx, shootDir, -back);

    mShell->mBaseMtx.getQuat(mShell->mRotQuat);
    mShell->makeActorAppeared();

    MR::showPlayer();
    MR::startBckPlayer("CannonFlyStart", nullptr);

    MR::shakeCameraStrong();
    MR::tryRumblePadStrong(this, WPAD_CHAN0);

    MR::setEffectHostMtx(this, "LocusSmoke", mEffectMtxSmoke);
    MR::setEffectHostMtx(this, "Gravity", mEffectMtxGravity);

    MR::validateHitSensor(this, "Shell");
    mLayout->kill();

    MR::emitEffect(this, "LocusSmoke");
    mIsInAttractor = false;
    MR::endStarPointerMode(this);
    MR::endLauncherCamera();
    MR::startLauncherFlightCamera(60);

    return true;
}

// NONMATCHING: gravity temporary register lifetimes differ.
bool MarioLauncher::doFlight() {
    bool isInAttractor = false;

    MR::calcPlayerJointMtx(&mEffectMtxSmoke, "Center");
    mEffectMtxGravity = mEffectMtxSmoke;

    TVec3f attract;
    MR::calcAttractMarioLauncherOrZero(mShell, &attract, nullptr, 0);

    if (!MR::isNearZero(attract)) {
        isInAttractor = true;
    }

    TVec3f currGrav;
    currGrav = mShell->mGravity;
    attract *= (1.0f - ::sGravityRatio);
    currGrav *= ::sGravityRatio;
    const TVec3f& newGravity = attract + currGrav;
    mShell->mGravity = newGravity;

    if (isInAttractor != mIsInAttractor) {
        mIsInAttractor = isInAttractor;
    }

    TQuat4f gravRot;
    TVec3f grav = mShell->mGravity;

    if (isInAttractor) {
        MR::normalizeOrZero(&grav);

        TVec3f up, gravityEffectPos;

        mEffectMtxGravity.getTrans(gravityEffectPos);
        mEffectMtxGravity.setTrans(TVec3f(0.0f, 0.0f, 0.0f));
        mEffectMtxGravity.getYDir(up);

        gravRot.setRotate(-up, grav);

        TPos3f gravRotMtx;
        gravRotMtx.setQuat(gravRot);

        TPos3f rot;
        rot.concat(gravRotMtx, mEffectMtxGravity);
        mEffectMtxGravity.set(rot);
        mEffectMtxGravity.setTrans(gravityEffectPos);
        MR::emitEffect(this, "Gravity");
    } else {
        MR::deleteEffect(this, "Gravity");
    }

    if (testHitCollision()) {
        MR::reboundVelocityFromEachCollision(mShell, ::sReboundScale0, ::sReboundScale1, ::sReboundScale2, 0.0f);
        return MR::isGreaterEqualStep(this, ::sInvalidCollideFrame);
    }

    MR::addVelocityToGravity(mShell, ::sGravityScale);
    MR::restrictVelocity(mShell, ::sEjectSpeed);
    mShell->calcBaseMtx();

    mPlayerMtx.set(mShell->getBaseMtx());
    if (MR::isInWater(mShell->mPosition)) {
        return true;
    }

    return false;
}

bool MarioLauncher::doLanding(s32 msg) {
    if (!MR::isPlayerInRush()) {
        return false;
    }

    if (MR::isValidSwitchB(this)) {
        MR::offSwitchB(this);
    }

    MR::invalidateHitSensor(this, "Shell");
    MR::deleteEffect(this, "LocusSmoke");
    MR::deleteEffect(this, "Gravity");

    if (!MR::isBindedGround(mShell)) {
        MR::startBckPlayer("WallHit", "MarioLauncher");
    }

    if (msg != -1) {
        MR::endBindAndPlayerDamageMsg(this, msg, mShell->mReboundDir * ::sLandingSpeedRatio);
    } else {
        MR::endBindAndPlayerJump(this, mShell->mReboundDir * ::sLandingSpeedRatio, 0);
    }

    mShell->makeActorDead();
    MR::showModel(this);
    mHost = nullptr;

    MR::startAction(this, "Wait");
    setNerve(GET_NERVE(MarioLauncher, MarioLauncherNrvLand));

    return true;
}

bool MarioLauncher::testHitCollision() {
    return MR::isBinded(mShell) != false;
}

// NONMATCHING: saved-register allocation differs.
bool MarioLauncher::isBreak(const Triangle* pTriangle) {
    HitSensor* pSensor = pTriangle->mSensor;
    if (pSensor == getSensor("Bind")) {
        return true;
    }

    return MR::sendArbitraryMsg(0xF4 /* ACTMES_LAUNCHER_BREAKABLE */, pSensor, getSensor("Shell"));
}

void MarioLauncher::onSwitchA() {
    MR::validateHitSensor(this, "Bind");
}

void MarioLauncher::offSwitchA() {
    MR::invalidateHitSensor(this, "Bind");
}

bool MarioLauncher::isFollowValid(s32) const {
    return isNerve(GET_NERVE(MarioLauncher, MarioLauncherNrvAim)) || isNerve(GET_NERVE(MarioLauncher, MarioLauncherNrvFlight));
}

void MarioLauncherShell::calcAndSetBaseMtx() {
    LiveActor::calcAndSetBaseMtx();
}
