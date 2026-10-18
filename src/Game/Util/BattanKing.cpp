#define JGEOMETRY_VEC3_SET_NOINLINE
#include "Game/Enemy/BattanKing.hpp"
#include "Game/Util.hpp"

#include "Game/Enemy/Pattan.hpp"
#include "Game/LiveActor/VolumeDrawer.hpp"
#include "Game/LiveActor/LiveActorGroup.hpp"
#include "Game/Util/ActorAnimUtil.hpp"
#include "Game/Util/ActorMovementUtil.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/ActorShadowUtil.hpp"
#include "Game/Util/ActorStateUtil.hpp"
#include "Game/Util/DemoUtil.hpp"
#include "Game/Util/EffectUtil.hpp"
#include "Game/Util/JointUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/MtxUtil.hpp"
#include "Game/Util/NerveUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/TalkUtil.hpp"
#include "Game/Map/CollisionParts.hpp"
#include "Game/LiveActor/HitSensor.hpp"

#include "Game/Util/BattanKingCollisionPress.hpp"
#include <JSystem/JMath/JMath.hpp>

namespace {
    inline bool isInsidePushBox(const TVec3f& pos, const TVec3f& range, const f32& depth) {
        if (!(0.5f * -range.x <= pos.x && pos.x < 0.5f * range.x)) return false;
        if (!(0.0f <= pos.y && pos.y < range.y)) return false;
        if (!(0.5f * -depth <= pos.z && pos.z < 0.5f * depth)) return false;
        return true;
    }
}

void BattanKing::calcAndSetBaseMtx() {
    MR::calcAndSetBaseTRMtxFromGravityAndZAxis(this, mFront);
    PSMTXCopy(getBaseMtx(), mMtxFollow);
    MR::addTransMtxLocal(mMtxFollow, BattanKingVec::sOffset920);
    MR::calcFrontVec(&mFront, this);
}

// NONMATCHING: regalloc
void BattanKing::calcAnim() {
    LiveActor::calcAnim();
    if (!MR::isDead(this) && !MR::isNoCalcAnim(this)) {
        if (MR::isClipped(this)) {
            return;
        } else {
            for (s32 i = 0; i < 5; i++) {
                mCollisions[i]->calcAnim();
                mCollisionPresses[i]->calcAnim();
            }
        }
    }
}

// NONMATCHING: regalloc
void BattanKing::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    const char* strings = BattanKingData::sStringTable;
    if (MR::isEqualSensor(pSender, this, strings + 0x370)) {
        if (isNerve(&NrvBattanKing::NrvE94::sInstance) && MR::isGreaterEqualStep(this, 0x28) &&
            MR::isLessStep(this, 0x3F)) {
            TVec3f push(0.0f);
            if (calcPlayerPush(&push)) {
                MR::pushPlayer(push * 1.0f);
            }
        } else if (isNerve(&NrvBattanKing::NrvEA0::sInstance) && MR::isGreaterEqualStep(this, BattanKingParam::sPushStartStep) &&
                   MR::isLessStep(this, 0x6E)) {
            TVec3f push(0.0f);
            if (calcPlayerPush(&push)) {
                MR::pushPlayer(push * 1.0f);
            }
        } else {
            mFlagB = 0;
            mFlagC = 0;
        }
    } else if (MR::isSensorPlayer(pReceiver)) {
        if (isNerve(&NrvBattanKing::NrvE94::sInstance)) {
            if (MR::isEqualSensor(pSender, this, strings + 0x164) || MR::isEqualSensor(pSender, this, strings + 0x17C)) {
                MR::sendMsgPush(pReceiver, pSender);
                return;
            }
        } else {
            bool isWalking = false;
            if (isNerve(&NrvBattanKing::NrvE74::sInstance) || isNerve(&NrvBattanKing::NrvE7C::sInstance)) isWalking = true;
            if (isWalking) {
                if (MR::isEqualSensor(pSender, this, strings + 0x184) || MR::isEqualSensor(pSender, this, strings + 0x18C) ||
                    MR::isEqualSensor(pSender, this, strings + 0x378) || MR::isEqualSensor(pSender, this, strings + 0x380)) {
                    MR::sendMsgPush(pReceiver, pSender);
                    return;
                }
            }
        }
    } else {
        bool isWalking = false;
        if (isNerve(&NrvBattanKing::NrvE74::sInstance) || isNerve(&NrvBattanKing::NrvE7C::sInstance)) isWalking = true;
        if (isWalking && MR::isSensorEnemy(pReceiver)) {
            if (MR::sendArbitraryMsg(0xF4, pReceiver, pSender)) return;
        }
    }
}

bool BattanKing::receiveMsgPlayerAttack(u32 msg, HitSensor*, HitSensor*) {
    if (MR::isMsgPlayerHipDropFloor(msg)) {
        if (MR::isPlayerInCollisionCylinder(MR::getJointMtx(this, "Root"), &BattanKingVec::sDropPoint938, 450.0f, 300.0f)) {
            mAttackType = 0;
            return tryAdvanceAttack();
        }
        return false;
    }
    return false;
}

// NONMATCHING: regalloc
bool BattanKing::receiveMsgEnemyAttack(u32, HitSensor*, HitSensor* pReceiver) {
    if (!MR::isSensorAutoRush(pReceiver)) {
        return false;
    } else {
        return MR::isPlayerElementModeInvincible();
    }
}

bool BattanKing::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    return false;
}

void BattanKing::resetVolumes() {
    mPhase = 0;
    for (s32 i = 0; i < 7; i++) {
        mVolumeDrawers[i]->makeActorAppeared();
        mVolumeDrawers[i]->setScaleDownRate(0);
    }
    mEarthquakeDrawer->makeActorAppeared();
    mEarthquakeDrawer->setScaleDownRate(0);
    mAttackType = 0;
    mTalkStateA->kill();
    mTalkStateB->kill();
    hideCollisionPartsAll();
}

bool BattanKing::isAttackPhaseStart() const {
    bool result = false;
    if (mFlagA == 0 && (isNerve(&NrvBattanKing::NrvE98::sInstance) || isNerve(&NrvBattanKing::NrvE9C::sInstance) ||
        (isNerve(&NrvBattanKing::NrvEA0::sInstance) && MR::isLessStep(this, 0x28)))) {
        result = true;
    }
    return result;
}

bool BattanKing::tryAdvanceAttack() {
    const char* strings = BattanKingData::sStringTable;
    if (isAttackPhaseStart()) {
        s32 phase = mPhase;
        mFlagA = 1;
        mPhase = phase + 1;
        updateJointVisibility();
        if (mPhase < 3) {
            setNerve(&NrvBattanKing::NrvEA4::sInstance);
            MR::tryHitReaction(this, strings + 0x388);
            return true;
        }
        MR::tryDeleteEffect(this, strings + 0x2CC);
        setNerve(&NrvBattanKing::NrvEA8::sInstance);
        MR::tryHitReaction(this, strings + 0x398);
        return true;
    }
    return false;
}

// NONMATCHING: regalloc
void BattanKing::spawnPattans() {
    s32 count = 0;
    if (mPhase == 0) count = BattanKingParam::sInitialPattanCount;
    if (mPhase == 1) count = 3;
    if (mPhase == 2) count = 5;
    const TVec3f* gravity = &mGravity;
    for (s32 i = 0; i < count; i++) {
        PattanHolder* holder = mPattanHolder;
        LiveActor* actor = holder->getDeadActor() ? holder->getDeadActor() : nullptr;
        if (actor == nullptr) break;
        f32 angle = (360.0f * (f32)i) / (f32)count;
        TVec3f offset(BattanKingVec::sOrigin944);
        MR::rotateVecDegree(&offset, offset, *gravity, angle);
        TMtx34f mtx;
        PSMTXCopy(getBaseMtx(), mtx);
        MR::addTransMtxLocal(mtx, offset);
        MR::extractMtxTrans(mtx, &actor->mPosition);
        actor->mGravity = *gravity;
        TVec3f front(mFront);
        MR::calcVecToPlayerH(&front, actor, nullptr);
        appearPattanByJump(actor, &front);
    }
}

void BattanKing::updateJointVisibility() {
    const char* strings = BattanKingData::sStringTable;
    MR::hideJoint(this, strings + 0x3B0);
    MR::hideJoint(this, strings + 0x3B8);
    MR::hideJoint(this, strings + 0x3C0);
    if (mPhase == 0) {
        MR::showJoint(this, strings + 0x3B0);
    }
    if (mPhase == 1) {
        MR::showJoint(this, strings + 0x3B8);
    }
    if (mPhase == 2) {
        MR::showJoint(this, strings + 0x3C0);
    }
    if (mPhase == 3) {
        MR::showJoint(this, strings + 0x3C0);
    }
}

void BattanKing::updateShockPos() {
    TVec3f pos(0.0f);
    TVec3f origin;
    origin = mPosition;
    TVec3f dir;
    MR::calcVecToPlayerH(&dir, this, nullptr);
    pos.set<f32>(origin);
    JMAVECScaleAdd(mGravity, pos, pos, -1000.0f);
    JMAVECScaleAdd(dir, pos, pos, -100.0f);
    mUnk1B0 = pos;
    TMtx34f mtx;
    PSMTXCopy(getBaseMtx(), mtx);
    MR::addTransMtxLocal(mtx, BattanKingVec::sOffset92C);
    f32 z = mtx[2][3];
    f32 y = mtx[1][3];
    f32 x = mtx[0][3];
    mUnk1BC.set<f32>(x, y, z);
}

void BattanKing::showCollisionPartsNormal() {
    for (s32 i = 0; i < 5; i++) {
        mCollisions[i]->appear();
        mCollisionPresses[i]->kill();
    }
}

void BattanKing::showCollisionPartsSwapped() {
    for (s32 i = 0; i < 5; i++) {
        mCollisions[i]->kill();
        mCollisionPresses[i]->appear();
    }
}

void BattanKing::updateCollisionPartsFootState() {
    showCollisionPartsNormal();
    mCollisions[3]->kill();
    mCollisions[4]->kill();
    mCollisionPresses[3]->appear();
    mCollisionPresses[4]->appear();
}

void BattanKing::hideCollisionPartsAll() {
    for (s32 i = 0; i < 5; i++) {
        mCollisions[i]->kill();
        mCollisionPresses[i]->kill();
    }
}

bool BattanKing::isAnyCollisionOnActor() {
    for (s32 i = 0; i < 5; i++) {
        if (MR::isOnPlayer(mCollisions[i])) {
            return true;
        }
        if (MR::isOnPlayer(mCollisionPresses[i])) {
            return true;
        }
    }
    return MR::isOnPlayer(this);
}

void BattanKing::restoreDemoPose() {
    MR::setPosition(this, mUnk180);
    mFront = mUnk18C;
    calcAndSetBaseMtx();
}

void BattanKing::updateDemoPlayerBaseMtx() {
    TMtx34f mtx;
    PSMTXCopy(mMtxFx, mtx);
    MR::addTransMtxLocal(mtx, BattanKingVec::sOffset914);
    MR::rotateMtxLocalYDegree(mtx, 40.0f);
    MR::setPlayerBaseMtx(mtx);
}

bool BattanKing::isValidCollisionState(s32 id) const {
    if (id == 0) {
        if ((isNerve(&NrvBattanKing::NrvE94::sInstance) && MR::isGreaterEqualStep(this, 0x3D)) ||
            isNerve(&NrvBattanKing::NrvE98::sInstance) || isNerve(&NrvBattanKing::NrvE9C::sInstance) ||
            isNerve(&NrvBattanKing::NrvEA4::sInstance)) {
            return true;
        }
        return false;
    } else if (id == 1) {
        if (!isNerve(&NrvBattanKing::NrvE68::sInstance) && !isNerve(&NrvBattanKing::NrvE6C::sInstance) &&
            !isNerve(&NrvBattanKing::NrvEA8::sInstance) && !isNerve(&NrvBattanKing::NrvEAC::sInstance)) {
            return true;
        }
        return false;
    } else if (id == 2) {
        if ((!isNerve(&NrvBattanKing::NrvE94::sInstance) || !MR::isGreaterEqualStep(this, 0x3D)) &&
            !isNerve(&NrvBattanKing::NrvE98::sInstance) && !isNerve(&NrvBattanKing::NrvE9C::sInstance) &&
            !isNerve(&NrvBattanKing::NrvEA4::sInstance) && !isNerve(&NrvBattanKing::NrvE68::sInstance) &&
            !isNerve(&NrvBattanKing::NrvE6C::sInstance) && !isNerve(&NrvBattanKing::NrvEA8::sInstance) &&
            !isNerve(&NrvBattanKing::NrvEAC::sInstance)) {
            return true;
        }
        return false;

    }
    return false;
}

bool BattanKing::isInvalidCollisionParts(const CollisionParts* pParts) {
    LiveActor* pHost = pParts->mHitSensor->mHost;
    for (s32 i = 0; i < 5; i++) {
        if (mCollisions[i] == pHost) return true;
        if (mCollisionPresses[i] == pHost) return true;
    }
    return false;
}

void BattanKing::updateAttackAnim(s32 phase) {
    mAttackType = phase;
    if (MR::isOnGroundPlayer() && !isAnyCollisionOnActor()) {
        MR::tryHitReaction(this, "攻撃・プレイヤー地上");
    } else {
        MR::tryHitReaction(this, "攻撃・プレイヤー空中");
    }
}

// NONMATCHING: regalloc
bool BattanKing::calcPlayerPush(TVec3f* pPush) {
    u8 wasFront = mFlagB;
    u8 wasInside = mFlagC;
    TMtx34f inverse;
    TMtx34f mtx;
    PSMTXCopy(MR::getJointMtx(this, "BattanKingBody"), mtx);
    MR::addTransMtxLocal(mtx, BattanKingVec::sPushOffset);
    PSMTXInverse(mtx, inverse);
    TVec3f local(0.0f);
    PSMTXMultVec(inverse, MR::getPlayerCenterPos(), local);
    f32 z = local.z;
    bool inside = isInsidePushBox(local, BattanKingVec::sRange95C, 600.0f);
    u8 isFront = z >= 0.0f;
    mFlagB = isFront;
    if (!inside) return false;
    mFlagC = 1;
    MR::extractMtxZDir(MR::getJointMtx(this, "BattanKingBody"), pPush);
    f32 edge;
    if (wasFront) edge = 0.5f * BattanKingVec::sRange95C.z;
    else edge = 0.5f * -BattanKingVec::sRange95C.z;
    *pPush *= edge - z;
    if (wasInside && isFront != wasFront) {
        mFlagB = wasFront;
        return true;
    }
    if (z < 0.5f * BattanKingVec::sRange95C.z) return true;
    return false;
}
