#pragma auto_inline off

#include "Game/MapObj/LeafBoard.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Map/WaterInfo.hpp"
#include "Game/Util.hpp"

// Float constants (.sdata2) used by LeafBoard.

namespace MR {
    // NONMATCHING: SMG2 target calls scaleVelocity__2MRFP9LiveActorf with a TVec3f ref and reads f1 (symbol name looks wrong).
    f32 scaleVelocity(LiveActor*, const TVec3f&);
}

// Unnamed SMG2 helpers (C linkage so the target reloc names match).
extern "C" void fn_802C2AA0(LiveActor* pActor, s32 flag, f32 a, f32 b, f32 c);
extern "C" void fn_8000E670(TVec3f* pDst, const LiveActor* pActor, const TVec3f* pPos, s32 flag);

namespace NrvLeafBoard {
    NEW_NERVE(LeafBoardNrvWait, LeafBoard, Wait);
    NEW_NERVE(LeafBoardNrvFloat, LeafBoard, Float);
    NEW_NERVE(LeafBoardNrvWind, LeafBoard, Wind);
}

LeafBoard::LeafBoard(const char* pName) : LiveActor(pName) {
    f32 zero = 0.0f;
    f32 one = 1.0f;
    mRot.x = zero;
    mRot.y = zero;
    mRot.z = zero;
    mRot.w = one;
    mDir.x = zero;
    mDir.y = one;
    mDir.z = zero;
    mPlayerPos.x = zero;
    mPlayerPos.y = zero;
    mPlayerPos.z = zero;
    mShadowPos.x = zero;
    mShadowPos.y = zero;
    mShadowPos.z = zero;
    mShadowNormal.x = zero;
    mShadowNormal.y = one;
    mShadowNormal.z = zero;
    mShadowAlpha = zero;
    mTimer104 = 0;
    mFlag108 = false;
    mMtx90.identity();
}

void LeafBoard::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, "LeafBoard", false);
    MR::makeQuatFromRotate(&mRot, this);
    MR::setBinderExceptSensorType(this, 0x34);
    MR::excludeCalcShadowToMyCollision(this, nullptr);
    MR::setShadowSurfaceOvalColor(this, "Water", Color8(0, 64, 64, 128));
    MR::setShadowSurfaceOvalAlpha(this, "Water", 64);
    MR::setShadowProjectionPtr(this, "Water", &mShadowPos, &mShadowNormal);
    MR::setShadowSurfaceOvalAlpha(this, "Water", (u8)(s32)mShadowAlpha);
    MR::setEffectHostMtx(this, "Ripple", mMtx90);
    MR::emitEffect(this, "Ripple");
    initNerve(GET_NERVE(LeafBoard, LeafBoardNrvWait), 0);
    makeActorAppeared();
}

void LeafBoard::initAfterPlacement() {
    MR::invalidateCollisionParts(this);
    MR::trySetMoveLimitCollision(this);
    MR::validateCollisionParts(this);
}

void LeafBoard::control() {
    s32 timer = mTimer104;
    mFlag108 = false;
    if (timer > 0) {
        mTimer104 = timer - 1;
    }
    MR::turnQuatYDirRate(&mRot, mRot, mDir, 0.03f);
    updateWaterShadow();
    MR::makeMtxUpNoSupportPos(&mMtx90, mShadowNormal, mShadowPos);
    updateMoveSound();
}

// NONMATCHING: target calls out-of-line TVec3 operator+ (__pl) for mPosition + sp8; ours inlines it (shared header inline).
void LeafBoard::updateWaterShadow() {
    TVec3f sp8 = mGravity * 500.0f;
    TVec3f sp14 = mPosition + sp8;
    TVec3f sp20;
    sp20.set(sp14);
    mShadowAlpha = 0.0f;
    WaterInfo info;
    if (MR::getWaterAreaObj(&info, sp20)) {
        if (MR::getWaterAreaInfo(&info, sp20, mGravity, false)) {
            mShadowPos.set(info.mSurfacePos);
            mShadowNormal.set(info.mSurfaceNormal);
            mShadowAlpha = 64.0f;
            MR::validateShadow(this, "Water");
        }
    } else {
        MR::invalidateShadow(this, "Water");
    }
}

void LeafBoard::calcAndSetBaseMtx() {
    MR::setBaseTRMtx(this, mRot);
}

void LeafBoard::updateMoveSound() {
    f32 speed = MR::calcVelocityLength(this);

    if (speed <= 8.0f) {
        MR::startActionSound(this, "Waiting", -1, -1, -1);
    }
    if (speed > 1.0f && speed < 16.0f) {
        f32 vol;
        if (speed < 13.0f) {
            vol = MR::getLinerValueFromMinMax(speed, 1.0f, 8.0f, 0.1f, 1.0f);
        } else {
            vol = MR::getLinerValueFromMinMax(speed, 13.0f, 16.0f, 1.0f, 0.0f);
        }
        MR::startActionSound(this, "MoveSlow", (s32)(1000.0f * vol), -1, -1);
    }
    if (speed > 8.0f) {
        MR::startActionSound(this, "MoveMiddle", (s32)(1000.0f * MR::getLinerValueFromMinMax(speed, 8.0f, 16.0f, 0.1f, 1.0f)), -1, -1);
    }
    if (speed > 13.0f) {
        MR::startActionSound(this, "MoveMidHigh", (s32)(1000.0f * MR::getLinerValueFromMinMax(speed, 13.0f, 16.0f, 0.1f, 1.0f)), -1, -1);
    }
    if (speed > 16.0f) {
        MR::startActionSound(this, "MoveFast", (s32)(1000.0f * MR::getLinerValueFromMinMax(speed, 16.0f, 27.0f, 0.1f, 1.0f)), -1, -1);
    }
    if (speed > 22.0f) {
        MR::startActionSound(this, "MoveWind", (s32)(1000.0f * MR::getLinerValueFromMinMax(speed, 22.0f, 50.0f, 0.1f, 1.0f)), -1, -1);
    }
}

void LeafBoard::reboundFromWall() {
    if (MR::isBindedWall(this)) {
        TVec3f normal = -MR::getWallNormal(this);
        f32 speed = MR::scaleVelocity(this, normal);
        if (speed > 5.0f) {
            MR::startAction(this, "Reaction");
            MR::tryHitReaction(this, "\x82\xd4\x82\xc2\x82\xa9\x82\xe8");
            MR::startActionSound(this, "HitWall", (s32)(100.0f * speed), -1, -1);
        }
    }
    MR::reboundVelocityFromEachCollision(this, 0.0f, 0.3f, 0.0f, 2.0f);
}

void LeafBoard::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorType(pReceiver, 0x34)) {
        MR::sendMsgPushAndKillVelocityToTarget(this, pReceiver, pSender);
    }
}

bool LeafBoard::receiveMsgPush(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorEnemy(pSender) || MR::isSensorNpc(pSender) || MR::isSensorMapObj(pSender) || MR::isSensorRide(pSender)) {
        f32 threshold = 6.0f;
        if (MR::isSensorType(pSender, 0x34)) {
            threshold = 4.0f;
        }
        TVec3f horizon;
        MR::calcSensorHorizonNormalize(&horizon, mGravity, pSender, pReceiver);
        if (mVelocity.dot(horizon) < threshold) {
            TVec3f push = horizon * threshold;
            MR::addVelocity(this, push);
        }
        MR::startAction(this, "Reaction");
        MR::tryHitReaction(this, "\x82\xd4\x82\xc2\x82\xa9\x82\xe8");
        MR::startActionSound(this, "HitWall", 0x320, -1, -1);
        return true;
    }
    return false;
}

#pragma dont_inline on
bool LeafBoard::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgFloorTouch(msg)) {
        mPlayerPos.set(*MR::getPlayerPos());  // out-of-line set<f> in target
        mFlag108 = true;
        mTimer104 = 60;
    }
    return false;
}
#pragma dont_inline reset

void LeafBoard::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Wait");
    }
    TVec3f up = -mGravity;
    mDir.set(up);
    fn_802C2AA0(this, 0, 40.0f, 0.2f, 50.0f);
    MR::attenuateVelocity(this, 0.96f);
    if (mTimer104 > 0) {
        setNerve(GET_NERVE(LeafBoard, LeafBoardNrvFloat));
    }
}

void LeafBoard::exeFloat() {
    TVec3f up = -mGravity;
    mDir.set(up);
    fn_802C2AA0(this, 0, 100.0f, 0.4f, 40.0f);
    MR::attenuateVelocity(this, 0.992f);
    reboundFromWall();
    if (MR::isGreaterStep(this, 30)) {
        setNerve(GET_NERVE(LeafBoard, LeafBoardNrvWind));
    }
}

// NONMATCHING: stack slots of sp8/sp14 are swapped vs target (same calls and order).
void LeafBoard::exeWind() {
    TVec3f up = -mGravity;
    TVec3f dir(up);
    TVec3f sp20;
    sp20.set(0.0f, 0.0f, 0.0f);
    f32 var_f31 = 0.97f;
    f32 var_f30 = 0.0f;
    if (mFlag108) {
        fn_8000E670(&sp20, this, &mPlayerPos, 0);
        f32 len = PSVECMag(&sp20);
        var_f30 = MR::normalize(len, 100.0f, 150.0f);
        var_f31 = MR::getLinerValue(var_f30, 1.0f, 0.97f, 1.0f);
        if (MR::normalizeOrZero(&sp20)) {
            dir.set(up);
        } else {
            dir.set(sp20);
        }
    }
    MR::turnVecToVecCos(&mDir, up, dir, var_f31, mGravity, 0.02f);
    TVec3f sp8 = sp20 * 0.2f;
    TVec3f sp14 = sp8 * var_f30;
    MR::addVelocity(this, sp14);
    fn_802C2AA0(this, 0, 100.0f, 0.4f, 40.0f);
    MR::attenuateVelocity(this, 0.992f);
    reboundFromWall();
    if (mTimer104 <= 0) {
        MR::zeroVelocity(this);
        setNerve(GET_NERVE(LeafBoard, LeafBoardNrvWait));
    }
}

LeafBoard::~LeafBoard() {
}
