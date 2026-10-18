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
#include "Game/Map/HitInfo.hpp"
#include "Game/LiveActor/HitSensor.hpp"

// Helpers of the BattanKing unit. Names are provisional (descriptive of the behaviour); the lead renames them.
namespace {
    void resetVolumes(BattanKing* pBattan);
    void hideAllCollisionParts(BattanKing* pBattan);
    void showCollisionPartsNormal(BattanKing* pBattan);
    void showCollisionPartsSwapped(BattanKing* pBattan);
    void updateCollisionPartsMtx(BattanKing* pBattan);
    void updateCollisionPartsFootState(BattanKing* pBattan);
    void hideCollisionPartsAll(BattanKing* pBattan);
    bool isAnyCollisionOnActor(BattanKing* pBattan);
    void setPositionAndFront(BattanKing* pBattan, const TVec3f& rPos);
    void updatePlayerBaseMtx(BattanKing* pBattan);
    bool tryAdvanceAttack(BattanKing* pBattan);
    void updateAttackAnim(BattanKing* pBattan, s32 phase);
    bool isAttackPhaseStart(const BattanKing* pBattan);
    void updateVolumeRange(BattanKing* pBattan);
    void updateJointVisibility(BattanKing* pBattan);
    void updateDropVector(BattanKing* pBattan, TVec3f* pVec);
    void hideAllVolumes(BattanKing* pBattan);
    void updateShockPos(BattanKing* pBattan);
}

void BattanKing::calcAndSetBaseMtx() {
    MR::calcAndSetBaseTRMtxFromGravityAndZAxis(this, mFront);
    PSMTXCopy(getBaseMtx(), mMtxFollow);
    MR::addTransMtxLocal(mMtxFollow, BattanKingVec::sOffset920);
    MR::calcFrontVec(&mFront, this);
}

void BattanKing::calcAnim() {
    if (MR::isDead(this) || MR::isNoCalcAnim(this)) {
        return;
    }
    LiveActor::calcAnim();
    if (MR::isClipped(this)) {
        return;
    }
    for (s32 i = 0; i < 5; i++) {
        mCollisions[i]->calcViewAndEntry();
        mCollisionPresses[i]->calcViewAndEntry();
    }
}

void BattanKing::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isEqualSensor(pSender, this, "Body")) {
        if (isNerve(&NrvBattanKing::NrvE94::sInstance) && MR::isGreaterEqualStep(this, 0x28) &&
            MR::isLessStep(this, 0x3F)) {
            TVec3f push(0.0f, 0.0f, 0.0f);
            if (isValidCollisionState(0)) {
                MR::pushPlayer(push * 600.0f);
            }
        } else if (isNerve(&NrvBattanKing::NrvEA0::sInstance) && MR::isGreaterEqualStep(this, 0x28) &&
                   MR::isLessStep(this, 0x6E)) {
            TVec3f push(0.0f, 0.0f, 0.0f);
            if (isValidCollisionState(0)) {
                MR::pushPlayer(push * 600.0f);
            }
        } else {
            mFlagB = 0;
            mFlagC = 0;
        }
    } else if (MR::isSensorPlayer(pReceiver)) {
        if (isNerve(&NrvBattanKing::NrvE94::sInstance)) {
            if (MR::isEqualSensor(pSender, this, "ArmL") || MR::isEqualSensor(pSender, this, "ArmR")) {
                MR::sendMsgPush(pReceiver, pSender);
            }
        } else if (isNerve(&NrvBattanKing::NrvE74::sInstance) || isNerve(&NrvBattanKing::NrvE7C::sInstance)) {
            if (MR::isEqualSensor(pSender, this, "FootL") || MR::isEqualSensor(pSender, this, "FootR") ||
                MR::isEqualSensor(pSender, this, "AttackSign") || MR::isEqualSensor(pSender, this, "QuakeSign")) {
                MR::sendMsgPush(pReceiver, pSender);
            }
        }
    } else {
        if (isNerve(&NrvBattanKing::NrvE74::sInstance) || isNerve(&NrvBattanKing::NrvE7C::sInstance)) {
            if (MR::isSensorEnemy(pReceiver)) {
                MR::sendArbitraryMsg(0xF4, pReceiver, pSender);
            }
        }
    }
}

bool BattanKing::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (!MR::isMsgPlayerHipDropFloor(msg)) {
        return false;
    }
    MR::getJointMtx(this, "Root");
    if (!isNearDropPoint(BattanKingVec::sDropPoint938, 450.0f, 300.0f)) {
        return false;
    }
    mAttackType = 0;
    return tryAdvanceAttack(this);
}

bool BattanKing::receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (!MR::isSensorAutoRush(pReceiver)) {
        return false;
    }
    return MR::isPlayerElementModeInvincible();
}

bool BattanKing::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    return false;
}

namespace {
    // fn_8008C4A0: reset the attack state, drawers and talk states.
    void resetVolumes(BattanKing* pBattan) {
        pBattan->mPhase = 0;
        for (s32 i = 0; i < 7; i++) {
            VolumeDrawer* pDrawer = pBattan->mVolumeDrawers[i];
            pDrawer->kill();
            pDrawer->setScaleDownRate(0);
        }
        pBattan->mEarthquakeDrawer->kill();
        pBattan->mEarthquakeDrawer->setScaleDownRate(0);
        pBattan->mAttackType = 0;
        pBattan->mTalkStateA->update();
        pBattan->mTalkStateB->update();
        hideAllCollisionParts(pBattan);
    }

    // fn_8008C570: true while a counter-attack window is open.
    bool isAttackPhaseStart(const BattanKing* pBattan) {
        if (pBattan->mFlagA != 0) {
            return false;
        }
        if (pBattan->isNerve(&NrvBattanKing::NrvE98::sInstance) || pBattan->isNerve(&NrvBattanKing::NrvE9C::sInstance)) {
            return true;
        }
        return pBattan->isNerve(&NrvBattanKing::NrvEA0::sInstance) && MR::isLessStep(pBattan, 0x28);
    }

    // fn_8008C610: advance the attack sequence; returns true when a new phase was started.
    bool tryAdvanceAttack(BattanKing* pBattan) {
        if (!isAttackPhaseStart(pBattan)) {
            return false;
        }
        s32 phase = pBattan->mPhase;
        pBattan->mFlagA = 1;
        pBattan->mPhase = phase + 1;
        updateJointVisibility(pBattan);
        if (pBattan->mPhase < 3) {
            pBattan->setNerve(&NrvBattanKing::NrvEA4::sInstance);
            MR::startAction(pBattan, "AttackJump");
            return true;
        }
        MR::tryDeleteEffect(pBattan, "AttackSign");
        pBattan->setNerve(&NrvBattanKing::NrvEA8::sInstance);
        MR::startAction(pBattan, "QuakeSign");
        return true;
    }
}

namespace {
    void hideAllCollisionParts(BattanKing* pBattan) {
        for (s32 i = 0; i < 5; i++) {
            pBattan->mCollisions[i]->kill();
            pBattan->mCollisionPresses[i]->kill();
        }
    }

    // fn_8008CA00: collision parts appear, press parts go away.
    void showCollisionPartsNormal(BattanKing* pBattan) {
        for (s32 i = 0; i < 5; i++) {
            pBattan->mCollisions[i]->appear();
            pBattan->mCollisionPresses[i]->kill();
        }
    }

    // fn_8008CA80: swapped visibility of showCollisionPartsNormal.
    void showCollisionPartsSwapped(BattanKing* pBattan) {
        for (s32 i = 0; i < 5; i++) {
            pBattan->mCollisions[i]->kill();
            pBattan->mCollisionPresses[i]->appear();
        }
    }

    // fn_8008CB00: normal visibility, then the arm/leg slots 3 and 4 are swapped.
    void updateCollisionPartsFootState(BattanKing* pBattan) {
        showCollisionPartsNormal(pBattan);
        pBattan->mCollisions[3]->kill();
        pBattan->mCollisions[4]->kill();
        pBattan->mCollisionPresses[3]->appear();
        pBattan->mCollisionPresses[4]->appear();
    }

    // fn_8008CB90: every collision part calls kill().
    void hideCollisionPartsAll(BattanKing* pBattan) {
        hideAllCollisionParts(pBattan);
    }

    // fn_8008CC10: true when any collision part (or the body) is standing on the player.
    bool isAnyCollisionOnActor(BattanKing* pBattan) {
        for (s32 i = 0; i < 5; i++) {
            if (MR::isOnPlayer(pBattan->mCollisions[i])) {
                return true;
            }
            if (MR::isOnPlayer(pBattan->mCollisionPresses[i])) {
                return true;
            }
        }
        return MR::isOnPlayer(pBattan);
    }

    // fn_8008CCA0: set the body position, copy the front vector and recompute the base matrix.
    void setPositionAndFront(BattanKing* pBattan, const TVec3f& rPos) {
        MR::setPosition(pBattan, rPos);
        pBattan->mFront = pBattan->mUnk18C;
        pBattan->calcAndSetBaseMtx();
    }

    // fn_8008CCF0: copy the base matrix and apply the player base matrix.
    void updatePlayerBaseMtx(BattanKing* pBattan) {
        TMtx34f mtx;
        PSMTXCopy(pBattan->mMtxFx, mtx);
        MR::addTransMtxLocal(mtx, BattanKingVec::sOffset914);
        MR::rotateMtxLocalYDegree(mtx, 40.0f);
        MR::setPlayerBaseMtx(mtx);
    }

    // fn_8008C870: show exactly the joint for the current phase, hide the rest.
    void updateJointVisibility(BattanKing* pBattan) {
        MR::hideJoint(pBattan, "Back1");
        MR::hideJoint(pBattan, "Back2");
        MR::hideJoint(pBattan, "Back3");
        if (pBattan->mPhase == 0) {
            MR::showJoint(pBattan, "Back1");
        }
        if (pBattan->mPhase == 1) {
            MR::showJoint(pBattan, "Back2");
        }
        if (pBattan->mPhase == 2) {
            MR::showJoint(pBattan, "Back3");
        }
        if (pBattan->mPhase == 3) {
            MR::showJoint(pBattan, "Back3");
        }
    }

    // fn_8008CF70: set the attack type and start the matching animation.
    void updateAttackAnim(BattanKing* pBattan, s32 phase) {
        pBattan->mAttackType = phase;
        if (MR::isOnGroundPlayer() && !isAnyCollisionOnActor(pBattan)) {
            MR::startAction(pBattan, "攻撃・プレイヤー地上");
        } else {
            MR::startAction(pBattan, "攻撃・プレイヤー空中");
        }
    }
}

// fn_8008CD40: per-id validity of the collision parts (used by the follow-matrix validater).
bool BattanKing::isValidCollisionState(s32 id) const {
    switch (id) {
    case 0:
        if ((isNerve(&NrvBattanKing::NrvE94::sInstance) && MR::isGreaterEqualStep(this, 0x3D)) ||
            isNerve(&NrvBattanKing::NrvE98::sInstance) || isNerve(&NrvBattanKing::NrvE9C::sInstance) ||
            isNerve(&NrvBattanKing::NrvEA4::sInstance)) {
            return true;
        }
        return false;
    case 1:
        if (!isNerve(&NrvBattanKing::NrvE68::sInstance) && !isNerve(&NrvBattanKing::NrvE6C::sInstance) &&
            !isNerve(&NrvBattanKing::NrvEA8::sInstance) && !isNerve(&NrvBattanKing::NrvEAC::sInstance)) {
            return true;
        }
        return false;
    case 2:
        if ((!isNerve(&NrvBattanKing::NrvE94::sInstance) || !MR::isGreaterEqualStep(this, 0x3D)) &&
            !isNerve(&NrvBattanKing::NrvE98::sInstance) && !isNerve(&NrvBattanKing::NrvE9C::sInstance) &&
            !isNerve(&NrvBattanKing::NrvEA4::sInstance) && !isNerve(&NrvBattanKing::NrvE68::sInstance) &&
            !isNerve(&NrvBattanKing::NrvE6C::sInstance) && !isNerve(&NrvBattanKing::NrvEA8::sInstance) &&
            !isNerve(&NrvBattanKing::NrvEAC::sInstance)) {
            return true;
        }
        return false;
    default:
        return false;
    }
}

// fn_8008CF10: a triangle is invalid when its sensor's host is one of this boss's collision parts.
// NONMATCHING: the target reads the sensor at Triangle+0xC8; our Triangle::getSensor() is at 0x8.
bool BattanKing::isInvalidTriangle(const Triangle* pTriangle) {
    LiveActor* pHost = pTriangle->getSensor()->mHost;
    for (s32 i = 0; i < 5; i++) {
        if (static_cast<LiveActor*>(mCollisions[i]) == pHost) {
            return true;
        }
    }
    return false;
}
