#pragma auto_inline off
#include "Game/Ride/Tamakoro.hpp"
#include "Game/GameAudio/AudTamakoroBgmCtrl.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/MapObj/Coin.hpp"
#include "Game/Util/ActorInitUtil.hpp"
#include "Game/Util/ActorAnimUtil.hpp"
#include "Game/Util/ActionSoundUtil.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/StringUtil.hpp"
#include "Game/Ride/SphereAccelSensorController.hpp"
#include "Game/Ride/TamakoroTutorial.hpp"
#include "Game/Scene/SceneFunction.hpp"
#include "Game/Util/ActorMovementUtil.hpp"
#include "Game/Util/BgmUtil.hpp"
#include "Game/Util/SupportPlayUtil.hpp"
#include "Game/Util/ActorShadowUtil.hpp"
#include "Game/Util/ActorSwitchUtil.hpp"
#include "Game/Util/EffectUtil.hpp"
#include "Game/Util/EventUtil.hpp"
#include "Game/Util/JointController.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/MtxUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/SceneUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include <revolution/mtx.h>
#ifdef GALAXY_NATIVE
#include "Game/Util/NativeActorObserve.hpp"
#endif
#include <revolution/wpad.h>


namespace {
    static const f32 mBaseRadius = 150.0f;
    static const f32 sBindableUpperDegree = 60.0f;
    static const s32 sBindStartTime = 40;
    static const f32 sBindStartJumpHeight = 150.0f;
    static const f32 sBindStartUpAdjustRate = 0.1f;
    static const f32 sBindStartFrontAdjustRate = 0.1f;
    static const s32 sBindStartLandTime = 14;
    static const f32 sLandStartPositionDegree = 35.0f;
    static const f32 sLandStartVelocityDegree = 40.0f;
    static const f32 sGravityAccel = 2.0f;
    static const f32 sGroundFric = 0.99f;
    static const f32 sIdleBckRate = 1.0f;
    static const f32 sSoftWalkSpeed = 0.25f;
    static const f32 sSoftWalkBckRate = 1.0f;
    static const f32 sWalkSpeed = 6.0f;
    static const f32 sWalkBckRate = 2.0f;
    static const f32 sRunSpeed = 18.0f;
    static const f32 sRunBckRate = 3.0f;
    static const f32 sRunMaxBckRate = 4.0f;
    static const s32 sFallStartTime = 5;
    static const f32 sFallOffsetSpeed = 3.0f;
    static const f32 sJumpPowerV = 40.0f;
    static const f32 sJumpPowerH = 7.0f;
    static const f32 sJumpOffsetSpeed = 4.0f;
    static const f32 sMarioJumpOffsetMax = 150.0f;
    static const f32 sMarioOffsetGravityAccel = 1.0f;
    static const f32 sMarioOffsetFric = 0.99f;
    static const f32 sNeedLandPower = 20.0f;
    static const s32 sLandTime = 2;
    static const f32 sLandFric = 0.88f;
    static const f32 sNeedBumpWallPower = 10.0f;
    static const f32 sCollisionStrongPower = 60.0f;
    static const f32 sCollisionMiddlePower = 25.0f;
    static const f32 sCollisionWeakPower = 5.0f;
    // static const f32 sStampReactionH =
    static const f32 sStampReactionV = 30.0f;
    static const f32 sStampReactionInvalidSpeed = 15.0f;
    static const f32 sExplosionReactionH = 20.0f;
    static const f32 sExplosionReactionV = 40.0f;
    static const f32 sEnemyRollReactionSpeed = 10.0f;
    static const f32 sRaidRailFastSpeed = 30.0f;
    static const f32 sRollBckRate = 1.5f;
    // static const f32 sRollMaxBckRate = ;
    static const s32 sDashRailTime = 40;
    static const f32 sDashRailFric = 0.99f;
    static const f32 sDashBallRollSpeed = 60.0f;
    static const f32 sEndBindFrontPower = 5.0f;
    static const f32 sEndBindJumpPower = 35.0f;
    static const s32 sForceBindEndTime = 90;
    static const f32 sMarioUpAdjustRate = 0.1f;
    static const f32 sMarioFrontAdjustRate = 0.25f;
    // static const f32 sStandDirMaxAccelDegree = ;
    static const f32 sStandDirInterRate = 0.02f;
    // static const ___ sMarioAccelDir = ; // ????
    static const f32 sMarioUpperYAccel = 5.0f;
    static const f32 sMarioUpperYFric = 0.9f;
    static const f32 sRingMaxDegree = 45.0f;
    static const f32 sRingMaxAngleSpeed = 4.0f;
    static const f32 sTutorialAccel = 0.5f;
    static const f32 sTutorialFric = 0.95f;
    static const s32 sBgmStateChangeFrames = 30;
};  // namespace

namespace NrvTamakoro {
    NEW_NERVE(TamakoroNrvStandByTutorial, Tamakoro, StandByTutorial);
    NEW_NERVE(TamakoroNrvStandByBind, Tamakoro, StandByBind);
    NEW_NERVE(TamakoroNrvBindStart, Tamakoro, BindStart);
    NEW_NERVE(TamakoroNrvBindStartLand, Tamakoro, BindStartLand);
    NEW_NERVE(TamakoroNrvTutorial, Tamakoro, Tutorial);
    NEW_NERVE(TamakoroNrvWait, Tamakoro, Wait);
    NEW_NERVE(TamakoroNrvFall, Tamakoro, Fall);
    NEW_NERVE(TamakoroNrvJump, Tamakoro, Jump);
    NEW_NERVE(TamakoroNrvBumpWall, Tamakoro, BumpWall);
    NEW_NERVE(TamakoroNrvLand, Tamakoro, Land);
    NEW_NERVE(TamakoroNrvFreeze, Tamakoro, Freeze);
    NEW_NERVE(TamakoroNrvJumpHole, Tamakoro, JumpHole);
    NEW_NERVE(TamakoroNrvJumpHoleSetUp, Tamakoro, JumpHoleSetUp);
    NEW_NERVE_ONEND(TamakoroNrvJumpHoleLaunch, Tamakoro, JumpHoleLaunch, JumpHoleLaunch);
    NEW_NERVE(TamakoroNrvDashRail, Tamakoro, DashRail);
    NEW_NERVE(TamakoroNrvDashRailEnd, Tamakoro, DashRailEnd);
    NEW_NERVE(TamakoroNrvRideRail, Tamakoro, RideRail);
    NEW_NERVE(TamakoroNrvRideRailFastStart, Tamakoro, RideRailFastStart);
    NEW_NERVE(TamakoroNrvRideRailFast, Tamakoro, RideRailFast);
    NEW_NERVE(TamakoroNrvRideRailFastEnd, Tamakoro, RideRailFastEnd);
    NEW_NERVE(TamakoroNrvBindEnd, Tamakoro, BindEnd);
};  // namespace NrvTamakoro

// Shared GameAudio layout: AudTamakoroBgmCtrl must be 0x2C bytes (target allocation), not 4.
Tamakoro::Tamakoro(const char* pName)
    : LiveActor(pName), mAccelSensorCtrl(nullptr), mTutorial(nullptr), mJointCtrl(nullptr), mPurpleCoin(nullptr), mBallRotateQuat(0, 0, 0, 1),
      mMarioRotateQuat(0, 0, 0, 1), mRingUp(0, 1, 0), mKickVel(0, 0, 0), mMarioBindRequestPos(0, 0, 0), mMarioUp(0, 1, 0), mMarioPos(0, 0, 0),
      mMoment(0, 0, 0), mMarioFront(0, 0, 1), mDirectionToMario(0, 0, 0), mAccelDir(0, 0, 0), mAccelRate(0.0f), mMarioRotateYAngle(0.0f),
      mMarioOffset(0.0f), mMarioOffsetVelocity(0.0f), mAirTime(0), _14C(0), mBgmCtrl(nullptr), mControlDisabled(false), mHasTutorial(false),
      mIsPurple(false) {
    mAccelSensorCtrl = new SphereAccelSensorController();
    mBgmCtrl = new AudTamakoroBgmCtrl();
}

void Tamakoro::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, "Tamakoro", false);
    initJointControl();
    initNerve(GET_NERVE(Tamakoro, TamakoroNrvStandByBind));
    MR::getJMapInfoArg0WithInit(rIter, &mIsPurple);
    if (!mIsPurple) {
        MR::declarePowerStar(this);
    }
    MR::initUseStageSwitchWriteA(this, rIter);

    s32 childNum = MR::getChildObjNum(rIter);
    for (s32 i = 0; i < childNum; i++) {
        const char* pChildName;
        MR::getChildObjName(&pChildName, rIter, i);
        if (MR::isEqualString(pChildName, "SignBoardTamakoro")) {
            mHasTutorial = true;
            mTutorial = new TamakoroTutorial("タマコロチュートリアル");
            mTutorial->mHost = this;
            MR::initChildObj(mTutorial, rIter, i);
            setNerve(GET_NERVE(Tamakoro, TamakoroNrvStandByTutorial));
            break;
        }
    }

    MR::tryInitFromRestartPos(this, rIter);
    MR::makeQuatFromRotate(&mBallRotateQuat, this);
    MR::startBva(this, "Tamakoro");
    if (mIsPurple) {
        MR::setBvaFrameAndStop(this, 2.0f);
        mPurpleCoin = MR::createPurpleCoin(this, "タマコロ用パープルコイン");
        mPurpleCoin->initWithoutIter();
        mPurpleCoin->makeActorDead();
        MR::startBrk(this, "Purple");
    } else {
        bool hasPowerStar = MR::hasPowerStarInCurrentStageWithDeclarer(mName, -1);
        MR::setBvaFrameAndStop(this, static_cast< s32 >(hasPowerStar));
        MR::startBrk(this, "Normal");
    }
    makeActorAppeared();
}

void Tamakoro::initJointControl() {
    mJointCtrl = MR::createJointDelegatorWithNullChildFunc(this, &Tamakoro::ringMtxCallBack, "Ring");
}

bool Tamakoro::ringMtxCallBack(TPos3f* pMtx, const JointControllerInfo& pJointCtrlInfo) {
    TVec3f trans;
    pMtx->getTrans(trans);

    if (MR::isNearZero(mAccelDir) || MR::isSameDirection(mAccelDir, mRingUp)) {
        MR::makeMtxUpNoSupport(pMtx, mRingUp);
    } else {
        MR::makeMtxUpFront(pMtx, mRingUp, mAccelDir);
    }

    pMtx->setTrans(trans);
    return true;
}

void Tamakoro::control() {
    updateMoment();
    MR::rotateQuatMoment(&mBallRotateQuat, mMoment);
    updateAirTime();
    updateRingUpVec();
    f32 speed = mVelocity.length();
    if (isNerve(GET_NERVE(Tamakoro, TamakoroNrvJumpHole)) || isNerve(GET_NERVE(Tamakoro, TamakoroNrvJumpHoleSetUp))) {
        speed = 0.0f;
    }
    if (_14C > 0) {
        _14C--;
    }
    mBgmCtrl->control(speed, mControlDisabled, ::sBgmStateChangeFrames);
    mControlDisabled = false;
#ifdef GALAXY_NATIVE
    {
        const bool riding = !isNerve(GET_NERVE(Tamakoro, TamakoroNrvStandByBind)) && !isNerve(GET_NERVE(Tamakoro, TamakoroNrvStandByTutorial)) &&
                            !isNerve(GET_NERVE(Tamakoro, TamakoroNrvBindEnd));
        MR::Native::publishActor("StarBall", mPosition, -mGravity, riding ? 1 : 0, riding ? PETARI_ACTOR_BOUND : PETARI_ACTOR_READY);
        MR::Native::publishActor("StarBallInput", mPosition, mAccelDir * mAccelRate, MR::isBindedGround(this) ? 1 : 0, 0);
    }
#endif
}

void Tamakoro::calcAndSetBaseMtx() {
    MR::setBaseTRMtx(this, mBallRotateQuat);
    mJointCtrl->registerCallBack();
}

void Tamakoro::updateBindActorMatrix() {
    if (isUseMarioOffset()) {
        mMarioPos.scaleAdd(mBaseRadius, mDirectionToMario, mPosition);
        if (mMarioOffset > 0.0f) {
            mMarioPos.scaleAdd(-mMarioOffset, mGravity, mMarioPos);
        }
    }

    // quaternion rotation
    TPos3f mtx;
    mtx.setQT(mMarioRotateQuat, mMarioPos);
    MR::setPlayerBaseMtx(mtx);
}

void Tamakoro::updateRingUpVec() {
    TVec3f up = -mGravity;

    if (!MR::isNearZero(mAccelDir)) {
        TVec3f v1;
        TVec3f up2 = -mGravity;
        v1.scaleAdd(mAccelRate, mAccelDir, up2);
        MR::normalizeOrZero(&v1);
        MR::turnVecToVecDegree(&up, up, v1, ::sRingMaxDegree, TVec3f(0, 1, 0));
    }

    MR::turnVecToVecDegree(&mRingUp, mRingUp, up, ::sRingMaxAngleSpeed, TVec3f(0, 1, 0));
}

void Tamakoro::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorType(pSender, 0xD) && MR::tryGetItem(pSender, pReceiver)) {
        return;
    }

    if ((MR::isSensorPlayerOrYoshi(pReceiver) || MR::isSensorEnemy(pReceiver)) && isEnablePushPlayer()) {
        MR::sendMsgPush(pReceiver, pSender);
        return;
    }

    if (isEnableEnemyAttack() && MR::isSensorEnemy(pReceiver) && MR::isSensorType(pSender, 0xD)) {
        if (MR::sendMsgPlayerTrample(pReceiver, pSender)) {
            f32 downVel = MR::calcGravitySpeed(this);
            if (downVel < ::sStampReactionInvalidSpeed) {
                downVel = -::sStampReactionV - downVel;
                mVelocity.add(mGravity * downVel);
                MR::tryRumblePadWeak(this, WPAD_CHAN0);
                MR::shakeCameraNormalWeak();
            }
            return;
        }

        if (MR::sendMsgToEnemyAttackRoll(pReceiver, pSender)) {
            return;
        }
    }

    if (isEnableBallBind() && pSender == getSensor("Body")) {
        if (isBindedSphereDash(pSender, pReceiver)) {
            setNerve(GET_NERVE(Tamakoro, TamakoroNrvDashRail));
            return;
        }

        if (isBindedJumpHole(pSender, pReceiver)) {
            setNerve(GET_NERVE(Tamakoro, TamakoroNrvJumpHole));
            return;
        }

        if (isBindedBallRail(pSender, pReceiver)) {
            setNerve(GET_NERVE(Tamakoro, TamakoroNrvRideRail));
            return;
        }
    }
}

bool Tamakoro::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgStarPieceReflect(msg)) {
        return true;
    }
    return false;
}

bool Tamakoro::receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgExplosionAttack(msg) && isEnableEnemyAttack()) {
        MR::tryRumblePadStrong(this, WPAD_CHAN0);
        MR::setVelocitySeparateHV(this, pSender, pReceiver, ::sExplosionReactionH, ::sExplosionReactionV);
        MR::startSoundPlayer("SE_PV_UPSET", -1, -1);
        return true;
    }

    if (MR::isMsgToEnemyAttackRoll(msg) && isEnableEnemyAttack()) {
        TVec3f dir;
        MR::calcSensorHorizonNormalize(&dir, mGravity, pSender, pReceiver);
        if (mVelocity.dot(dir) < ::sEnemyRollReactionSpeed) {
            if (_14C <= 0) {
                MR::tryHitReactionBetweenSensors(this, "衝突", pSender, pReceiver);
                _14C = 6;
            }
            MR::addVelocity(this, dir * ::sEnemyRollReactionSpeed);
        }
        return true;
    }
    return false;
}

bool Tamakoro::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgAutoRushBegin(msg) && requestBind(pSender)) {
        return true;
    }

    if (MR::isMsgUpdateBaseMtx(msg)) {
        updateBindActorMatrix();
        return true;
    }

    if (MR::isMsgRushForceCancel(msg) || MR::isMsgRushCancel(msg)) {
        setNerve(GET_NERVE(Tamakoro, TamakoroNrvStandByBind));
        return true;
    }

    if (msg == 0xA5 && isNerve(GET_NERVE(Tamakoro, TamakoroNrvJumpHole))) {
        setNerve(GET_NERVE(Tamakoro, TamakoroNrvJumpHoleSetUp));
        return true;
    }

    if (msg == 0xA6 && isNerve(GET_NERVE(Tamakoro, TamakoroNrvJumpHoleSetUp))) {
        setNerve(GET_NERVE(Tamakoro, TamakoroNrvJumpHoleLaunch));
        return true;
    }

    if (msg == 0xA7 && isNerve(GET_NERVE(Tamakoro, TamakoroNrvJumpHoleLaunch))) {
        setNerve(GET_NERVE(Tamakoro, TamakoroNrvFall));
        return true;
    }

    if (msg == 0xA8 && isNerve(GET_NERVE(Tamakoro, TamakoroNrvDashRail))) {
        setNerve(GET_NERVE(Tamakoro, TamakoroNrvDashRailEnd));
        return true;
    }

    if (msg == 0xA9 && isRideRail()) {
        setNerve(GET_NERVE(Tamakoro, TamakoroNrvWait));
        return true;
    }

    if (msg == 0xAA && requestEndBind()) {
        return true;
    }

    return false;
}

bool Tamakoro::requestBind(HitSensor* pSensor) {
    if (!isNerve(GET_NERVE(Tamakoro, TamakoroNrvStandByBind))) {
        return false;
    }

    // check if sensor within vertical angle range
    TVec3f vThisToSensor = *MR::getSensorPos(pSensor) - mPosition;
    MR::normalizeOrZero(&vThisToSensor);
    f32 cosPositionDegree = vThisToSensor.dot(-mGravity);
    if (cosPositionDegree < MR::cosDegree(::sBindableUpperDegree)) {
        return false;
    }

    MR::invalidateClipping(this);

    MR::makeQuatRotateDegree(&mMarioRotateQuat, *MR::getPlayerRotate());
    mMarioPos.set(*MR::getPlayerPos());
    mMarioRotateQuat.getZDir(mMarioFront);
    mMarioFront.orthogonalize(mGravity);
    if (MR::normalizeOrZero(&mMarioFront)) {
        MR::makeAxisVerticalZX(&mMarioFront, mGravity);
    }

    mMarioBindRequestPos.set(mMarioPos);
    mMarioRotateYAngle = 0.0f;
    mAccelRate = 0.0f;
    mMarioOffset = 0.0f;
    mMarioOffsetVelocity = 0.0f;

    mAccelDir.zero();
    MR::zeroVelocity(this);
    MR::invalidateHitSensor(this, "Bind");

    TVec3f vel(*MR::getPlayerVelocity());
    MR::normalizeOrZero(&vel);
    f32 cosVelocityDegree = -vThisToSensor.dot(vel);

    if (cosVelocityDegree > MR::cosDegree(::sLandStartVelocityDegree) && cosPositionDegree > MR::cosDegree(::sLandStartPositionDegree)) {
        // if angle between vel and "to this" < ::sLandStartVelocityDegree (40.0f) degrees and vertical angle to sensor < ::sLandStartPositionDegree
        // (35.0f) degrees
        setNerve(GET_NERVE(Tamakoro, TamakoroNrvBindStartLand));
    } else {
        setNerve(GET_NERVE(Tamakoro, TamakoroNrvBindStart));

        TVec3f v3;  // vec order shenanigans... not used
        TVec3f vPlayerToThis;
        vPlayerToThis = mPosition - mMarioPos;
        vPlayerToThis.orthogonalize(mGravity);

        if (!MR::normalizeOrZero(&vPlayerToThis)) {
            if (vPlayerToThis.dot(mMarioFront) > 0.0f) {
                mMarioFront.set(vPlayerToThis);
            } else {
                mMarioFront.set(-vPlayerToThis);
            }
        }
    }

    if (!isNeedTutorial()) {
        MR::stopStageBGM(40);
    }
    return true;
}

bool Tamakoro::requestEndBind() {
    if (!isNerve(GET_NERVE(Tamakoro, TamakoroNrvBindEnd)) && !isNerve(GET_NERVE(Tamakoro, TamakoroNrvStandByBind)) &&
        !isNerve(GET_NERVE(Tamakoro, TamakoroNrvStandByTutorial))) {
        MR::startBckPlayer("SwingRopeSpin", nullptr);
        MR::endBindAndPlayerForceJump(this, mMarioFront * (-::sEndBindFrontPower) + mGravity * (-::sEndBindJumpPower), 0);
        MR::hideModel(this);
        MR::invalidateHitSensors(this);
        MR::invalidateClipping(this);
        MR::zeroVelocity(this);
        mAccelSensorCtrl->notifyDeactivate();
        setNerve(GET_NERVE(Tamakoro, TamakoroNrvBindEnd));
        return true;
    }
    return false;
}

void Tamakoro::startRide() {
    if (isNerve(GET_NERVE(Tamakoro, TamakoroNrvStandByTutorial))) {
        setNerve(GET_NERVE(Tamakoro, TamakoroNrvStandByBind));
    }
}

bool Tamakoro::requestTutorialEnd() {
    if (isNerve(GET_NERVE(Tamakoro, TamakoroNrvTutorial))) {
        MR::zeroVelocity(this);

        startTamakoroBgm();
        setNerve(GET_NERVE(Tamakoro, TamakoroNrvWait));
        return true;
    }
    return false;
}

bool Tamakoro::tryFall() {
    if (mAirTime > ::sFallStartTime) {
        setNerve(GET_NERVE(Tamakoro, TamakoroNrvFall));
        return true;
    }
    return false;
}

bool Tamakoro::tryJump() {
    if (mAccelSensorCtrl->calcJumpPower() && MR::isBindedGround(this)) {
        setNerve(GET_NERVE(Tamakoro, TamakoroNrvJump));
        return true;
    }
    return false;
}

bool Tamakoro::tryBumpWall() {
    if (MR::calcHitPowerToWall(this) > ::sNeedBumpWallPower) {
        f32 rebound = -0.6f;
        if (isEnebleHitCollisionMessage()) {
            if (MR::sendMsgToWallSensor(0x41, getSensor("Body"))) {
                rebound = 0.3f;
            }
        }
        MR::reboundVelocityFromCollision(this, rebound, 0.0f, 1.0f);
        mMarioOffsetVelocity = 2.0f;
        setNerve(GET_NERVE(Tamakoro, TamakoroNrvBumpWall));
        return true;
    }
    return false;
}

bool Tamakoro::tryLand() {
    f32 power = MR::calcHitPowerToGround(this);

    if (MR::calcHitPowerToGround(this) > ::sNeedLandPower) {
        f32 rebound = 0.0f;
        if (isEnebleHitCollisionMessage()) {
            if (MR::sendMsgToWallSensor(0x42, getSensor("Body"))) {
                rebound = 0.3f;
            }
        }
        mMarioOffsetVelocity = -power;
        mMarioOffset -= power;
        MR::reboundVelocityFromCollision(this, rebound, 0.0f, 1.0f);
        setNerve(GET_NERVE(Tamakoro, TamakoroNrvLand));
        return true;
    }

    if (MR::isGreaterStep(this, ::sLandTime) && MR::isBindedGround(this)) {
        mMarioOffsetVelocity = -power;
        setNerve(GET_NERVE(Tamakoro, TamakoroNrvWait));
        return true;
    }

    return false;
}

bool Tamakoro::tryBindStarPointer() {
    if (MR::attachSupportTicoToTarget(this) && MR::isBindedGround(this)) {
        setNerve(GET_NERVE(Tamakoro, TamakoroNrvFreeze));
        return true;
    }
    return false;
}

void Tamakoro::reactionCollision() {
    f32 hitPower;

    f32 wallHitPower = MR::calcHitPowerToWall(this);
    f32 groundHitPower = MR::calcHitPowerToGround(this);

    if (wallHitPower >= groundHitPower) {
        hitPower = wallHitPower;
    } else {
        hitPower = groundHitPower;
    }

    if (wallHitPower > ::sCollisionWeakPower) {
        MR::emitEffectHit(this, MR::getWallHitPos(this), MR::getWallNormal(this), "WallHit");
    }

    if (groundHitPower > ::sCollisionMiddlePower) {
        MR::emitEffectHit(this, MR::getGroundHitPos(this), MR::getGroundNormal(this), "Land");
    }

    if (wallHitPower > ::sCollisionWeakPower) {
        MR::startActionSound(this, "SmIronsphHit", -1, -1, -1);
        MR::startSoundPlayer("SE_PV_GUARD", -1, -1);
    }

    if (hitPower > ::sCollisionStrongPower) {
        MR::tryRumblePadStrong(this, WPAD_CHAN0);
        MR::shakeCameraStrong();
    } else if (hitPower > ::sCollisionMiddlePower) {
        MR::tryRumblePadMiddle(this, WPAD_CHAN0);
        MR::shakeCameraNormalWeak();
    } else if (hitPower > ::sCollisionWeakPower) {
        MR::tryRumblePadWeak(this, WPAD_CHAN0);
        MR::shakeCameraWeak();
    }
}

void Tamakoro::exeStandByTutorial() {
    updateVelocityNormal(0.95f);
}

void Tamakoro::exeStandByBind() {
    if (MR::isFirstStep(this)) {
        MR::validateHitSensor(this, "Bind");
    }
    updateVelocityNormal(0.95f);
}

void Tamakoro::exeBindStart() {
    if (MR::isFirstStep(this)) {
        MR::startBckPlayer("SlideStomachRecover", nullptr);
        MR::tryRumblePadWeak(this, WPAD_CHAN0);
        mDirectionToMario.set(-mGravity);
        if (MR::normalizeOrZero(&mMarioFront)) {
            MR::makeAxisVerticalZX(&mMarioFront, mGravity);
        }
        MR::emitEffectHit(this, mMarioPos, mMarioPos - mPosition, "TamakoroLand");
    }

    // smoothly transition arc from bind request position
    // to top of the ball using the nerve rate as time
    f32 time = MR::calcNerveRate(this, ::sBindStartTime);
    TVec3f expectMario;
    expectMario.scaleAdd(mBaseRadius, mDirectionToMario, mPosition);
    TVec3f horizontalVec;
    MR::vecBlend(&horizontalVec, mMarioBindRequestPos, expectMario, time);
    time = (time * 2.0f) - 1.0f;
    TVec3f upVec;
    upVec.set(mGravity * (-(time * time * -::sBindStartJumpHeight + mBaseRadius)));

    MR::blendQuatUpFront(&mMarioRotateQuat, -mGravity, mMarioFront, ::sBindStartUpAdjustRate, ::sBindStartFrontAdjustRate);

    mMarioPos.set(horizontalVec + upVec);

    if (MR::isGreaterStep(this, ::sBindStartTime)) {
        setNerve(GET_NERVE(Tamakoro, TamakoroNrvBindStartLand));
    }
}

void Tamakoro::exeBindStartLand() {
    if (MR::isFirstStep(this)) {
        if (!isNeedTutorial()) {
            startTamakoroBgm();
        }
        MR::startBckPlayer("Land", "BallLand");
        MR::tryRumblePadMiddle(this, WPAD_CHAN0);

        mDirectionToMario.set(mMarioPos - mPosition);
        if (MR::normalizeOrZero(&mDirectionToMario)) {
            mDirectionToMario.set(-mGravity);
        }
        mMarioPos.scaleAdd(mBaseRadius, mDirectionToMario, mPosition);

        mMarioUp = (mDirectionToMario - mGravity) / 2.0f;
        if (MR::normalizeOrZero(&mMarioUp)) {
            mMarioUp.set(-mGravity);
        }

        MR::emitEffectHit(this, mMarioPos, mMarioPos - mPosition, "TamakoroLand");
        MR::startSoundPlayer("SE_PV_CATCH", -1, -1);
        MR::startActionSound(this, "SmRideStart", -1, -1, -1);

        if (MR::isValidSwitchA(this)) {
            MR::onSwitchA(this);
        }
    }

    MR::blendQuatUpFront(&mMarioRotateQuat, mMarioUp, mMarioFront, ::sBindStartUpAdjustRate, ::sBindStartFrontAdjustRate);

    if (MR::isGreaterStep(this, ::sBindStartLandTime)) {
        if (isNeedTutorial()) {
            mTutorial->requestStart();
            setNerve(GET_NERVE(Tamakoro, TamakoroNrvTutorial));
        } else {
            if (mTutorial != nullptr) {
                mTutorial->requestEnd();
            }
            setNerve(GET_NERVE(Tamakoro, TamakoroNrvWait));
        }
    }
}

void Tamakoro::exeTutorial() {
    if (MR::isFirstStep(this)) {
        MR::startBckPlayerJ("タマコロ移動");
        MR::zeroVelocity(this);
        mAccelDir.zero();
        mAccelRate = 0.0f;
        mMoment.zero();
        mKickVel.zero();
    }

    mAccelRate = mAccelSensorCtrl->calcMoveVector(&mAccelDir, mGravity);

    mKickVel.scaleAdd(mAccelRate * ::sTutorialAccel, mAccelDir, mKickVel);

    TVec3f moment;
    MR::calcMomentRollBall(&moment, mKickVel, -mGravity, mBaseRadius);
    MR::vecBlend(&mMoment, mMoment, moment, 0.25f);
    mKickVel.mult(::sTutorialFric);

    updateMoveBckBlend(mMoment.length() * mBaseRadius);
    updateMarioPose(3.0f);
    updateMarioOffset();
}

void Tamakoro::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::startBckPlayerJ("タマコロ移動");
    }
    reactionCollision();
    addVelocityOperate();
    updateVelocityNormal(::sGroundFric);
    updateMoveBckBlend(mMoment.length() * mBaseRadius);
    updateMarioPose(3.0f);
    updateMarioOffset();

    if (tryJump() || tryFall() || tryBindStarPointer()) {
        return;
    }
}

void Tamakoro::exeFall() {
    if (MR::isFirstStep(this)) {
        MR::startBckPlayer("Fall", "BallFall");
    }
    mMarioOffset += ::sFallOffsetSpeed;
    mMarioOffset = MR::clamp(mMarioOffset, 0.0f, ::sMarioJumpOffsetMax);

    reactionCollision();

    if (!tryLand()) {
        addVelocityOperate();
        updateVelocityNormal(::sGroundFric);
        updateMarioPose(3.0f);
    }
}

void Tamakoro::exeJump() {
    if (MR::isFirstStep(this)) {
        MR::startBckPlayer("BallJump", "BallJump");
        MR::startSoundPlayer("SE_PV_JUMP_M", -1, -1);
        MR::startActionSound(this, "SmBalloonsphJump", -1, -1, -1);
        MR::addVelocityToGravity(this, (-::sJumpPowerV - MR::calcGravitySpeed(this)));
        MR::addVelocityMoveToDirection(this, mAccelDir, mAccelRate * ::sJumpPowerH);
    }

    mMarioOffset += ::sJumpOffsetSpeed;
    mMarioOffset = MR::clamp(mMarioOffset, 0.0f, ::sMarioJumpOffsetMax);

    reactionCollision();

    if (tryBumpWall() || tryLand()) {
        return;
    }

    addVelocityOperate();
    updateVelocityNormal(::sGroundFric);
    updateMarioPose(3.0f);
}

// NONMATCHING: equivalent early-return branch layout differs.
void Tamakoro::exeBumpWall() {
    if (MR::isFirstStep(this)) {
        MR::startBckPlayer("BallHit", nullptr);
        MR::tryRumblePadMiddle(this, WPAD_CHAN0);
        MR::shakeCameraNormalWeak();
    }

    reactionCollision();

    addVelocityOperate();
    updateVelocityNormal(::sGroundFric);
    updateMarioPose(3.0f);
    updateMarioOffset();

    if (tryJump()) {
        return;
    }
    if (tryBindStarPointer()) {
        return;
    } else {
        if (MR::isGreaterStep(this, 15)) {
            setNerve(GET_NERVE(Tamakoro, TamakoroNrvWait));
        }
    }
}

// NONMATCHING: equivalent early-return branch layout differs.
void Tamakoro::exeLand() {
    if (MR::isFirstStep(this)) {
        MR::startBckPlayer("Land", nullptr);
        MR::tryRumblePadMiddle(this, WPAD_CHAN0);
        MR::shakeCameraNormalWeak();
        MR::startActionSound(this, "SmIronsphLand", -1, -1, -1);
    }

    reactionCollision();

    addVelocityOperate();
    updateVelocityNormal(::sLandFric);
    updateMarioPose(3.0f);
    updateMarioOffset();

    if (tryJump()) {
        return;
    }
    if (tryBindStarPointer()) {
        return;
    } else {
        if (MR::isGreaterStep(this, 15)) {
            setNerve(GET_NERVE(Tamakoro, TamakoroNrvWait));
        }
    }
}

void Tamakoro::exeFreeze() {
    if (MR::isFirstStep(this)) {
        MR::startBckPlayerJ("タマコロ移動");
    }
    addVelocityOperate();
    updateMarioPose(3.0f);
    updateMarioOffset();
    updateVelocityNormal(0.99f);
    MR::attenuateVelocityExceptDirection(this, mGravity, 0.2f);
    if (!MR::attachSupportTicoToTarget(this)) {
        setNerve(GET_NERVE(Tamakoro, TamakoroNrvWait));
    }
}

void Tamakoro::exeJumpHole() {
    if (MR::isFirstStep(this)) {
        MR::tryRumblePadWeak(this, WPAD_CHAN0);
        MR::offBind(this);
        MR::startBckPlayerJ("タマコロしゃがみ");
        mAccelDir.zero();
        mAccelRate = 0.0f;
        mMarioOffsetVelocity = 0.0f;
    }
    updateSquatBckBlend(mMoment.length() * mBaseRadius);
    updateMarioPose(3.0f);
    updateMarioOffset();
}

void Tamakoro::exeJumpHoleSetUp() {
    if (MR::isFirstStep(this)) {
        MR::tryRumblePadWeak(this, WPAD_CHAN0);
    }
    updateSquatBckBlend(0.0f);
    updateMarioPose(3.0f);
    updateMarioOffset();
}

void Tamakoro::exeJumpHoleLaunch() {
    if (MR::isFirstStep(this)) {
        MR::startBckPlayer("BallJump", nullptr);
        MR::tryRumblePadStrong(this, WPAD_CHAN0);
        MR::shakeCameraStrong();
        MR::emitEffect(this, "BigJumpBlur");
    }
    mControlDisabled = true;

    mMarioOffset += 3.0f;
    mMarioOffset = MR::clamp(mMarioOffset, 0.0f, ::sMarioJumpOffsetMax);

    updateMarioPose(3.0f);
}

void Tamakoro::endJumpHoleLaunch() {
    MR::onBind(this);
    MR::deleteEffect(this, "BigJumpBlur");
    MR::emitEffect(this, "BigJumpLand");
}

void Tamakoro::exeDashRail() {
    if (MR::isFirstStep(this)) {
        MR::startBckPlayer("BallRoll", nullptr);
        MR::startLevelSoundPlayer("JumpObj", -1, -1);
        MR::startActionSound(this, "SmIronsphRaildash", -1, -1, -1);
        MR::tryRumblePadStrong(this, WPAD_CHAN0);
        mAccelDir.zero();
        mAccelRate = 0.0f;
    }
    mControlDisabled = true;
    updateMarioPose(10.0f);
    updateMarioOffset();
    MR::setBckRatePlayer(mVelocity.length() / ::sDashBallRollSpeed);
}

void Tamakoro::exeDashRailEnd() {
    if (MR::isFirstStep(this)) {
        mMarioOffsetVelocity = 0.0f;
    }

    if (MR::isLessStep(this, ::sDashRailTime)) {
        MR::setBckRatePlayer(mVelocity.length() / ::sDashBallRollSpeed);
    }

    if (MR::isStep(this, ::sDashRailTime)) {
        MR::startBckPlayer("Fall", "BallFall");
    }

    mMarioOffset += 3.0f;
    mMarioOffset = MR::clamp(mMarioOffset, 0.0f, ::sMarioJumpOffsetMax);

    if (tryBumpWall() || tryLand()) {
        return;
    }

    updateVelocityNormal(::sDashRailFric);
    updateMarioPose(10.0f);
}

void Tamakoro::exeRideRail() {
    if (MR::isFirstStep(this)) {
        MR::startBckPlayerJ("タマコロ移動");
        MR::tryRumblePadMiddle(this, WPAD_CHAN0);
    }

    f32 speed = updateRideRail();
    updateMoveBckBlend(speed);
    if (speed > ::sRaidRailFastSpeed) {
        setNerve(GET_NERVE(Tamakoro, TamakoroNrvRideRailFastStart));
    }
}

void Tamakoro::exeRideRailFastStart() {
    if (MR::isFirstStep(this)) {
        MR::startBckPlayer("BallRollStart", nullptr);
        MR::setBckRatePlayer(::sRollBckRate);
    }
    updateRideRail();
    mControlDisabled = true;
    if (MR::isBckStoppedPlayer()) {
        setNerve(GET_NERVE(Tamakoro, TamakoroNrvRideRailFast));
    }
}

void Tamakoro::exeRideRailFast() {
    if (MR::isFirstStep(this)) {
        MR::startBckPlayer("BallRoll", nullptr);
        MR::setBckRatePlayer(::sRollBckRate);
    }
    mControlDisabled = true;
    f32 speed = updateRideRail();

    MR::setBckRatePlayer(mVelocity.length() / ::sDashBallRollSpeed);

    if (speed < ::sRaidRailFastSpeed) {
        setNerve(GET_NERVE(Tamakoro, TamakoroNrvRideRailFastEnd));
    }
}

void Tamakoro::exeRideRailFastEnd() {
    if (MR::isFirstStep(this)) {
        MR::startBckPlayer("BallRollEnd", nullptr);
        MR::setBckRatePlayer(::sRollBckRate);
    }
    updateRideRail();
    if (MR::isBckStoppedPlayer()) {
        setNerve(GET_NERVE(Tamakoro, TamakoroNrvRideRail));
    }
}

void Tamakoro::exeBindEnd() {
    if (MR::isFirstStep(this)) {
        MR::tryRumblePadStrong(this, WPAD_CHAN0);
        MR::shakeCameraStrong();
        mAccelDir.zero();
        mAccelRate = 0.0f;
        if (MR::isValidSwitchA(this)) {
            MR::offSwitchA(this);
        }
        MR::emitEffect(this, "Break");
        MR::startActionSound(this, "SmRideEnd", -1, -1, -1);
        MR::startSoundPlayer("SE_PV_JUMP_S", -1, -1);
        MR::stopStageBGM(10);
        if (mPurpleCoin != nullptr) {
            TVec3f vel(mGravity);
            vel.x *= -30.0f;
            vel.y *= -30.0f;
            vel.z *= -30.0f;
            mPurpleCoin->appearMove(mPosition, vel, -1, -1);
        }
    }

    if ((MR::isGreaterStep(this, 12) && MR::isOnGroundPlayer()) || MR::isGreaterStep(this, ::sForceBindEndTime)) {
        MR::startLastStageBGM();
        if (!mIsPurple) {
            MR::requestAppearPowerStar(this, mPosition);
        }
        kill();
    }
}

void Tamakoro::addVelocityOperate() {
    mAccelRate = mAccelSensorCtrl->calcMoveVector(&mAccelDir, mGravity);

    TVec3f* vel = &mVelocity;  // this needs to be like this to match...
    mVelocity.scaleAdd(mAccelRate * (MR::isBindedGround(this) ? 0.4f : 0.2f), mAccelDir, *vel);
}

f32 Tamakoro::updateRideRail() {
    mAccelDir.set(mVelocity);

    if (MR::normalizeOrZero(&mAccelDir)) {
        mAccelRate = 0.5f;
    } else {
        mAccelRate = 0.0f;
    }

    f32 speed = mMoment.length() * mBaseRadius;

    updateMarioPose(15.0f);
    updateMarioOffset();

    return speed;
}

void Tamakoro::updateVelocityNormal(f32 friction) {
    TVec3f vel(mVelocity);  // unused! woo!
    MR::reboundVelocityFromCollision(this, 0.0f, 0.0f, 1.0f);
    MR::addVelocityToGravity(this, ::sGravityAccel);
    MR::attenuateVelocity(this, MR::isBindedGround(this) ? friction : 0.995f);
}

void Tamakoro::updateMoment() {
    if (isGroundMomentBall()) {
        TVec3f moment;
        const TVec3f* up;
        // TVec3f tmpVec;
#ifdef GALAXY_NATIVE
        // up is read after the block (calcMomentRollBall), so the vector must
        // outlive the block; Clang reuses block-local stack slots.
        TVec3f tmpVec;
        if (MR::isBindedGround(this)) {
            up = &MR::getGroundNormal(this);
        } else {
            tmpVec.set(-mGravity);
            up = &tmpVec;
        }
#else
        if (MR::isBindedGround(this)) {
            up = &MR::getGroundNormal(this);
        } else {
            TVec3f tmpVec(-mGravity);
            up = &tmpVec;
        }
#endif

        MR::calcMomentRollBall(&moment, mVelocity, *up, mBaseRadius);
        MR::vecBlend(&mMoment, mMoment, moment, 0.25f);
        startRotateLevelSound();
    } else {
        mMoment.mult(0.98f);
    }
}

void Tamakoro::updateAirTime() {
    if (MR::isBindedGround(this)) {
        mAirTime = 0;
    } else {
        mAirTime++;
    }
}

void Tamakoro::updateMarioPose(f32 degree) {
    TVec3f velH;
    const TVec3f& vel = mVelocity;
    const TVec3f& grav = mGravity;
    velH.scaleAdd(-grav.dot(vel), grav, vel);

    if (MR::isNearZero(velH)) {
        if (mAccelRate > 0.001f) {
            MR::turnDirectionDegree(this, &mMarioFront, mAccelDir, degree * mAccelRate);
        }
    } else {
        MR::turnDirectionDegree(this, &mMarioFront, velH, degree);
    }

    TVec3f zDir;
    TVec3f xDir;
    mMarioRotateQuat.getXDir(xDir);
    mMarioRotateQuat.getZDir(zDir);

    if (mAccelRate > 0.0f) {
        f32 dot = MR::normalize(1.0f - mAccelDir.dot(zDir), 0.1f, 1.0f);
        f32 sign;
        if (mAccelDir.dot(xDir) >= 0.0f) {
            sign = 1.0f;
        } else {
            sign = -1.0f;
        }
        dot = sign * dot;
        mMarioRotateYAngle += dot * ::sMarioUpperYAccel;
    }
    mMarioRotateYAngle *= ::sMarioUpperYFric;

    TVec3f up;
    up.set(-mGravity);
    mMarioUp.set(-mGravity);

    TVec3f rotAxis = mAccelDir.cross(mGravity);

    if (!MR::normalizeOrZero(&rotAxis)) {
        MR::rotateVecDegree(&up, rotAxis, mAccelRate * 35.0f);
        MR::rotateVecDegree(&mMarioUp, rotAxis, mAccelRate * 20.0f);
    }
    MR::vecBlendSphere(&mDirectionToMario, mDirectionToMario, up, ::sStandDirInterRate);

    if (MR::normalizeOrZero(&mDirectionToMario)) {
        mDirectionToMario.set(-mGravity);
    }
    MR::blendQuatUpFront(&mMarioRotateQuat, mMarioUp, mMarioFront, ::sMarioUpAdjustRate, ::sMarioFrontAdjustRate);

    f32 rotY = mMarioRotateYAngle;
    MR::setPlayerUpperRotateY(PI_180 * rotY);
}

// NONMATCHING: floating-point register allocation differs.
void Tamakoro::updateMarioOffset() {
    f32 velocity = mMarioOffsetVelocity - ::sMarioOffsetGravityAccel;
    velocity *= ::sMarioOffsetFric;

    mMarioOffsetVelocity = velocity;
    mMarioOffset += mMarioOffsetVelocity;
    mMarioOffset = MR::clamp(mMarioOffset, 0.0f, ::sMarioJumpOffsetMax);
}

void Tamakoro::updateMoveBckBlend(f32 pSpeed) {
    f32 idleBckWeight = 0.0f;
    f32 softWalkBckWeight = 0.0f;
    f32 walkBckWeight = 0.0f;
    f32 runBckWeight = 0.0f;
    f32 rate = 0.0f;
    f32 softWalkSpeed = ::sSoftWalkSpeed;
    f32 walkSpeed = ::sWalkSpeed;
    f32 runSpeed = ::sRunSpeed;

    if (pSpeed < softWalkSpeed) {
        softWalkBckWeight = MR::normalize(pSpeed, 0.0f, softWalkSpeed);
        idleBckWeight = 1.0f - softWalkBckWeight;
        rate = MR::getLinerValue(softWalkBckWeight, ::sIdleBckRate, ::sSoftWalkBckRate, 1.0f);
    } else if (pSpeed < walkSpeed) {
        walkBckWeight = MR::normalize(pSpeed, softWalkSpeed, walkSpeed);
        softWalkBckWeight = 1.0f - walkBckWeight;
        rate = MR::getLinerValue(walkBckWeight, ::sSoftWalkBckRate, ::sWalkBckRate, 1.0f);
    } else if (pSpeed < runSpeed) {
        runBckWeight = MR::normalize(pSpeed, walkSpeed, runSpeed);
        walkBckWeight = 1.0f - runBckWeight;
        rate = MR::getLinerValue(runBckWeight, ::sWalkBckRate, ::sRunBckRate, 1.0f);
    } else {
        runBckWeight = 1.0f;
        rate = (pSpeed / runSpeed) * ::sRunBckRate;
        if (rate > ::sRunMaxBckRate) {
            rate = ::sRunMaxBckRate;
        }
    }

    MR::setBckBlendWeight(idleBckWeight, softWalkBckWeight, walkBckWeight, runBckWeight);
    MR::setBckRatePlayer(rate);
}

void Tamakoro::updateSquatBckBlend(f32 pSpeed) {
    f32 limit2 = 6.0f;
    f32 w1 = MR::normalize(pSpeed, 0.0f, limit2);
    f32 w0 = 1.0f - w1;
    f32 rate = MR::getLinerValue(w1, 1.0f, 2.0f, 1.0f);

    MR::setBckBlendWeight(w0, w1);
    MR::setBckRatePlayer(rate);
}

bool Tamakoro::isEnablePushPlayer() const {
    if (isNerve(GET_NERVE(Tamakoro, TamakoroNrvStandByTutorial)) || isNerve(GET_NERVE(Tamakoro, TamakoroNrvStandByBind))) {
        return true;
    }
    return false;
}

bool Tamakoro::isEnableEnemyAttack() const {
    if (isNerve(GET_NERVE(Tamakoro, TamakoroNrvWait)) || isNerve(GET_NERVE(Tamakoro, TamakoroNrvFall)) ||
        isNerve(GET_NERVE(Tamakoro, TamakoroNrvJump)) || isNerve(GET_NERVE(Tamakoro, TamakoroNrvBumpWall)) ||
        isNerve(GET_NERVE(Tamakoro, TamakoroNrvDashRailEnd)) || isNerve(GET_NERVE(Tamakoro, TamakoroNrvFreeze)) ||
        isNerve(GET_NERVE(Tamakoro, TamakoroNrvLand))) {
        return true;
    }
    return false;
}

bool Tamakoro::isEnableBallBind() const {
    if (isNerve(GET_NERVE(Tamakoro, TamakoroNrvWait)) || isNerve(GET_NERVE(Tamakoro, TamakoroNrvFall)) ||
        isNerve(GET_NERVE(Tamakoro, TamakoroNrvJump)) || isNerve(GET_NERVE(Tamakoro, TamakoroNrvBumpWall)) ||
        isNerve(GET_NERVE(Tamakoro, TamakoroNrvDashRailEnd)) || isNerve(GET_NERVE(Tamakoro, TamakoroNrvLand))) {
        return true;
    }
    return false;
}

bool Tamakoro::isEnebleHitCollisionMessage() const {
    return isNerve(GET_NERVE(Tamakoro, TamakoroNrvJump));
}

bool Tamakoro::isUseMarioOffset() const {
    if (isNerve(GET_NERVE(Tamakoro, TamakoroNrvBindStart)) || isNerve(GET_NERVE(Tamakoro, TamakoroNrvBindStartLand))) {
        return false;
    }
    return true;
}

bool Tamakoro::isGroundMomentBall() const {
    if (MR::isBindedGround(this) || isRideRail() || isNerve(GET_NERVE(Tamakoro, TamakoroNrvDashRail)) ||
        isNerve(GET_NERVE(Tamakoro, TamakoroNrvJumpHole))) {
        return true;
    }
    return false;
}

bool Tamakoro::isRideRail() const {
    if (isNerve(GET_NERVE(Tamakoro, TamakoroNrvRideRail)) || isNerve(GET_NERVE(Tamakoro, TamakoroNrvRideRailFastStart)) ||
        isNerve(GET_NERVE(Tamakoro, TamakoroNrvRideRailFast)) || isNerve(GET_NERVE(Tamakoro, TamakoroNrvRideRailFastEnd))) {
        return true;
    }
    return false;
}

bool Tamakoro::isNeedTutorial() const {
    if (mTutorial == nullptr) {
        return false;
    }
    return mTutorial->isNeedTutorial();
}

void Tamakoro::startRotateLevelSound() {
    f32 mag = mMoment.length();

    if (mag <= 0.001f) {
        return;
    }

    s32 lvl = ((mag * 10000.0f) / TWO_PI);
    if (isNerve(GET_NERVE(Tamakoro, TamakoroNrvRideRail))) {
        MR::startActionSound(this, "SmLvIronsphRollRail", lvl, -1, -1);
    } else {
        MR::startActionSound(this, "SmLvIronsphRoll1", lvl, -1, -1);
    }
}

void Tamakoro::startTamakoroBgm() {
    const char* pName = MR::isEqualStageName("TamakoroSliderGalaxy") ? "BGM_TAMAKORO_2" : "BGM_TAMAKORO";
    MR::startStageBGM(pName, false);
    mBgmCtrl->init(pName);
}

namespace MR {
    NameObj* createTamakoroWithTutorial(const char* pName) {
        Tamakoro* tamakoro = new Tamakoro(pName);
        tamakoro->setTutorial();
        return tamakoro;
    }
};  // namespace MR
