// Keep vector arithmetic helpers outlined for the ride updates.
#define JGEOMETRY_VEC3_WMR_OUTLINE
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Enemy/AttackRock.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/BreakModel.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/MaterialCtrl.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/LiveActor/PartsModel.hpp"
#include "Game/LiveActor/Spine.hpp"
#include "Game/LiveActor/VolumeDrawer.hpp"
#include "Game/Util/FixedPosition.hpp"
#include "Game/Boss/KoopaFinalBattlePos.hpp"

#include "Game/Boss/KoopaBattleAccessor.hpp"
#include "Game/Util/BaseMatrixFollowTargetHolder.hpp"

AttackRockRide::AttackRockRide(LiveActor* pHost)
    : mHost(pHost), mQuat0(0.0f, 0.0f, 0.0f, 1.0f), mQuat1(0.0f, 0.0f, 0.0f, 1.0f), mQuat2(0.0f, 0.0f, 0.0f, 1.0f) {
    this->mHostZDir.set(0.0f, 0.0f, 1.0f);
    this->mRidePos.set(0.0f, 0.0f, 0.0f);
    this->mPlayerMove.set(0.0f, 0.0f, 0.0f);
    this->mDrift.set(0.0f, 0.0f, 0.0f);
    this->mStartDir.set(0.0f, 0.0f, 1.0f);
    this->mTargetDir.set(0.0f, 0.0f, 1.0f);
    this->mRotAxis.set(0.0f, 0.0f, 1.0f);
    this->mRotAngle = 0.0f;
    this->mRotAngleSum = 0.0f;
    this->mStartDistance = 0.0f;
    this->mRushFrames = 0;
    this->mRushStep = 0;
    this->mIsRush = false;
    this->mIsEnable = false;
}

void AttackRockRide::setHostZDir(const TVec3f* pVec) {
    this->mHostZDir = *pVec;
}

// NONMATCHING: Only stack temporary slot offsets differ (calls, operands and registers match).
void AttackRockRide::initRush() {
    MR::getPlayerQuatAndPos(&mQuat0, &mRidePos);
    TVec3f tmp(*MR::getPlayerLastMove());
    tmp.scale(0.9f);
    this->mPlayerMove.set(tmp);
    this->mDrift.set(this->mPlayerMove);
    MR::calcDirToPlayer(&this->mStartDir, this->mHost);
    TVec3f playerMove = *MR::getPlayerLastMove();
    const TVec3f& hostPos = this->mHost->mPosition;
    TVec3f sum(this->mRidePos);
    sum.add(playerMove);
    TVec3f dir = sum - hostPos;
    MR::normalizeOrZero(&dir);
    this->mRotAngle = MR::diffAngleAbs(this->mStartDir, dir);
    this->mRotAngleSum = 0.0f;
    PSVECCrossProduct(&this->mStartDir, &dir, &this->mRotAxis);
    if (MR::normalizeOrZero(&this->mRotAxis)) {
        MR::makeAxisVerticalZX(&this->mRotAxis, this->mStartDir);
    }
    this->mRushStep = 0;
    this->mQuat1.set(this->mQuat0);
    MR::rotateVecRadian(&dir, this->mStartDir, this->mRotAxis, 10.0f * this->mRotAngle);
    TVec3f up;
    up.x = 0.0f;
    up.y = 1.0f;
    up.z = 0.0f;
    MR::turnVecToVecDegree(&this->mTargetDir, this->mHostZDir, dir, 45.0f, up);
    TVec3f side = this->mTargetDir - this->mStartDir;
    if (MR::isNearZero(side, 0.001f)) {
        MR::makeAxisVerticalZX(&side, this->mTargetDir);
    }
    MR::makeQuatUpFront(&this->mQuat2, this->mTargetDir, side);
    f32 angle = 57.29578f * MR::diffAngleAbs(this->mStartDir, this->mTargetDir);
    s32 frames = (s32)(angle / 3.0f);
    this->mRushFrames = frames + 15;
    this->mStartDistance = PSVECDistance(&this->mRidePos, &this->mHost->mPosition);
    MR::startBckPlayer("Fall", "SpinDriverFall");
}

// NONMATCHING: Store/load scheduling of mRotAngleSum/mRotAngle and FPR allocation differ (same calls and operands).
void AttackRockRide::updateRush() {
    if (this->mIsRush) {
        f32 a = this->mRotAngle;
        f32 sum = this->mRotAngleSum + a;
        this->mRotAngleSum = sum;
        f32 t = (f32)this->mRushStep / (f32)this->mRushFrames;
        this->mRotAngle = a * 0.95f;
        f32 invT = 1.0f - t;
        TVec3f rotated;
        MR::rotateVecRadian(&rotated, this->mStartDir, this->mRotAxis, sum * invT);
        TVec3f blend;
        MR::vecBlendSphere(&blend, this->mStartDir, this->mTargetDir, t);
        MR::normalizeOrZero(&blend);
        f32 len = MR::lerp(this->mStartDistance, 300.0f, t);
        TVec3f pos(blend);
        pos.x *= len;
        pos.y *= len;
        pos.z *= len;
        this->mRidePos.set(pos);
        this->mRidePos.add(this->mHost->mPosition);
        TVec3f drift(this->mDrift);
        drift.scale(invT);
        this->mRidePos.add(drift);
        this->mQuat0.set(this->mQuat1);
        this->mQuat0.slerp(this->mQuat2, t);
        this->mPlayerMove.x *= 0.9f;
        this->mRushStep += 1;
        this->mPlayerMove.y *= 0.9f;
        this->mPlayerMove.z *= 0.9f;
        this->mDrift.add(this->mPlayerMove);
        if (this->mRushStep > this->mRushFrames) {
            TVec3f jump(this->mTargetDir);
            jump.scale(-20.0f);
            MR::endBindAndPlayerJump(this->mHost, jump, 0);
            this->mIsRush = false;
            this->mIsEnable = false;
        }
    }
}

bool AttackRockRide::tryReceiveRideMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgAutoRushBegin(msg) && !MR::isOnGroundPlayer() && this->mIsEnable) {
        this->initRush();
        this->mIsRush = true;
        this->mIsEnable = false;
        return true;
    }
    if (MR::isMsgUpdateBaseMtx(msg) && this->mIsRush) {
        MR::setPlayerQuatAndPos(&this->mQuat0, &this->mRidePos);
        return true;
    }
    if (MR::isMsgRushCancel(msg)) {
        this->mIsRush = false;
        return true;
    }
    if (MR::isMsgFloorTouch(msg)) {
        this->mIsEnable = false;
    }
    return false;
}

void AttackRockRide::enableRide() {
    this->mIsEnable = true;
}

void AttackRockRide::endRide() {
    if (this->mIsRush) {
        MR::endBindAndPlayerWait(this->mHost);
        this->mIsRush = false;
    }
    this->mIsEnable = false;
}

