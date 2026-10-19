#pragma auto_inline off
#include "Game/Boss/KoopaJrCastleNerves.hpp"
#include <cmath>
#include "Game/LiveActor/ActorCameraInfo.hpp"
#include "Game/LiveActor/BreakModel.hpp"
#include "Game/LiveActor/LodCtrl.hpp"
#include "Game/LiveActor/PartsModelLodCtrl.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Map/CollisionParts.hpp"
#include "Game/Util/JointController.hpp"
#include "Game/Ride/FluffWind.hpp"
#include "Game/LiveActor/ModelObj.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/NPC/TalkMessageCtrl.hpp"

namespace NrvKoopaJrCastleDominate { KoopaJrCastleDominateNrvA KoopaJrCastleDominateNrvA::sInstance ATTRIBUTE_WEAK; }

KoopaJrCastleDominate::KoopaJrCastleDominate(const char* pName) : LiveActor(pName) {
    mBoundActor = nullptr;
    mBinder = nullptr;
    mCancelStep = -1;
    mLaunchSpeed = 0.0f;
    mStartActionName = nullptr;
    mWaitActionName = nullptr;
}

void KoopaJrCastleDominate::init(const JMapInfoIter&) {
    MR::connectToScene(this, 0x29, -1, -1, -1);
    initEffectKeeper(0, "KoopaJrCastlePlayerLuncher", false);
    initNerve(&NrvKoopaJrCastleDominate::KoopaJrCastleDominateNrvA::sInstance, 0);
    initHitSensor(1);
    MR::addHitSensorEnemy(this, "Body", 8, 100.0f, TVec3f(0.0f, 0.0f, 0.0f));
    initBinder(100.0f, 100.0f, 0);
    MR::invalidateClipping(this);
    makeActorDead();
}

// NONMATCHING: floating-point register allocation and vector temporary scheduling
bool KoopaJrCastleDominate::tryBindPlayer(LiveActor* binder, HitSensor* sensor, const TVec3f* pos, s32 cancelStep) {
    if (MR::isOnGroundPlayer() && !MR::isPlayerInRush()) {
        MR::tryRumblePadMiddle(this, 0);
        mBinder = binder;
        mBoundActor = MR::getSensorHost(sensor);
        const TVec3f& gravity = mBoundActor->mGravity;
        mCancelStep = cancelStep;
        MR::setPosition(this, mBoundActor->mPosition);
        TVec3f diff(*pos);
        JMathInlineVEC::PSVECSubtract(&diff, &mPosition, &diff);
        TVec3f horizontal(0.0f);
        MR::vecKillElement(diff, gravity, &horizontal);
        f32 height = diff.dot(gravity);
        f32 speed = mLaunchSpeed;
        f32 time = (-speed + MR::fastSqrtf(speed * speed + (2.0f * 0.7f) * height)) / -0.7f;
        TVec3f negGravity(-gravity);
        TVec3f verticalVelocity(negGravity);
        verticalVelocity.scale(speed);
        TVec3f negHorizontal(-horizontal);
        TVec3f horizontalVelocity(negHorizontal);
        horizontalVelocity.scale(1.0f / time);
        TVec3f velocity(horizontalVelocity);
        velocity += verticalVelocity;
        TVec3f front(-horizontal);
        MR::normalizeOrZero(&front);
        TPos3f mtx;
        MR::makeMtxFrontUpPos(&mtx, front, -mGravity, mPosition);
        mtx.getEuler(mRotation);
        mRotation.x *= 57.295776f;
        mRotation.y *= 57.295776f;
        mRotation.z *= 57.295776f;
        MR::setVelocity(this, velocity);
        LiveActor::appear();
        setNerve(&NrvKoopaJrCastleDominate::KoopaJrCastleDominateNrvA::sInstance);
        MR::offBind(this);
        MR::startBckPlayer(mStartActionName, nullptr);
        return true;
    }
    return false;
}

void KoopaJrCastleDominate::kill() {
    if (mBoundActor) {
        MR::endBindAndPlayerJumpWithRollLanding(mBinder, mVelocity, 0);
        mBoundActor = nullptr;
        mBinder = nullptr;
    }
    LiveActor::kill();
}

void KoopaJrCastleDominate::setPlayerActionNames(const char* start, const char* wait) {
    mStartActionName = start;
    mWaitActionName = wait;
}

void KoopaJrCastleDominate::exeBindPlayerFall() {
    MR::attenuateVelocity(this, 0.98f);
    MR::addVelocityToGravity(this, 0.7f);
    if (MR::isStep(this, 10)) MR::onBind(this);
    if (mBoundActor && mWaitActionName && MR::isBckOneTimeAndStopped(mBoundActor)) MR::startBckPlayer(mWaitActionName, nullptr);
    if (mBoundActor && mCancelStep >= 0 && MR::isGreaterEqualStep(this, mCancelStep) && MR::isPadSwing(0)) {
        MR::startSoundPlayer("SE_PV_TWIST_START", -1, -1);
        MR::startSoundPlayer("SE_PM_SPIN_ATTACK", -1, -1);
        MR::tryRumblePadMiddle(this, 0);
        MR::startBckPlayer("Spin", nullptr);
        MR::endBindAndPlayerJump(mBinder, mVelocity, 0);
        mBoundActor = nullptr;
        mBinder = nullptr;
        kill();
        return;
    }
    if (MR::isBindedGround(this)) {
        MR::shakeCameraNormal();
        MR::tryRumblePadStrong(this, 0);
        MR::emitEffect(this, "LandSmoke");
        MR::startSoundPlayer("SE_PM_FALLDOWN_M", -1, -1);
        HitSensor* sensor = getSensor(nullptr);
        MR::sendArbitraryMsg(0xF4, MR::getGroundSensor(this), sensor);
        kill();
    }
}

void KoopaJrCastleDominate::attackSensor(HitSensor* sender, HitSensor* receiver) {
    if (MR::isSensorMapObj(receiver) && MR::sendArbitraryMsg(0xF4, receiver, sender))
        MR::tryRumblePad(this, "\x8e\xe3", 0);
}

MtxPtr KoopaJrCastleDominate::getBaseMtx() const {
    if (mBoundActor) return mBoundActor->getBaseMtx();
    return nullptr;
}

void KoopaJrCastleDominate::updateBaseMtx() {
    TPos3f mtx;
    mtx.identity();
    MR::makeMtxTR((MtxPtr)&mtx, this);
    MR::setBaseTRMtx(mBoundActor, mtx);
}

KoopaJrCastleDominate::~KoopaJrCastleDominate() {}
