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
    _90 = nullptr;
    _94 = nullptr;
    _98 = -1;
    _9C = 0.0f;
    _A0 = nullptr;
    _A4 = nullptr;
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
bool KoopaJrCastleDominate::fn_800D71B0(LiveActor* binder, s32 sensor, const TVec3f* pos, s32 cancelStep) {
    if (MR::isOnGroundPlayer() && !MR::isPlayerInRush()) {
        MR::tryRumblePadMiddle(this, 0);
        _94 = binder;
        _90 = MR::getSensorHost(reinterpret_cast<HitSensor*>(sensor));
        const TVec3f& gravity = _90->mGravity;
        _98 = cancelStep;
        MR::setPosition(this, _90->mPosition);
        TVec3f diff(*pos);
        JMathInlineVEC::PSVECSubtract(&diff, &mPosition, &diff);
        TVec3f horizontal(0.0f);
        MR::vecKillElement(diff, gravity, &horizontal);
        f32 height = diff.dot(gravity);
        f32 speed = _9C;
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
        MR::startBckPlayer(_A0, nullptr);
        return true;
    }
    return false;
}

void KoopaJrCastleDominate::kill() {
    if (_90) {
        MR::endBindAndPlayerJumpWithRollLanding(_94, mVelocity, 0);
        _90 = nullptr;
        _94 = nullptr;
    }
    LiveActor::kill();
}

void KoopaJrCastleDominate::fn_800D7560(const char* start, const char* wait) {
    _A0 = start;
    _A4 = wait;
}

void KoopaJrCastleDominate::exe800D7570() {
    MR::attenuateVelocity(this, 0.98f);
    MR::addVelocityToGravity(this, 0.7f);
    if (MR::isStep(this, 10)) MR::onBind(this);
    if (_90 && _A4 && MR::isBckOneTimeAndStopped(_90)) MR::startBckPlayer(_A4, nullptr);
    if (_90 && _98 >= 0 && MR::isGreaterEqualStep(this, _98) && MR::isPadSwing(0)) {
        MR::startSoundPlayer("SE_PV_TWIST_START", -1, -1);
        MR::startSoundPlayer("SE_PM_SPIN_ATTACK", -1, -1);
        MR::tryRumblePadMiddle(this, 0);
        MR::startBckPlayer("Spin", nullptr);
        MR::endBindAndPlayerJump(_94, mVelocity, 0);
        _90 = nullptr;
        _94 = nullptr;
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
    if (_90) return _90->getBaseMtx();
    return nullptr;
}

void KoopaJrCastleDominate::updateBaseMtx() {
    TPos3f mtx;
    mtx.identity();
    MR::makeMtxTR((MtxPtr)&mtx, this);
    MR::setBaseTRMtx(_90, mtx);
}

KoopaJrCastleDominate::~KoopaJrCastleDominate() {}
