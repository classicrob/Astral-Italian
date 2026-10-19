#include "Game/Util/MessageUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#pragma auto_inline off
#include "Game/Ride/RollingRock.hpp"
#include "Game/Camera/CameraDirector.hpp"
#include "Game/Camera/CameraHolder.hpp"
#include "Game/Camera/CameraTargetArg.hpp"
#include "Game/Camera/CameraTargetMtx.hpp"
#include "Game/LiveActor/Binder.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/ModelObj.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include "Game/Util/ActorStateUtil.hpp"
#include "Game/Util/NerveUtil.hpp"
#include <JSystem/JMath.hpp>
#include <cmath>

#define sNameRollState "\x8a\xe2\x83}\x83\x8a\x83I\x83S\x83\x8d\x83S\x83\x8d\x8f\xf3\x91\xd4"
#define sNameCameraTarget "\x83J\x83\x81\x83\x89\x83^\x81[\x83Q\x83""b\x83g(Rock)"
#define sRumblePushReflect "\x89\x9f\x82\xb3\x82\xea\x94\xbd\x8e\xcb"
#define sNameCage1 "\x82\xb2\x82\xeb\x8a\xe2\x94j\x89\xf3\x83P\x81[\x83W"
#define sNameCage2 "\x83\x8d\x83""b\x83N\x83P\x81[\x83W"
#define sRumbleBoardReflect "\x94\xbd\x8e\xcb\x94\xc2\x94\xbd\x8e\xcb"
#define sRumbleLand "\x92\x85\x92n"
#define sRumbleRollEnd "\x83S\x83\x8d\x83S\x83\x8d\x8fI\x97\xb9"
#define sRumbleRollCancel "\x83S\x83\x8d\x83S\x83\x8d\x83L\x83\x83\x83\x93\x83Z\x83\x8b"
#define sBckRollJump "\x83S\x83\x8d\x8a\xe2\x89\xf0\x8f\x9c\x83W\x83\x83\x83\x93\x83v"
#define sBckRollHitJump "\x83S\x83\x8d\x8a\xe2\x8f\xd5\x93\xcb\x89\xf0\x8f\x9c\x83W\x83\x83\x83\x93\x83v"
#define sRumbleRollStart "\x83S\x83\x8d\x83S\x83\x8d\x8aJ\x8en"
#define sRumbleAccel "\x89\xc1\x91\xac"
#define sRumbleForceDash "\x8b\xad\x90\xa7\x83_\x83""b\x83V\x83\x85"
#define sRumbleJump "\x83W\x83\x83\x83\x93\x83v"
namespace {
    extern const f32 sJumpGravityRate;
    extern const f32 sReflectSideRate;
    static const f32 sBinderRadiusMin = 40.0f;
    static const f32 sBinderRadiusMax = 106.0f;
    static const s32 sVolumeScale = 450;
    static const s32 sPitchScale = 29;
    static const s32 sPitchBase = -200;
};  // namespace

namespace NrvRollingRockStateRoll {
    class RollingRockStateRollNrvWait : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {}
        static RollingRockStateRollNrvWait sInstance;
    };
    RollingRockStateRollNrvWait RollingRockStateRollNrvWait::sInstance ATTRIBUTE_WEAK;
    NEW_NERVE(RollingRockStateRollNrvStart, RollingRockStateRoll, Start);
    NEW_NERVE(RollingRockStateRollNrvAppear, RollingRockStateRoll, Appear);
    NEW_NERVE(RollingRockStateRollNrvMove, RollingRockStateRoll, Move);
    NEW_NERVE(RollingRockStateRollNrvAccel, RollingRockStateRoll, Accel);
    NEW_NERVE(RollingRockStateRollNrvForceDash, RollingRockStateRoll, ForceDash);
    NEW_NERVE(RollingRockStateRollNrvJump, RollingRockStateRoll, Jump);
};  // namespace NrvRollingRockStateRoll

RollingRockStateRoll::RollingRockStateRoll(RollingRock* pHost)
    : RollingRockStateBase(sNameRollState, pHost) {
    mCameraTarget = new CameraTargetMtx(sNameCameraTarget);
    mDir.set(0.0f, 0.0f, 1.0f);
    mDashStepMax = 180;
    mDashStep = 0;
    mAirStep = 0;
    mTurnRate = 3.0f;
    mStep = 0;
    _34 = 0.0f;
    mSpeed = 0.0f;
    mJumpBuffer = 0;
    mStickRate = 0.0f;
    mIsFirstRoll = false;
    initNerve(&NrvRollingRockStateRoll::RollingRockStateRollNrvWait::sInstance, 0);
}

RollingRockStateRoll::~RollingRockStateRoll() {
}

void RollingRockStateRoll::appear() {
    mIsDead = false;
    mStep = 0;
    mDashStep = 0;
    mDashStepMax = 180;
    mSpeed = 0.0f;
    mHost->appearBlur();
    mHost->appearBreakStart();
    MR::setPlayerRollingOn();
    if (MR::isValidSwitchA(mHost)) {
        MR::onSwitchA(mHost);
    }
    setNerve(&NrvRollingRockStateRoll::RollingRockStateRollNrvStart::sInstance);
}

void RollingRockStateRoll::kill() {
    if (!mIsDead) {
        mHost->killBlur();
        MR::deleteEffect(mHost, "Move");
        MR::forceDeleteEffect(mHost, "MoveBlur");
        mHost->appearBreakEnd();
        MR::setPlayerRollingOff();
        MR::offBind(mHost);
        MR::invalidateHitSensors(mHost);
        if (MR::isValidSwitchA(mHost)) {
            MR::offSwitchA(mHost);
        }
        mIsDead = true;
    }
}

void RollingRockStateRoll::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isEqualSensor(pSender, mHost, "Attack") && isAttackEnable() && MR::sendMsgRockAttack(pReceiver, pSender)) {
        mHost->onHitEnemy(pSender, pReceiver);
    }
}

bool RollingRockStateRoll::receiveMsgPush(HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(&NrvRollingRockStateRoll::RollingRockStateRollNrvStart::sInstance) ||
        isNerve(&NrvRollingRockStateRoll::RollingRockStateRollNrvAppear::sInstance)) {
        if (MR::isSensorNpc(pSender)) {
            MR::addVelocityFromPush(mHost, 0.1f, pSender, pReceiver);
            return true;
        }
    }
    if (!isAttackEnable()) {
        return false;
    }
    if (!RollingRock::isBreakableSensor(pSender)) {
        return false;
    }
    if (!MR::isEqualSensor(pReceiver, mHost, "Body")) {
        return false;
    }
    if (MR::fn_80012BA0(mHost, pReceiver, pSender, 0.7f)) {
        if (isNerve(&NrvRollingRockStateRoll::RollingRockStateRollNrvMove::sInstance) ||
            isNerve(&NrvRollingRockStateRoll::RollingRockStateRollNrvAccel::sInstance) ||
            isNerve(&NrvRollingRockStateRoll::RollingRockStateRollNrvAccel::sInstance) ||
            isNerve(&NrvRollingRockStateRoll::RollingRockStateRollNrvJump::sInstance)) {
            MR::tryHitReaction(mHost, sRumblePushReflect);
        }
        return true;
    }
    return false;
}

// NONMATCHING: vector temporary placement and boolean branch layout differ.
bool RollingRockStateRoll::receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (msg == 0x62 && RollingRock::isNotBreakableSensor(pSender) && MR::isEqualSensor(pReceiver, mHost, "Body")) {
        if (!mIsDead) {
            if (MR::isMsgToEnemyAttackBlow(msg) && MR::isSensorEnemy(pSender)) {
                mHost->endBindWait();
                changeState(3);
                return false;
            }
            TVec3f neg;
            neg.negate(mHost->mGravity);
            TVec3f dir(neg);
            dir.scale(10.0f);
            if (MR::reflectVelocityH(mHost, pReceiver, pSender, 1.0f)) {
                dir.set(mHost->mVelocity);
                TVec3f scaled(dir);
                scaled.scale(1.0f);
                dir = scaled;
            }
            mHost->endBindDamage(msg, dir);
            changeState(3);
            return true;
        }
        return false;
    }
    if (isNerve(&NrvRollingRockStateRoll::RollingRockStateRollNrvStart::sInstance)) {
        if (MR::isEqualSensor(pReceiver, mHost, "Body")) {
            if (MR::isMsgToEnemyAttackBlow(msg) && MR::isSensorEnemy(pSender)) {
                TVec3f neg;
                neg.negate(mHost->mGravity);
                TVec3f dir(neg);
                dir.scale(10.0f);
                if (MR::reflectVelocityH(mHost, pReceiver, pSender, 1.0f)) {
                    dir.set(mHost->mVelocity);
                    TVec3f scaled(dir);
                    scaled.scale(1.0f);
                    dir = scaled;
                }
                mHost->endBindFlip(msg, dir);
                changeState(3);
                return false;
            }
            TVec3f zero;
            zero.setAll< f32 >(0.0f);
            mHost->endBindDamage(msg, zero);
            changeState(2);
            return true;
        }
    }
    return false;
}

bool RollingRockStateRoll::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgUpdateBaseMtx(msg)) {
        setPlayerPose();
        return true;
    }
    if (MR::isMsgIsRushEnable(msg)) {
        changeState(5);
        return true;
    }
    return false;
}

void RollingRockStateRoll::initFront() {
    MR::getPlayerFrontVec(&mDir);
    MR::vecKillElement(mDir, mHost->mGravity, &mDir);
    MR::normalize(&mDir);
}

void RollingRockStateRoll::control() {
    updateStep();
    updateVelocity();
    updateMoment();
    updatePose();
    updateEffect();
    updatePoseNoMoment();
    if (mJumpBuffer > 0) {
        mJumpBuffer--;
    }
    tryDeath();
    if (!MR::isNearZero(mHost->mVelocity, 0.001f)) {
        mHost->mFront.set(mHost->mVelocity);
        MR::normalize(&mHost->mFront);
    }
    if (!isNerve(&NrvRollingRockStateRoll::RollingRockStateRollNrvWait::sInstance) &&
        !isNerve(&NrvRollingRockStateRoll::RollingRockStateRollNrvStart::sInstance) &&
        !isNerve(&NrvRollingRockStateRoll::RollingRockStateRollNrvAppear::sInstance) &&
        !isNerve(&NrvRollingRockStateRoll::RollingRockStateRollNrvJump::sInstance)) {
        if (MR::getBrkRate(mHost) <= 0.0f && MR::isBindedGroundSnow(mHost)) {
            MR::setBrkRate(mHost, 1.0f);
        }
    }
}

bool RollingRockStateRoll::tryEnd() {
    if (mHost->mArg7 == 1) {
        return false;
    }
    if (!isNerve(&NrvRollingRockStateRoll::RollingRockStateRollNrvJump::sInstance)) {
        return false;
    }
    if (!MR::testSubPadTriggerZ(0)) {
        return false;
    }
    changeState(4);
    return true;
}

bool RollingRockStateRoll::tryJump() {
    bool trig = MR::testCorePadTriggerA(0);
    bool air = mAirStep < 5;
    if (mJumpBuffer > 0 || (air && trig)) {
        setNerve(&NrvRollingRockStateRoll::RollingRockStateRollNrvJump::sInstance);
        return true;
    }
    return false;
}

bool RollingRockStateRoll::tryDeath() {
    TVec3f zero(0.0f, 0.0f, 0.0f);
    if (MR::isInDeath(mHost, zero) || mAirStep > 600) {
        MR::forceKillPlayerByAbyss();
        return true;
    }
    return false;
}

// NONMATCHING: boolean result register and branch layout differ.
static bool isBreakCageSensor(const HitSensor* pSensor) {
    if (pSensor == nullptr) {
        return false;
    }
    bool ret = true;
    if (!MR::isEqualString(MR::getSensorHost(pSensor)->mName, sNameCage1) && !MR::isEqualString(MR::getSensorHost(pSensor)->mName, sNameCage2)) {
        ret = false;
    }
    return ret;
}

// NONMATCHING: host and sensor registers are exchanged.
bool RollingRockStateRoll::tryReflect() {
    if (MR::isBindedWall(mHost)) {
        LiveActor* pHost = mHost;
        HitSensor* pAttack = pHost->getSensor("Attack");
        if (MR::sendMsgReflect(MR::getWallSensor(pHost), pAttack)) {
            MR::tryHitReaction(mHost, sRumbleBoardReflect);
            mDashStep = 0;
            f32 speed;
            if (MR::isNearZero(mSpeed, 0.001f)) {
                speed = 42.0f;
            } else {
                speed = mSpeed;
            }
            MR::restrictVelocity(mHost, speed);
            return true;
        }
    }
    LiveActor* pHost = mHost;
    bool hit;
    if (!MR::isBinded(pHost)) {
        hit = false;
    } else if (MR::isBindedWall(pHost) && isBreakCageSensor(MR::getWallSensor(pHost))) {
        hit = true;
    } else if (MR::isBindedGround(pHost) && isBreakCageSensor(MR::getGroundSensor(pHost))) {
        hit = true;
    } else if (MR::isBindedRoof(pHost) && isBreakCageSensor(MR::getRoofSensor(pHost))) {
        hit = true;
    } else {
        hit = false;
    }
    if (hit) {
        if (!(MR::isBindedWall(mHost) && MR::sendMsgToWallSensor(0x11, mHost->getSensor("Attack"))) &&
            !(MR::isBindedGround(mHost) && MR::sendMsgToGroundSensor(0x11, mHost->getSensor("Attack"))) && MR::isBindedRoof(mHost)) {
            MR::sendMsgToRoofSensor(0x11, mHost->getSensor("Attack"));
        }
        changeState(1);
        return true;
    }
    if (!MR::isBindedWall(mHost)) {
        return false;
    }
    MR::sendMsgToWallSensor(0x11, mHost->getSensor("Attack"));
    changeState(1);
    return true;
}

bool RollingRockStateRoll::tryLand() {
    if (MR::isLessStep(this, 15)) {
        return false;
    }
    if (!MR::isBindedGround(mHost)) {
        return false;
    }
    MR::tryHitReaction(mHost, sRumbleLand);
    setNerve(&NrvRollingRockStateRoll::RollingRockStateRollNrvMove::sInstance);
    return true;
}

void RollingRockStateRoll::updateVelocity() {
    f32 gravityAccel = 1.8f;
    if (isNerve(&NrvRollingRockStateRoll::RollingRockStateRollNrvMove::sInstance) && !MR::isBindedGround(mHost)) {
        MR::attenuateVelocityExceptDirection(mHost, mHost->mGravity, 1.0f);
        gravityAccel = 2.0f;
    }
    if (!isNerve(&NrvRollingRockStateRoll::RollingRockStateRollNrvAppear::sInstance)) {
        if (MR::isBindedGround(mHost)) {
            if (mAirStep > 5) {
                TVec3f proj;
                proj.setAll< f32 >(0.0f);
                TVec3f dir(mHost->mGravity);
                MR::normalize(&dir);
                proj.scale(dir.dot(mHost->mVelocity), dir);
                MR::startActionSound(mHost, "Land", (s32)(1000.0f * MR::abs(PSVECMag(proj))), -1, -1);
            }
            mAirStep = 0;
        } else {
            MR::addVelocityToGravity(mHost, gravityAccel);
            mAirStep++;
        }
    }
    f32 speed = PSVECMag(mHost->mVelocity);
    TVec3f before(mHost->mVelocity);
    MR::reboundVelocityFromCollision(mHost, 0.0f, 0.0f, 1.0f);
    TVec3f after(mHost->mVelocity);
    TVec3f diff = after - before;
    mHost->mVelocity.setLength(speed);
    if (!isNerve(&NrvRollingRockStateRoll::RollingRockStateRollNrvForceDash::sInstance)) {
        MR::restrictVelocity(mHost, 65.0f);
    }
}

void RollingRockStateRoll::updatePoseNoMoment() {
    if (MR::isSameDirection(mHost->mGravity, mDir, 0.01f)) {
        TPos3f* matrix = &mCameraTarget->mMatrix;
        TVec3f up;
        up.negate(mHost->mGravity);
        MR::makeMtxUpNoSupportPos(matrix, up, mHost->mPosition);
    } else {
        TPos3f* matrix = &mCameraTarget->mMatrix;
        TVec3f up;
        up.negate(mHost->mGravity);
        MR::makeMtxUpFrontPos(matrix, up, mDir, mHost->mPosition);
    }
}

void RollingRockStateRoll::setPlayerPose() {
    MR::setPlayerQuatAndPos(&mHost->mQuat, &mHost->mPosition);
}

void RollingRockStateRoll::updateStep() {
    mStep++;
    if (isNerve(&NrvRollingRockStateRoll::RollingRockStateRollNrvMove::sInstance) ||
        isNerve(&NrvRollingRockStateRoll::RollingRockStateRollNrvAccel::sInstance) ||
        isNerve(&NrvRollingRockStateRoll::RollingRockStateRollNrvJump::sInstance) ||
        (isNerve(&NrvRollingRockStateRoll::RollingRockStateRollNrvForceDash::sInstance) && mDashStepMax != 180)) {
        mDashStep++;
    }
}

// NONMATCHING: vector temporary stack slots differ.
void RollingRockStateRoll::updateMoment() {
    TVec3f moment;
    moment.setAll< f32 >(0.0f);
    if (isNerve(&NrvRollingRockStateRoll::RollingRockStateRollNrvAppear::sInstance)) {
        TVec3f vel(mDir);
        vel.scale(48.0f);
        TVec3f up;
        up.negate(mHost->mGravity);
        MR::calcMomentRollBall(&moment, vel, up, MR::getSensorRadius(mHost, "Body"));
    } else if (&NrvRollingRockStateRoll::RollingRockStateRollNrvMove::sInstance) {
        TVec3f up;
        up.negate(mHost->mGravity);
        if (MR::isBindedGround(mHost)) {
            up.set(MR::getGroundNormal(mHost));
        }
        MR::calcMomentRollBall(&moment, mHost->mVelocity.scaleInline(1.2f), up, MR::getSensorRadius(mHost, "Body"));
    } else {
        return;
    }
    MR::rotateQuatMoment(&mHost->mQuat, moment);
}

void RollingRockStateRoll::updatePose() {
    TVec3f front;
    if (isNerve(&NrvRollingRockStateRoll::RollingRockStateRollNrvStart::sInstance) ||
        isNerve(&NrvRollingRockStateRoll::RollingRockStateRollNrvAppear::sInstance)) {
        front.set(mDir);
        MR::vecKillElement(front, mHost->mGravity, &front);
    } else {
        if (!MR::isNearZero(mHost->mVelocity, 0.001f)) {
            front.set(mHost->mVelocity);
        } else {
            mHost->mEffectMtx.getZDir(front);
        }
        MR::vecKillElement(front, mHost->mGravity, &front);
    }
    if (MR::normalizeOrZero(&front)) {
        mHost->mQuat.getZDir(front);
    }
    if (MR::isSameDirection(front, mHost->mGravity, 0.01f)) {
        RollingRock* pHost = mHost;
        TVec3f up;
        up.negate(mHost->mGravity);
        MR::makeMtxUpNoSupportPos(&pHost->mEffectMtx, up, mHost->mPosition);
    } else {
        RollingRock* pHost = mHost;
        TVec3f up;
        up.negate(pHost->mGravity);
        MR::makeMtxUpFrontPos(&pHost->mEffectMtx, up, front, mHost->mPosition);
    }
}

// NONMATCHING: equivalent early-return branch layout differs.
void RollingRockStateRoll::updateEffect() {
    if (!isNerve(&NrvRollingRockStateRoll::RollingRockStateRollNrvStart::sInstance)) {
        if (isNerve(&NrvRollingRockStateRoll::RollingRockStateRollNrvAppear::sInstance)) {
            return;
        } else {
            if (mAirStep > 5) {
                if (MR::isEffectValid(mHost, "Move")) {
                    MR::deleteEffect(mHost, "Move");
                }
            } else if (!MR::isEffectValid(mHost, "Move")) {
                MR::emitEffect(mHost, "Move");
            }
        }
    }
}

bool RollingRockStateRoll::isAttackEnable() const {
    if (mIsDead) {
        return false;
    }
    if (isNerve(&NrvRollingRockStateRoll::RollingRockStateRollNrvWait::sInstance)) {
        return false;
    }
    return !isNerve(&NrvRollingRockStateRoll::RollingRockStateRollNrvStart::sInstance);
}

void RollingRockStateRoll::startMove() {
    TVec3f vel(mDir);
    vel.scale(36.0f);
    MR::setVelocity(mHost, vel);
    MR::emitEffect(mHost, "Move");
    MR::emitEffect(mHost, "MoveBlur");
    setNerve(&NrvRollingRockStateRoll::RollingRockStateRollNrvMove::sInstance);
}

// NONMATCHING: vector temporary placement and register allocation differ.
void RollingRockStateRoll::changeState(s32 type) {
    RollingRock* pHost = mHost;
    TVec3f* pVel = &pHost->mVelocity;
    switch (type) {
    case 0: {
        TVec3f front(pHost->mFront);
        TVec3f pos(pHost->mPosition);
        pVel->set(front.scaleInline(1.0f - sJumpGravityRate) - pHost->mGravity.scaleInline(sJumpGravityRate));
        pVel->setLength(15.0f);
        break;
    }
    case 1: {
        pVel->set((-pHost->mGravity).scaleInline(18.0f));
        break;
    }
    case 5:
    case 3:
        if (MR::isNearZero(pHost->mVelocity, 0.001f)) {
            pVel->set((-pHost->mGravity).scaleInline(18.0f));
        } else {
            TVec3f up;
            up.negate(pHost->mGravity);
            TVec3f side;
            MR::vecKillElement(pHost->mVelocity, up, &side);
            MR::normalizeOrZero(&side);
            side.negate();
            pVel->set((side.scaleInline(sReflectSideRate) + up.scaleInline(1.0f - sReflectSideRate)).scaleInline(18.0f));
        }
        break;
    case 4:
        pVel->setAll< f32 >(0.0f);
        break;
    }
    switch (type) {
    case 2:
        MR::tryHitReaction(mHost, sRumbleRollEnd);
        mHost->hideRock();
        break;
    case 3:
        MR::tryHitReaction(mHost, sRumbleRollEnd);
        break;
    case 0:
    case 4:
        MR::tryHitReaction(mHost, sRumbleRollCancel);
        MR::startBckPlayerJ(sBckRollJump);
        break;
    case 1:
    case 5:
        MR::tryHitReaction(mHost, sRumbleRollEnd);
        MR::shakeCameraStrong();
        MR::tryRumblePadMiddle(mHost, 0);
        MR::startBckPlayerJ(sBckRollHitJump);
        break;
    default:
        MR::tryHitReaction(mHost, sRumbleRollEnd);
        break;
    }
    kill();
}

// NONMATCHING: branch layout and register scheduling differ.
void RollingRockStateRoll::exeStart() {
    if (MR::isFirstStep(this)) {
        MR::zeroVelocity(mHost);
        MR::onCalcGravity(mHost);
        MR::onBind(mHost);
        MR::addVelocityJump(mHost, 20.0f);
        MR::startBckPlayer("RollingRockStart", nullptr);
        MR::startLevelSoundPlayer("RockRollStart", -1, -1);
        MR::tryHitReaction(mHost, sRumbleRollStart);
        CameraTargetArg arg(mCameraTarget);
        MR::setGameCameraTarget(arg);
        mIsFirstRoll = true;
        MR::setBinderRadius(mHost, sBinderRadiusMin);
        return;
    }
    LiveActor* pHost = mHost;
    MR::setBinderRadius(pHost, MR::calcNerveValue(this, 20, sBinderRadiusMin, sBinderRadiusMax));
    turnByStickStart();
    if (MR::isBindedGround(mHost) && MR::isGreaterStep(this, 5)) {
        f32 binderRadiusMin = sBinderRadiusMin;
        if (MR::isLessEqualStep(this, (s32)binderRadiusMin)) {
            MR::setBinderRadius(mHost, sBinderRadiusMax);
        }
        setNerve(&NrvRollingRockStateRoll::RollingRockStateRollNrvAppear::sInstance);
    }
}

void RollingRockStateRoll::exeAppear() {
    if (MR::isFirstStep(this)) {
        MR::zeroVelocity(mHost);
        MR::tryHitReaction(mHost, sRumbleLand);
        mHost->showRock();
        MR::startBrk(mHost, "Snow");
        MR::setBrkRate(mHost, 0.0f);
    }
    turnByStickAppear();
    if (MR::isGreaterStep(this, 30)) {
        startMove();
    }
}

// NONMATCHING: integer conversion spills and floating-point operand order differ.
void RollingRockStateRoll::exeMove() {
    TVec3f prevVel(mHost->mVelocity);
    TVec3f newVel;
    newVel.z = newVel.y = newVel.x = 0.0f;
    if (turnByStick()) {
        newVel = mHost->mVelocity;
        MR::diffAngleAbs(newVel, prevVel);
    }
    if (!tryJump() && !tryReflect() && !tryForceDash() && !tryTimeOver()) {
        s32 volScale = sVolumeScale;
        s32 base = (s32)MR::clamp(volScale * (mStickRate - 0.1f), 0.0f, 1000.0f);
        s32 pitchBase = sPitchBase;
        s32 pitchScale = sPitchScale;
        s32 pitch = (s32)MR::clamp(pitchBase + pitchScale * PSVECMag(mHost->mVelocity), 0.0f, 1000.0f);
        s32 volume = (base + (s32)MR::clamp(pitchBase + pitchScale * PSVECMag(mHost->mVelocity), 0.0f, 1000.0f)) - 200;
        if (mAirStep > 5) {
            volume += 100;
            pitch = (s32)(pitch * 0.9f);
        }
        if (MR::isLessStep(this, 120)) {
            MR::startActionSound(mHost, "Move", pitch, volume, -1);
        } else {
            MR::startActionSound(mHost, "MoveEnding", pitch, volume, -1);
        }
        if (MR::isFirstStep(this)) {
            MR::startActionSound(mHost, "MoveStart", -1, -1, -1);
            if (mIsFirstRoll) {
                MR::startLevelSoundPlayer("RockDashStart", -1, -1);
                mIsFirstRoll = false;
            }
        }
    }
}

// NONMATCHING: integer conversion spills and floating-point operand order differ.
void RollingRockStateRoll::exeAccel() {
    if (MR::isFirstStep(this)) {
        MR::startLevelSoundPlayer("RockAccel", -1, -1);
        MR::tryHitReaction(mHost, sRumbleAccel);
        TVec3f vel(mHost->mVelocity);
        MR::fn_80010D10(mHost, vel, mTurnRate);
        mHost->appearBlur();
        mDashStep -= 100;
        if (mDashStep <= 0) {
            mDashStep = 0;
        }
    }
    turnByStick();
    if (!tryJump() && !tryReflect() && !tryTimeOver()) {
        if (MR::isGreaterStep(this, 45)) {
            setNerve(&NrvRollingRockStateRoll::RollingRockStateRollNrvMove::sInstance);
        }
        s32 pitchBase = sPitchBase;
        s32 pitchScale = sPitchScale;
        s32 pitch = (s32)MR::clamp(pitchBase + pitchScale * PSVECMag(mHost->mVelocity), 0.0f, 1000.0f);
        MR::startActionSound(mHost, "Move", pitch, (s32)MR::clamp(pitchBase + pitchScale * PSVECMag(mHost->mVelocity), 0.0f, 1000.0f), -1);
    }
}

void RollingRockStateRoll::exeForceDash() {
    if (MR::isFirstStep(this)) {
        mHost->appearBlur();
        MR::startLevelSoundPlayer("RockAccel", -1, -1);
        MR::tryHitReaction(mHost, sRumbleForceDash);
        mDashStep = 0;
    }
    if (MR::testCorePadTriggerA(0)) {
        mJumpBuffer = 5;
    }
    if (!tryReflect() && !tryTimeOver() && MR::isGreaterStep(this, 45)) {
        setNerve(&NrvRollingRockStateRoll::RollingRockStateRollNrvMove::sInstance);
    }
}

bool RollingRockStateRoll::turnByStick() {
    mStickRate = 0.0f;
    if (mIsDead) {
        return false;
    }
    if (mAirStep > 5) {
        return false;
    }
    if (!MR::isGamePadStickOperated(0)) {
        return false;
    }
    if (MR::isNearZero(mHost->mVelocity, 0.001f)) {
        return false;
    }
    TVec3f stick(mHost->mVelocity);
    RollingRock::calcStickDir(&stick);
    TVec3f dir(mHost->mVelocity);
    MR::normalize(&dir);
    mStickRate = MR::abs(dir.dot(stick));
    MR::turnDirectionDegree(mHost, &dir, stick, 1.2f);
    f32 speed = MR::calcVelocityLength(mHost);
    TVec3f vel(dir);
    vel.scale(speed);
    MR::setVelocity(mHost, vel);
    return true;
}

bool RollingRockStateRoll::turnByStickAppear() {
    if (!MR::isGamePadStickOperated(0)) {
        return false;
    }
    if (mStep < 30) {
        return false;
    }
    TVec3f stick;
    RollingRock::calcStickDir(&stick);
    MR::turnDirectionDegree(mHost, &mDir, stick, 1.75f);
    return true;
}

bool RollingRockStateRoll::turnByStickStart() {
    if (!MR::isGamePadStickOperated(0)) {
        return false;
    }
    if (mStep < 30) {
        return false;
    }
    TVec3f stick;
    RollingRock::calcStickDir(&stick);
    MR::turnDirectionDegree(mHost, &mDir, stick, 1.75f);
    return true;
}

bool RollingRockStateRoll::tryForceDash() {
    if (mAirStep > 5) {
        return false;
    }
    if (!mHost->isOnForceDashGround()) {
        return false;
    }
    TVec3f vel;
    if (!RollingRock::calcForceDashVelocity(&vel, mHost, 50.0f)) {
        return false;
    }
    mTurnRate = 20.0f;
    MR::setVelocity(mHost, vel);
    mSpeed = MR::calcVelocityLength(mHost);
    RollingRock::getForceDashStep(&mDashStepMax, mHost, 180);
    setNerve(&NrvRollingRockStateRoll::RollingRockStateRollNrvForceDash::sInstance);
    return true;
}

bool RollingRockStateRoll::tryPadSwing() {
    if (!MR::isPadSwing(0)) {
        return false;
    }
    if (mAirStep <= 5) {
        return false;
    }
    mTurnRate = 3.0f;
    setNerve(&NrvRollingRockStateRoll::RollingRockStateRollNrvAccel::sInstance);
    return true;
}

bool RollingRockStateRoll::tryTimeOver() {
    if (mHost->mArg7 == 1) {
        return false;
    }
    if (mDashStep < mDashStepMax) {
        return false;
    }
    changeState(0);
    return true;
}

void RollingRockStateRoll::exeJump() {
    if (MR::isFirstStep(this)) {
        MR::tryHitReaction(mHost, sRumbleJump);
        MR::addVelocityJump(mHost, 33.0f);
        mJumpBuffer = 0;
    }
    if (MR::testCorePadTriggerA(0)) {
        mJumpBuffer = 5;
    }
    if (!tryEnd() && !tryPadSwing() && !tryReflect()) {
        if (tryLand()) return;
    }
}

namespace {
    const f32 sJumpGravityRate = 0.72f;
    const f32 sReflectSideRate = 0.3f;
}
