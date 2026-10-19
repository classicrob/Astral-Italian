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

namespace NrvKoopaJrCastleCannonBall { KoopaJrCastleCannonBallNrvMove KoopaJrCastleCannonBallNrvMove::sInstance ATTRIBUTE_WEAK; }

KoopaJrCastleCannonBall::KoopaJrCastleCannonBall(const char* pName) : LiveActor(pName) {
    mDir.x = 0.0f;
    mDir.y = 0.0f;
    mDir.z = 0.0f;
    mShotSpeed = 0.0f;
}

void KoopaJrCastleCannonBall::init(const JMapInfoIter&) {
    MR::initActor(this, "ElectricBullet", "KoopaJrCastle", false);
    initNerve(&NrvKoopaJrCastleCannonBall::KoopaJrCastleCannonBallNrvMove::sInstance, 0);
    makeActorDead();
}

void KoopaJrCastleCannonBall::kill() {
    MR::tryHitReactionBreak(this);
    MR::emitEffect(this, "Break");
    MR::startActionSound(this, "EmElecbublletBreak", -1, -1, -1);
    LiveActor::kill();
}

// NONMATCHING: matrix component loads are scheduled in the opposite argument order
void KoopaJrCastleCannonBall::shoot(MtxPtr mtx, const f32& speed) {
    f32 shotSpeed = speed;
    f32 z = mtx[2][2];
    f32 y = mtx[1][2];
    f32 x = mtx[0][2];
    mShotSpeed = shotSpeed;
    mDir.set<f32>(x, y, z);
    MR::normalize(&mDir);
    mVelocity.scale(mShotSpeed, mDir);
    reinterpret_cast<TPos3f*>(mtx)->getTrans(mPosition);
    mRotation.z = 0.0f;
    mRotation.y = 0.0f;
    mRotation.x = 0.0f;
    makeActorAppeared();
    MR::validateHitSensors(this);
    MR::invalidateClipping(this);
    setNerve(&NrvKoopaJrCastleCannonBall::KoopaJrCastleCannonBallNrvMove::sInstance);
}

// NONMATCHING: string-address registers and vector temporary stack placement
void KoopaJrCastleCannonBall::exe800D1830() {
    const char* const reaction = "\x94\x6a\x89\xf3\x81\x69\x97\x5d\x89\x43\x82\xa0\x82\xe8\x81\x6a";
    const char* const effect = "BreakLong";
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Shot");
        MR::startBtk(this, "ElectricBullet");
    }
    if (MR::isBckOneTimeAndStopped(this)) MR::startAction(this, "Move");
    MR::startActionSound(this, "EmLvElecbublletFly", -1, -1, -1);
    if (MR::isGreaterEqualStep(this, 10) && MR::isLessStep(this, 90)) {
        TVec3f target(0.0f, 0.0f, 0.0f);
        TVec3f up(0.0f, 1.0f, 0.0f);
        MR::getPlayerUpVec(&up);
        JMAVECScaleAdd(&up, MR::getPlayerPos(), &target, -120.0f);
        TVec3f toTarget;
        JMathInlineVEC::PSVECSubtract(&target, &mPosition, &toTarget);
        MR::normalize(&toTarget);
        TVec3f dir(mDir);
        TVec3f antiGravity(-mGravity);
        f32 turnCos = MR::cosDegree(0.35f);
        MR::turnVecToVecCos(&dir, dir, toTarget, turnCos, -antiGravity, 0.02f);
        TVec3f planar;
        MR::turnVecToPlane(&planar, dir, antiGravity);
        TVec3f axis;
        PSVECCrossProduct(&planar, &dir, &axis);
        if (!MR::normalizeOrZero(&axis))
            MR::turnVecToVecCos(&dir, dir, planar, MR::cosDegree(0.05f), axis, 0.02f);
        mDir = dir;
        mVelocity.scale(mShotSpeed, mDir);
    }
    if (MR::isBinded(this)) {
        if (MR::isBindedGround(this)) {
            HitSensor* sensor = getSensor(nullptr);
            MR::sendMsgEnemyAttackElectric(MR::getGroundSensor(this), sensor);
        } else if (MR::isBindedRoof(this)) {
            HitSensor* sensor = getSensor(nullptr);
            MR::sendMsgEnemyAttackElectric(MR::getRoofSensor(this), sensor);
        } else if (MR::isBindedWall(this)) {
            HitSensor* sensor = getSensor(nullptr);
            MR::sendMsgEnemyAttackElectric(MR::getWallSensor(this), sensor);
        }
        MR::tryHitReaction(this, reaction);
        MR::emitEffect(this, effect);
        MR::startActionSound(this, "EmElecbublletBreak", -1, -1, -1);
        LiveActor::kill();
        return;
    }
    f32 length = 200.0f;
    TVec3f offset(mDir * 150.0f);
    TVec3f ahead(mPosition);
    ahead += offset;
    TVec3f halfGravity((mGravity * length) * 0.5f);
    TVec3f origin(ahead);
    JMathInlineVEC::PSVECSubtract(&origin, &halfGravity, &origin);
    TVec3f hitPos(0.0f);
    if (MR::getFirstPolyOnLineToMap(&hitPos, nullptr, origin, mGravity * length) && MR::getSensor(nullptr)) {
        HitSensor* sensor = getSensor(nullptr);
        if (MR::sendMsgEnemyAttackElectric(MR::getSensor(nullptr), sensor)) {
            MR::tryHitReaction(this, reaction);
            MR::emitEffect(this, effect);
            MR::startActionSound(this, "EmElecbublletBreak", -1, -1, -1);
            LiveActor::kill();
        }
    } else if (MR::isStep(this, 600)) kill();
}

void KoopaJrCastleCannonBall::control() {
    if (MR::isStarPointerPointing2PAttachTarget(this)) kill();
}

void KoopaJrCastleCannonBall::attackSensor(HitSensor* sender, HitSensor* receiver) {
    if ((MR::isSensorPlayerOrRide(receiver) || MR::isSensorEnemy(receiver) || MR::isSensorMapObj(receiver)) && MR::sendMsgEnemyAttackElectric(receiver, sender)) kill();
}

bool KoopaJrCastleCannonBall::receiveOtherMsg(u32 msg, HitSensor*, HitSensor*) {
    if (MR::isMsgStartPowerStarGet(msg)) {
        kill();
        return true;
    }
    return false;
}

KoopaJrCastleCannonBall::~KoopaJrCastleCannonBall() {}
