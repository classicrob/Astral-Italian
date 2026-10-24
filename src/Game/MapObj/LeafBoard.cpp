#pragma auto_inline off

#include "Game/Util/ActorMovementUtil.hpp"
#include "Game/MapObj/LeafBoard.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Map/WaterInfo.hpp"
#include "Game/Util.hpp"


namespace MR {
bool addVelocityKeepHeightUseShadowAndWater(LiveActor* actor, const char* shadowName, f32 height, f32 accel, f32 maxCorrection) {
    f32 distance = 100000.0f;
    bool inWater = false;
    TVec3f position;
    TVec3f probe = actor->mPosition + actor->mGravity * height;
    position.set(probe);
    WaterInfo info;
    if (getWaterAreaObj(&info, position) && getWaterAreaInfo(&info, position, actor->mGravity, false)) {
        distance = height - info.mCamWaterDepth;
        inWater = true;
    }
    if (shadowName) {
        if (isShadowProjected(actor, shadowName)) {
            f32 shadowDistance = getShadowProjectionLength(actor, shadowName);
            if (!inWater || shadowDistance < distance) distance = shadowDistance;
        }
    } else if (isShadowProjectedAny(actor)) {
        f32 shadowDistance = getShadowNearProjectionLength(actor);
        if (!inWater || shadowDistance < distance) distance = shadowDistance;
    }
    distance -= height;
    addVelocityToGravity(actor, accel * normalizeAbs(distance, -maxCorrection, maxCorrection));
    return true;
}
}

namespace NrvLeafBoard {
    NEW_NERVE(LeafBoardNrvWait, LeafBoard, Wait);
    NEW_NERVE(LeafBoardNrvFloat, LeafBoard, Float);
    NEW_NERVE(LeafBoardNrvWind, LeafBoard, Wind);
}

LeafBoard::LeafBoard(const char* pName) : LiveActor(pName) {
    f32 one = 1.0f;
    f32 zero = 0.0f;
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
    mFloatTimer = 0;
    mTouchedFloorFrame = false;
    mWaterMtx.identity();
}

void LeafBoard::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, "LeafBoard", false);
    MR::makeQuatFromRotate(&mRot, this);
    MR::setBinderExceptSensorType(this, 0x34);
    MR::excludeCalcShadowToMyCollision(this, nullptr);
    MR::setShadowSurfaceOvalColor(this, "Water", Color8(0, 64, 64, 128));
    MR::setShadowSurfaceOvalAlpha(this, "Water", 64);
    MR::setShadowProjectionPtr(this, "Water", &mShadowPos, &mShadowNormal);
    MR::setShadowSurfaceOvalAlpha(this, "Water", (u8)mShadowAlpha);
    MR::setEffectHostMtx(this, "Ripple", mWaterMtx);
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
    s32 timer = mFloatTimer;
    mTouchedFloorFrame = false;
    if (timer > 0) {
        mFloatTimer = timer - 1;
    }
    MR::turnQuatYDirRate(&mRot, mRot, mDir, 0.03f);
    updateWaterShadow();
    MR::makeMtxUpNoSupportPos(&mWaterMtx, mShadowNormal, mShadowPos);
    updateMoveSound();
}

void LeafBoard::updateWaterShadow() {
    TVec3f sp20;
    TVec3f sp14 = mPosition + mGravity * 500.0f;
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
        f32 speed = MR::calcVelocityDot(this, normal);
        if (speed > 5.0f) {
            MR::startAction(this, "Reaction");
            // Shift-JIS "ぶつかり" (bump) hit reaction
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
        // Shift-JIS "ぶつかり" (bump) hit reaction
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
        mTouchedFloorFrame = true;
        mFloatTimer = 60;
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
    MR::addVelocityKeepHeightUseShadowAndWater(this, 0, 40.0f, 0.2f, 50.0f);
    MR::attenuateVelocity(this, 0.96f);
    if (mFloatTimer > 0) {
        setNerve(GET_NERVE(LeafBoard, LeafBoardNrvFloat));
    }
}

void LeafBoard::exeFloat() {
    TVec3f up = -mGravity;
    mDir.set(up);
    MR::addVelocityKeepHeightUseShadowAndWater(this, 0, 100.0f, 0.4f, 40.0f);
    MR::attenuateVelocity(this, 0.992f);
    reboundFromWall();
    if (MR::isGreaterStep(this, 30)) {
        setNerve(GET_NERVE(LeafBoard, LeafBoardNrvWind));
    }
}

void LeafBoard::exeWind() {
    TVec3f up = -mGravity;
    TVec3f dir(up);
    TVec3f sp20;
    sp20.set(0.0f, 0.0f, 0.0f);
    f32 turnCos = 0.97f;
    f32 playerProximity = 0.0f;
    if (mTouchedFloorFrame) {
        MR::calcVecToTargetHorizon(&sp20, this, &mPlayerPos, 0);
        f32 len = PSVECMag(&sp20);
        playerProximity = MR::normalize(len, 100.0f, 150.0f);
        turnCos = MR::getLinerValue(playerProximity, 1.0f, 0.97f, 1.0f);
        if (MR::normalizeOrZero(&sp20)) {
            dir.set(up);
        } else {
            dir.set(sp20);
        }
    }
    MR::turnVecToVecCos(&mDir, up, dir, turnCos, mGravity, 0.02f);
    TVec3f sp14 = sp20 * 0.2f * playerProximity;
    MR::addVelocity(this, sp14);
    MR::addVelocityKeepHeightUseShadowAndWater(this, 0, 100.0f, 0.4f, 40.0f);
    MR::attenuateVelocity(this, 0.992f);
    reboundFromWall();
    if (mFloatTimer <= 0) {
        MR::zeroVelocity(this);
        setNerve(GET_NERVE(LeafBoard, LeafBoardNrvWait));
    }
}

LeafBoard::~LeafBoard() {
}
