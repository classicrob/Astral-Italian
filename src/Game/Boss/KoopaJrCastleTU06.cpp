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
#include "Game/Util/FixedPosition.hpp"
#include <cstdio>
#include "Game/LiveActor/DemoObj.hpp"

namespace NrvKoopaJrCastleFireBar { KoopaJrCastleFireBarNrvA KoopaJrCastleFireBarNrvA::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleFireBar { KoopaJrCastleFireBarNrvB KoopaJrCastleFireBarNrvB::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleFireBar { KoopaJrCastleFireBarNrvC KoopaJrCastleFireBarNrvC::sInstance ATTRIBUTE_WEAK; }

KoopaJrCastleFireBar::KoopaJrCastleFireBar(const char* pName) : LiveActor(pName) {
    mFixedPos = nullptr;
    mMtxs = nullptr;
    mChildren = nullptr;
    mChildCount = 0;
    _A4.set(0.0f, 0.0f, 1.0f);
    _B0 = 0.0f;
    _B4 = -0.5f;
    _B8 = 3;
    _BC = 1.0f;
    _C0 = 650.0f;
    _C4 = 0.0f;
    _C8 = 3000.0f;
    _CC = true;
}

// NONMATCHING: the target calls TVec3f(f32,f32,f32) out of line and keeps 100.0f * 1.5f as a runtime fmuls; ours inlines/folds.
void KoopaJrCastleFireBar::init(const JMapInfoIter& rIter) {
    MR::connectToScene(this, 0x21, -1, -1, -1);
    initHitSensor(1);
    MR::addHitSensorCallbackEnemyAttack(this, "fire", 8, 30.0f);
    updateHitSensor(getSensor("fire"));
    initSound(3, "FireBar", NULL, TVec3f(0.0f));
    TVec3f boxSize(100.0f * 1.5f, 600.0f, _C8);
    MR::initShadowControllerList(this, _B8);
    mMtxs = new TPos3f[_B8];
    mNames = new char*[_B8];
    for (s32 i = 0; i < _B8; i++) {
        PSMTXIdentity(mMtxs[i]);
        mNames[i] = new char[16];
        snprintf(mNames[i], 16, "Arm%d", i);
        MR::addShadowVolumeBox(this, mNames[i], boxSize, mMtxs[i]);
    }
    MR::calcGravity(this);
    f32 span = (_C8 - _C0) / (100.0f * 1.5f);
    mChildCount = _B8 * (s32)(1.0f + span);
    createChildren();
    MR::invalidateClipping(this);
    initNerve(&NrvKoopaJrCastleFireBar::KoopaJrCastleFireBarNrvA::sInstance, 0);
    makeActorDead();
}

void KoopaJrCastleFireBar::makeActorAppeared() {
    for (s32 i = 0; i < mChildCount; i++) {
        mChildren[i]->makeActorAppeared();
        MR::emitEffect(mChildren[i], "Move");
    }
    LiveActor::makeActorAppeared();
    setNerve(&NrvKoopaJrCastleFireBar::KoopaJrCastleFireBarNrvA::sInstance);
    _C4 = 0.0f;
    _B0 = 0.0f;
    _BC = 1.0f;
    _CC = true;
}

void KoopaJrCastleFireBar::makeActorDead() {
    for (s32 i = 0; i < mChildCount; i++) {
        mChildren[i]->makeActorDead();
    }
    LiveActor::makeActorDead();
}

// NONMATCHING: same constant-folding / TVec3f ctor inlining difference as init.
void KoopaJrCastleFireBar::control() {
    mFixedPos->calc();
    TVec3f trans(0.0f);
    TVec3f yDir(0.0f);
    MR::extractMtxTrans(mFixedPos->mMtx, &trans);
    MR::extractMtxYDir(mFixedPos->mMtx, &yDir);
    MR::setPosition(this, trans);
    MR::setQuatUpNoSupport(this, yDir);
    if (_CC) {
        f32 sum = _B0 + _B4;
        _B0 = 0.0f + (f32)fmod(360.0f + (sum - 0.0f), 360.0);
    }
    TVec3f zDir;
    zDir.set(mFixedPos->mMtx[0][2], mFixedPos->mMtx[1][2], mFixedPos->mMtx[2][2]);
    MR::rotateVecDegree(&_A4, zDir, mGravity, _B0);
    updateChildPositions();
    for (s32 i = 0; i < _B8; i++) {
        TPos3f tmp;
        PSMTXCopy(mFixedPos->mMtx, tmp);
        MR::rotateMtxLocalYDegree(tmp, -(_B0 + 360.0f * i / _B8));
        MR::addTransMtxLocalY(tmp, 0.5f * (100.0f * 1.5f));
    }
    TVec3f boxSize(60.0f * 1.5f, 600.0f, _C4);
    for (s32 i = 0; i < _B8; i++) {
        PSMTXCopy(mFixedPos->mMtx, mMtxs[i]);
        MR::rotateMtxLocalYDegree(mMtxs[i], -(_B0 + 360.0f * i / _B8));
        MR::addTransMtxLocalY(mMtxs[i], 0.5f * -boxSize.y);
        MR::addTransMtxLocalZ(mMtxs[i], 0.5f * boxSize.z);
        MR::setShadowVolumeBoxSize(this, mNames[i], boxSize);
    }
    LiveActor* pNearest = mChildren[0];
    f32 nearest = MR::calcDistanceToPlayer(pNearest);
    for (s32 i = 1; i < mChildCount; i++) {
        f32 dist = MR::calcDistanceToPlayer(mChildren[i]);
        if (nearest > dist) {
            nearest = dist;
            pNearest = mChildren[i];
        }
    }
    if (nearest <= 1400.0f) {
        MR::startActionSound(pNearest, "JrCastleFireBallMoveNear", (s32)(1000.0f * (1.0f - nearest / 1400.0f)), -1, -1);
    }
}

// NONMATCHING: clamp branch layout: target uses cror/fcmpu for the >=/== compares, ours uses bge/fcmpo
void KoopaJrCastleFireBar::exe800D2B50() {
    f32 temp = _C4 + 25.0f;
    f32 v = 0.0f;
    f32 max = _C8;
    _C4 = temp;
    if (temp >= v) {
        if (temp > max) {
            v = max;
        } else {
            v = temp;
        }
    }
    _C4 = v;
    MR::startActionSound(this, "OjLvFireBarCenter", -1, -1, -1);
    if (_C4 == _C8) {
        setNerve(&NrvKoopaJrCastleFireBar::KoopaJrCastleFireBarNrvB::sInstance);
    }
}

void KoopaJrCastleFireBar::exe800D2BF0() {
    MR::isFirstStep(this);
    _BC = MR::calcNerveEaseInRate(this, 60);
    MR::startActionSound(this, "OjLvFireBarCenter", -1, -1, -1);
}

void KoopaJrCastleFireBar::setCurrentToMax() {
    _C4 = _C8;
}

// fn_800D2AB0: fixed-position helper bound to the host; a name selects the named variant.
void KoopaJrCastleFireBar::setup(LiveActor* pHost, const TVec3f& rPos, const TVec3f& rOffset, const char* pName) {
    if (pName != nullptr) {
        mFixedPos = new FixedPosition(pHost, pName, rPos, rOffset);
    } else {
        mFixedPos = new FixedPosition(pHost, rPos, rOffset);
    }
}

// fn_800D2C60: request movement on every child part.
void KoopaJrCastleFireBar::requestMovementOnChildren() {
    for (s32 i = 0; i < mChildCount; i++) {
        MR::requestMovementOn(mChildren[i]);
    }
}

// fn_800D2CC0: shadow every child except one per group of _B8 (group size mChildCount / _B8).
void KoopaJrCastleFireBar::updateChildShadows() {
    MR::invalidateShadowAll(this);
    s32 lastInGroup;
    s32 groupSize = mChildCount / _B8;
    lastInGroup = groupSize - 1;
    for (s32 i = 0; i < mChildCount; i++) {
        if (lastInGroup != i % groupSize) {
            MR::validateShadowAll(mChildren[i]);
        } else {
            MR::invalidateShadowAll(mChildren[i]);
        }
    }
}

void KoopaJrCastleFireBar::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorPlayerOrRide(pReceiver) && MR::isSensorEnemyAttack(pSender)) {
        MR::sendMsgEnemyAttackFire(pReceiver, pSender);
    }
}

// NONMATCHING: same constant-folding difference as init.
void KoopaJrCastleFireBar::updateHitSensor(HitSensor* pSensor) {
    TVec3f closest(mPosition);
    f32 minDist = 3.4028235e38f;
    s32 perArm = mChildCount / _B8;
    TVec3f up = -mGravity;
    TVec3f start;
    start.scale(_C0, _A4);
    JMAVECScaleAdd(&up, &start, &start, 0.5f * 100.0f * 1.5f);
    TVec3f end;
    end.scale(_C0 + 1.5f * ((f32)(perArm - 1) * 100.0f), _A4);
    JMAVECScaleAdd(&up, &end, &end, 0.5f * 100.0f * 1.5f);
    for (s32 i = 0; i < _B8; i++) {
        TVec3f a;
        a.add(start, mPosition);
        TVec3f b;
        b.add(end, mPosition);
        TVec3f foot;
        MR::calcPerpendicFootToLineInside(&foot, *MR::getPlayerCenterPos(), a, b);
        f32 dist = PSVECDistance(&foot, MR::getPlayerCenterPos());
        if (minDist > dist) {
            minDist = dist;
            closest = foot;
        }
        if (i + 1 != _B8) {
            TVec3f axis = -mGravity;
            MR::rotateVecDegree(&start, axis, 360.0f / _B8);
            MR::rotateVecDegree(&end, axis, 360.0f / _B8);
        }
    }
    MR::setSensorPos(pSensor, closest);
}

// NONMATCHING: GPR/FPR allocation order of the loop variables (frame/zero copies) differs.
void KoopaJrCastleFireBar::createChildren() {
    mChildren = new LiveActor*[mChildCount];
    s32 perArm = mChildCount / _B8;
    f32 frame = 0.0f;
    f32 zeroA = frame;
    f32 zeroB = frame;
    for (s32 i = 0; i < mChildCount; i++) {
        mChildren[i] = MR::createDemoObj("\x83N\x83\x62\x83p""Jr\x83L\x83\x83\x83\x62\x83X\x83\x8b\x83t\x83""A\x83""C\x83""A\x83{\x81[\x83\x8b", "FireRingBall", "KoopaJrCastle", NULL);
        MR::invalidateClipping(mChildren[i]);
        s32 m = i % perArm;
        if (perArm - 1 != m) {
            MR::invalidateShadowAll(mChildren[i]);
        }
        mChildren[i]->makeActorDead();
        if (m == 0) {
            frame = 0.0f;
        } else {
            frame += 25.0f;
        }
        f32 maxFrame = MR::getBtkFrameMax(mChildren[i]);
        frame = zeroB + (f32)fmod(maxFrame + (frame - zeroA), maxFrame);
        MR::setBtkFrame(mChildren[i], frame);
    }
    updateChildPositions();
}

// NONMATCHING: the target keeps 100.0f * 1.5f, 0.5f * 100.0f * 1.5f etc. as runtime fmuls of separate pool constants (ours folds them).
void KoopaJrCastleFireBar::updateChildPositions() {
    TVec3f dir;
    dir.scale(100.0f * 1.5f, _A4);
    TVec3f cur;
    cur.zero();
    s32 perArm = mChildCount / _B8;
    for (s32 i = 0; i < mChildCount; i++) {
        s32 m = i % perArm;
        if (m == 0) {
            TVec3f up = -mGravity;
            MR::rotateVecDegree(&dir, up, 360.0f / _B8);
            TVec3f unit;
            MR::normalize(&unit, dir);
            JMAVECScaleAdd(&unit, &mPosition, &cur, _C0);
            JMAVECScaleAdd(&up, &cur, &cur, 100.0f * 0.5f * 1.5f);
        } else {
            cur.add(dir);
        }
        f32 t = MR::normalize(_C4, _C0 + 1.5f * (100.0f * (f32)(m - 1)), _C0 + 1.5f * (100.0f * (f32)m));
        MR::setPosition(mChildren[i], cur);
        MR::setScale(mChildren[i], 1.5f * (_BC * t));
    }
}

KoopaJrCastleFireBar::~KoopaJrCastleFireBar() {}
