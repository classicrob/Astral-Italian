#pragma auto_inline off

#include "Game/MapObj/LeafBoard.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Map/WaterInfo.hpp"
#include "Game/Util.hpp"

// Float constants (.sdata2) used by LeafBoard.
extern "C" f32 lbl_807DDAA8;
extern "C" f32 lbl_807DDAAC;
extern "C" f32 lbl_807DDAB0;
extern "C" f32 lbl_807DDAB8;
extern "C" f32 lbl_807DDABC;
extern "C" f32 lbl_807DDAC0;
extern "C" f32 lbl_807DDAC4;
extern "C" f32 lbl_807DDAC8;
extern "C" f32 lbl_807DDACC;
extern "C" f32 lbl_807DDAD0;
extern "C" f32 lbl_807DDAD4;
extern "C" f32 lbl_807DDAD8;
extern "C" f32 lbl_807DDADC;
extern "C" f32 lbl_807DDAE0;
extern "C" f32 lbl_807DDAE4;
extern "C" f32 lbl_807DDAE8;
extern "C" f32 lbl_807DDAEC;
extern "C" f32 lbl_807DDAF0;
extern "C" f32 lbl_807DDAF4;
extern "C" f32 lbl_807DDAF8;
extern "C" f32 lbl_807DDAFC;
extern "C" f32 lbl_807DDB00;
extern "C" f32 lbl_807DDB04;
extern "C" f32 lbl_807DDB08;
extern "C" f32 lbl_807DDB0C;
extern "C" f32 lbl_807DDB10;
extern "C" f32 lbl_807DDB14;

namespace MR {
    // NONMATCHING: SMG2 target calls scaleVelocity__2MRFP9LiveActorf with a TVec3f ref and reads f1 (symbol name looks wrong).
    f32 scaleVelocity(LiveActor*, const TVec3f&);
}

// Unnamed SMG2 helpers (C linkage so the target reloc names match).
extern "C" void fn_802C2AA0(LiveActor* pActor, s32 flag, f32 a, f32 b, f32 c);
extern "C" void fn_8000E670(TVec3f* pDst, const LiveActor* pActor, const TVec3f* pPos, s32 flag);
extern "C" void fn_800345E0(LiveActor* pActor, const char* pName);

namespace NrvLeafBoard {
    NEW_NERVE(LeafBoardNrvWait, LeafBoard, Wait);
    NEW_NERVE(LeafBoardNrvFloat, LeafBoard, Float);
    NEW_NERVE(LeafBoardNrvWind, LeafBoard, Wind);
}

LeafBoard::LeafBoard(const char* pName) : LiveActor(pName) {
    f32 zero = lbl_807DDAAC;
    f32 one = lbl_807DDAA8;
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
    MR::turnQuatYDirRate(&mRot, mRot, mDir, lbl_807DDAB8);
    updateWaterShadow();
    MR::makeMtxUpNoSupportPos(&mMtx90, mShadowNormal, mShadowPos);
    updateMoveSound();
}

// NONMATCHING: target calls out-of-line TVec3 operator+ (__pl) for mPosition + sp8; ours inlines it (shared header inline).
void LeafBoard::updateWaterShadow() {
    TVec3f sp8 = mGravity * lbl_807DDABC;
    TVec3f sp14 = mPosition + sp8;
    TVec3f sp20;
    sp20.set(sp14);
    mShadowAlpha = lbl_807DDAAC;
    WaterInfo info;
    if (MR::getWaterAreaObj(&info, sp20)) {
        if (MR::getWaterAreaInfo(&info, sp20, mGravity, false)) {
            mShadowPos.set(info.mSurfacePos);
            mShadowNormal.set(info.mSurfaceNormal);
            mShadowAlpha = lbl_807DDAC0;
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

    if (speed <= lbl_807DDAC4) {
        MR::startActionSound(this, "Waiting", -1, -1, -1);
    }
    if (speed > lbl_807DDAA8 && speed < lbl_807DDAC8) {
        f32 vol;
        if (speed < lbl_807DDAD0) {
            vol = MR::getLinerValueFromMinMax(speed, lbl_807DDAA8, lbl_807DDAC4, lbl_807DDACC, lbl_807DDAA8);
        } else {
            vol = MR::getLinerValueFromMinMax(speed, lbl_807DDAD0, lbl_807DDAC8, lbl_807DDAA8, lbl_807DDAAC);
        }
        MR::startActionSound(this, "MoveSlow", (s32)(lbl_807DDAD4 * vol), -1, -1);
    }
    if (speed > lbl_807DDAC4) {
        MR::startActionSound(this, "MoveMiddle", (s32)(lbl_807DDAD4 * MR::getLinerValueFromMinMax(speed, lbl_807DDAC4, lbl_807DDAC8, lbl_807DDACC, lbl_807DDAA8)), -1, -1);
    }
    if (speed > lbl_807DDAD0) {
        MR::startActionSound(this, "MoveMidHigh", (s32)(lbl_807DDAD4 * MR::getLinerValueFromMinMax(speed, lbl_807DDAD0, lbl_807DDAC8, lbl_807DDACC, lbl_807DDAA8)), -1, -1);
    }
    if (speed > lbl_807DDAC8) {
        MR::startActionSound(this, "MoveFast", (s32)(lbl_807DDAD4 * MR::getLinerValueFromMinMax(speed, lbl_807DDAC8, lbl_807DDAD8, lbl_807DDACC, lbl_807DDAA8)), -1, -1);
    }
    if (speed > lbl_807DDADC) {
        MR::startActionSound(this, "MoveWind", (s32)(lbl_807DDAD4 * MR::getLinerValueFromMinMax(speed, lbl_807DDADC, lbl_807DDAE0, lbl_807DDACC, lbl_807DDAA8)), -1, -1);
    }
}

void LeafBoard::reboundFromWall() {
    if (MR::isBindedWall(this)) {
        TVec3f normal = -MR::getWallNormal(this);
        f32 speed = MR::scaleVelocity(this, normal);
        if (speed > lbl_807DDAE4) {
            MR::startAction(this, "Reaction");
            fn_800345E0(this, "\x82\xd4\x82\xc2\x82\xa9\x82\xe8");
            MR::startActionSound(this, "HitWall", (s32)(lbl_807DDAE8 * speed), -1, -1);
        }
    }
    MR::reboundVelocityFromEachCollision(this, lbl_807DDAAC, lbl_807DDAEC, lbl_807DDAAC, lbl_807DDAB0);
}

void LeafBoard::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorType(pReceiver, 0x34)) {
        MR::sendMsgPushAndKillVelocityToTarget(this, pReceiver, pSender);
    }
}

bool LeafBoard::receiveMsgPush(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorEnemy(pSender) || MR::isSensorNpc(pSender) || MR::isSensorMapObj(pSender) || MR::isSensorRide(pSender)) {
        f32 threshold = lbl_807DDAF4;
        if (MR::isSensorType(pSender, 0x34)) {
            threshold = lbl_807DDAF8;
        }
        TVec3f horizon;
        MR::calcSensorHorizonNormalize(&horizon, mGravity, pSender, pReceiver);
        if (mVelocity.dot(horizon) < threshold) {
            TVec3f push = horizon * threshold;
            MR::addVelocity(this, push);
        }
        MR::startAction(this, "Reaction");
        fn_800345E0(this, "\x82\xd4\x82\xc2\x82\xa9\x82\xe8");
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
    fn_802C2AA0(this, 0, lbl_807DDAFC, lbl_807DDB00, lbl_807DDAE0);
    MR::attenuateVelocity(this, lbl_807DDB04);
    if (mTimer104 > 0) {
        setNerve(GET_NERVE(LeafBoard, LeafBoardNrvFloat));
    }
}

void LeafBoard::exeFloat() {
    TVec3f up = -mGravity;
    mDir.set(up);
    fn_802C2AA0(this, 0, lbl_807DDAE8, lbl_807DDB08, lbl_807DDAFC);
    MR::attenuateVelocity(this, lbl_807DDB0C);
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
    sp20.set(lbl_807DDAAC, lbl_807DDAAC, lbl_807DDAAC);
    f32 var_f31 = lbl_807DDB10;
    f32 var_f30 = lbl_807DDAAC;
    if (mFlag108) {
        fn_8000E670(&sp20, this, &mPlayerPos, 0);
        f32 len = PSVECMag(&sp20);
        var_f30 = MR::normalize(len, lbl_807DDAE8, lbl_807DDAF0);
        var_f31 = MR::getLinerValue(var_f30, lbl_807DDAA8, lbl_807DDB10, lbl_807DDAA8);
        if (MR::normalizeOrZero(&sp20)) {
            dir.set(up);
        } else {
            dir.set(sp20);
        }
    }
    MR::turnVecToVecCos(&mDir, up, dir, var_f31, mGravity, lbl_807DDB14);
    TVec3f sp8 = sp20 * lbl_807DDB00;
    TVec3f sp14 = sp8 * var_f30;
    MR::addVelocity(this, sp14);
    fn_802C2AA0(this, 0, lbl_807DDAE8, lbl_807DDB08, lbl_807DDAFC);
    MR::attenuateVelocity(this, lbl_807DDB0C);
    reboundFromWall();
    if (mTimer104 <= 0) {
        MR::zeroVelocity(this);
        setNerve(GET_NERVE(LeafBoard, LeafBoardNrvWait));
    }
}

LeafBoard::~LeafBoard() {
}
