#define JGEOMETRY_VEC3_WMR_OUTLINE
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

namespace {
    extern const f32 sFireBarZero;
    extern const f32 sFireBarSpacing;
    extern const f32 sFireBarScale;
    extern const f32 sFireBarHalf;
    extern const f32 sFireBarShadowWidth;
}

namespace NrvKoopaJrCastleFireBar { KoopaJrCastleFireBarNrvA KoopaJrCastleFireBarNrvA::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleFireBar { KoopaJrCastleFireBarNrvB KoopaJrCastleFireBarNrvB::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleFireBar { KoopaJrCastleFireBarNrvC KoopaJrCastleFireBarNrvC::sInstance ATTRIBUTE_WEAK; }

KoopaJrCastleFireBar::KoopaJrCastleFireBar(const char* pName) : LiveActor(pName) {
    mFixedPos = nullptr;
    mMtxs = nullptr;
    mChildren = nullptr;
    mChildCount = 0;
    mFront.set(0.0f, 0.0f, 1.0f);
    mRotationAngle = 0.0f;
    mRotationSpeed = -0.5f;
    mArmCount = 3;
    mScaleRate = 1.0f;
    mInnerRadius = 650.0f;
    mCurrentLength = 0.0f;
    mMaxLength = 3000.0f;
    mIsRotating = true;
}

void KoopaJrCastleFireBar::init(const JMapInfoIter& rIter) {
    MR::connectToScene(this, 0x21, -1, -1, -1);
    initHitSensor(1);
    MR::addHitSensorCallbackEnemyAttack(this, "fire", 8, 30.0f);
    updateHitSensor(getSensor("fire"));
    initSound(3, "FireBar", NULL, TVec3f(0.0f));
    TVec3f boxSize(sFireBarSpacing * sFireBarScale, 600.0f, mMaxLength);
    MR::initShadowControllerList(this, mArmCount);
    mMtxs = new TPos3f[mArmCount];
    mNames = new char*[mArmCount];
    for (s32 i = 0; i < mArmCount; i++) {
        PSMTXIdentity(mMtxs[i].mMtx);
        mNames[i] = new char[16];
        snprintf(mNames[i], 16, "Arm%d", i);
        MR::addShadowVolumeBox(this, mNames[i], boxSize, mMtxs[i].mMtx);
    }
    MR::calcGravity(this);
    f32 span = (mMaxLength - mInnerRadius) / (sFireBarSpacing * sFireBarScale);
    mChildCount = mArmCount * (s32)(1.0f + span);
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
    mCurrentLength = 0.0f;
    mRotationAngle = 0.0f;
    mScaleRate = 1.0f;
    mIsRotating = true;
}

void KoopaJrCastleFireBar::makeActorDead() {
    for (s32 i = 0; i < mChildCount; i++) {
        mChildren[i]->makeActorDead();
    }
    LiveActor::makeActorDead();
}

void KoopaJrCastleFireBar::control() {
    mFixedPos->calc();
    TVec3f trans(0.0f);
    TVec3f yDir(0.0f);
    MR::extractMtxTrans(mFixedPos->mMtx, &trans);
    MR::extractMtxYDir(mFixedPos->mMtx, &yDir);
    MR::setPosition(this, trans);
    MR::setQuatUpNoSupport(this, yDir);
    if (mIsRotating) {
        f32 sum = mRotationAngle + mRotationSpeed;
        mRotationAngle = sFireBarZero + (f32)fmod(360.0f + (sum - sFireBarZero), 360.0);
    }
    TVec3f zDir;
    f32 z = mFixedPos->mMtx[2][2];
    f32 y = mFixedPos->mMtx[1][2];
    f32 x = mFixedPos->mMtx[0][2];
    zDir.set<f32>(x, y, z);
    MR::rotateVecDegree(&mFront, zDir, mGravity, mRotationAngle);
    updateChildPositions();
    for (s32 i = 0; i < mArmCount; i++) {
        TPos3f tmp;
        PSMTXCopy(mFixedPos->mMtx.mMtx, tmp.mMtx);
        MR::rotateMtxLocalYDegree(tmp.mMtx, -(mRotationAngle + 360.0f * i / mArmCount));
        MR::addTransMtxLocalY(tmp.mMtx, sFireBarHalf * sFireBarSpacing);
    }
    TVec3f boxSize(sFireBarShadowWidth * sFireBarScale, 600.0f, mCurrentLength);
    for (s32 i = 0; i < mArmCount; i++) {
        PSMTXCopy(mFixedPos->mMtx.mMtx, mMtxs[i].mMtx);
        MR::rotateMtxLocalYDegree(mMtxs[i].mMtx, -(mRotationAngle + 360.0f * i / mArmCount));
        MR::addTransMtxLocalY(mMtxs[i].mMtx, 0.5f * -boxSize.y);
        MR::addTransMtxLocalZ(mMtxs[i].mMtx, 0.5f * boxSize.z);
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

void KoopaJrCastleFireBar::exeExtend() {
    mCurrentLength += 25.0f;
    mCurrentLength = MR::clamp(mCurrentLength, 0.0f, mMaxLength);
    MR::startActionSound(this, "OjLvFireBarCenter", -1, -1, -1);
    if (mCurrentLength >= mMaxLength) {
        setNerve(&NrvKoopaJrCastleFireBar::KoopaJrCastleFireBarNrvB::sInstance);
    }
}

#pragma dont_inline on
void KoopaJrCastleFireBar::exeScaleUp() {
    MR::isFirstStep(this);
    mScaleRate = MR::calcNerveEaseInRate(this, 60);
    MR::startActionSound(this, "OjLvFireBarCenter", -1, -1, -1);
}

#pragma dont_inline off

void KoopaJrCastleFireBar::setCurrentToMax() {
    mCurrentLength = mMaxLength;
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
    s32 groupSize = mChildCount / mArmCount;
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

// NONMATCHING: vector assignment inlining and loop/register allocation differ.
void KoopaJrCastleFireBar::updateHitSensor(HitSensor* pSensor) {
    TVec3f closest(static_cast<const Vec&>(mPosition));
    f32 minDist = 3.4028235e38f;
    s32 i;
    s32 perArm = mChildCount / mArmCount;
    TVec3f up = -mGravity;
    TVec3f start;
    start.scale(mInnerRadius, mFront);
    JMAVECScaleAdd(&up, &start, &start, 0.5f * sFireBarSpacing * sFireBarScale);
    TVec3f end;
    end.scale(mInnerRadius + sFireBarScale * ((f32)(perArm - 1) * sFireBarSpacing), mFront);
    JMAVECScaleAdd(&up, &end, &end, 0.5f * sFireBarSpacing * sFireBarScale);
    i = 0;
    if (mArmCount > 0) {
        TVec3f& position = mPosition;
        for (; i < mArmCount; i++) {
            TVec3f a;
            a.add(start, position);
            TVec3f b;
            b.add(end, position);
            TVec3f foot;
            MR::calcPerpendicFootToLineInside(&foot, *MR::getPlayerCenterPos(), a, b);
            f32 dist = PSVECDistance(&foot, MR::getPlayerCenterPos());
            if (minDist > dist) {
                minDist = dist;
                closest.x = foot.x;
                closest.y = foot.y;
                closest.z = foot.z;
            }
            if (i + 1 != mArmCount) {
                TVec3f axis = -mGravity;
                MR::rotateVecDegree(&start, axis, 360.0f / mArmCount);
                MR::rotateVecDegree(&end, axis, 360.0f / mArmCount);
            }
        }
    }
    MR::setSensorPos(pSensor, closest);
}

// NONMATCHING: GPR/FPR allocation order of the loop variables (frame/zero copies) differs.
void KoopaJrCastleFireBar::createChildren() {
    s32 m;
    s32 perArm;
    s32 i;
    f32 zeroB;
    f32 zeroA;
    f32 frameStep;
    mChildren = new LiveActor*[mChildCount];
    perArm = mChildCount / mArmCount;
    f32 frame = 0.0f;
    zeroA = frame;
    frameStep = 25.0f;
    zeroB = frame;
    for (i = 0; i < mChildCount; i++) {
        mChildren[i] = MR::createDemoObj("クッパJrキャッスルファイアボール", "FireRingBall", "KoopaJrCastle", NULL);
        MR::invalidateClipping(mChildren[i]);
        m = i % perArm;
        if (perArm - 1 != m) {
            MR::invalidateShadowAll(mChildren[i]);
        }
        mChildren[i]->makeActorDead();
        if (m == 0) {
            frame = 0.0f;
        } else {
            frame += frameStep;
        }
        f32 maxFrame = MR::getBtkFrameMax(mChildren[i]);
        frame = zeroB + (f32)fmod(maxFrame + (frame - zeroA), maxFrame);
        MR::setBtkFrame(mChildren[i], frame);
    }
    updateChildPositions();
}

// NONMATCHING: scalar temporary FPR differs; spacing/half/scale grouping and operand order match.
void KoopaJrCastleFireBar::updateChildPositions() {
    TVec3f dir;
    dir.scale(sFireBarSpacing * sFireBarScale, mFront);
    TVec3f cur;
    cur.zero();
    s32 perArm = mChildCount / mArmCount;
    for (s32 i = 0; i < mChildCount; i++) {
        s32 m = i % perArm;
        if (m == 0) {
            TVec3f up = -mGravity;
            MR::rotateVecDegree(&dir, up, 360.0f / mArmCount);
            TVec3f unit;
            MR::normalize(&unit, dir);
            JMAVECScaleAdd(&unit, &mPosition, &cur, mInnerRadius);
            f32 halfSpacing = sFireBarSpacing * sFireBarHalf;
            f32 scaledHalfSpacing = halfSpacing * sFireBarScale;
            JMAVECScaleAdd(&up, &cur, &cur, scaledHalfSpacing);
        } else {
            JMathInlineVEC::PSVECAdd(&cur, &dir, &cur);
        }
        f32 t = MR::normalize(mCurrentLength, mInnerRadius + sFireBarScale * (sFireBarSpacing * (f32)(m - 1)), mInnerRadius + sFireBarScale * (sFireBarSpacing * (f32)m));
        MR::setPosition(mChildren[i], cur);
        MR::setScale(mChildren[i], (mScaleRate * t) * sFireBarScale);
    }
}

KoopaJrCastleFireBar::~KoopaJrCastleFireBar() {}

namespace {
    const f32 sFireBarZero = 0.0f;
    const f32 sFireBarSpacing = 100.0f;
    const f32 sFireBarScale = 1.5f;
    const f32 sFireBarHalf = 0.5f;
    const f32 sFireBarShadowWidth = 60.0f;
}
