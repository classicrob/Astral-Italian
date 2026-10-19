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

namespace NrvKoopaJrCastleStateBattle { KoopaJrCastleStateBattleNrvA KoopaJrCastleStateBattleNrvA::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleStateBattle { KoopaJrCastleStateBattleNrvB KoopaJrCastleStateBattleNrvB::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleStateBattle { KoopaJrCastleStateBattleNrvC KoopaJrCastleStateBattleNrvC::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleStateBattle { KoopaJrCastleStateBattleNrvD KoopaJrCastleStateBattleNrvD::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleStateBattle { KoopaJrCastleStateBattleNrvE KoopaJrCastleStateBattleNrvE::sInstance ATTRIBUTE_WEAK; }

KoopaJrCastleStateBattle::KoopaJrCastleStateBattle(KoopaJrCastle* pHost) : ActorStateBase< KoopaJrCastle >("Phase[\x90\xed\x93\xac]", pHost) {
    mPhaseState[0] = nullptr;
    mPhaseState[1] = nullptr;
    mPhaseState[2] = nullptr;
    mPhaseState[3] = nullptr;
    mPhase = 3;
}

void KoopaJrCastleStateBattle::init() {
    using namespace NrvKoopaJrCastleStateBattle;
    initNerve(&KoopaJrCastleStateBattleNrvA::sInstance, 16);
    mPhaseState[0] = new KoopaJrCastleStateBattle1(mHost);
    mPhaseState[1] = new KoopaJrCastleStateBattle2(mHost);
    mPhaseState[2] = new KoopaJrCastleStateBattle3(mHost);
    mPhaseState[3] = new KoopaJrCastleStateBattle4(mHost);
    MR::initActorState(this, mPhaseState[0], &KoopaJrCastleStateBattleNrvA::sInstance, "Attack1st");
    MR::initActorState(this, mPhaseState[1], &KoopaJrCastleStateBattleNrvB::sInstance, "Attack2nd");
    MR::initActorState(this, mPhaseState[2], &KoopaJrCastleStateBattleNrvC::sInstance, "Attack3rd");
    MR::initActorState(this, mPhaseState[3], &KoopaJrCastleStateBattleNrvD::sInstance, "Damage");
    MR::initAnimCamera(mHost->mBody, mHost->mCameraInfo, "DemoPowerUpKoopaJrCastle");
    kill();
}

void KoopaJrCastleStateBattle::appear() {
    mPhase = 3;
    mIsDead = false;
    setNerve(&NrvKoopaJrCastleStateBattle::KoopaJrCastleStateBattleNrvA::sInstance);
    KoopaJrCastleFunction::validateBodyAndHeadSensors();
}

void KoopaJrCastleStateBattle::attackSensor(HitSensor* sender, HitSensor* receiver) {
    if (MR::isSensorPlayer(receiver)) return;
    if (MR::isSensorType(sender, 0x35)) {
        if (MR::isSensorMapObj(receiver) || MR::isSensorReceiver(receiver))
            MR::sendArbitraryMsg(0xF4, receiver, sender);
    } else if (MR::isEqualSensor(sender, mHost->mHead, "BreakObject")) {
        if (MR::isSensorMapObj(receiver) || MR::isSensorReceiver(receiver))
            MR::sendArbitraryMsg(0xF4, receiver, sender);
    }
}

bool KoopaJrCastleStateBattle::receiveMsgPlayerAttack(u32 msg, HitSensor*, HitSensor* receiver) {
    using namespace NrvKoopaJrCastleStateBattle;
    if (MR::isMsgStarPieceReflect(msg)) return true;
    if (MR::isMsgPlayerHipDropFloor(msg) && KoopaJrCastleFunction::isCapsuleSensorHost(mHost, receiver)) {
        if (KoopaJrCastleFunction::isElectricActive()) return false;
        if (isNerve(&KoopaJrCastleStateBattleNrvA::sInstance) || isNerve(&KoopaJrCastleStateBattleNrvB::sInstance) || isNerve(&KoopaJrCastleStateBattleNrvC::sInstance)) {
            setNerve(&KoopaJrCastleStateBattleNrvD::sInstance);
            return true;
        }
        return false;
    }
    return false;
}

bool KoopaJrCastleStateBattle::receiveOtherMsg(u32 msg, HitSensor* sender, HitSensor* receiver) {
    if (MR::isMsgFloorTouch(msg) || MR::isMsgWallTouch(msg)) {
        KoopaJrCastleBody* body = mHost->mBody;
        if (MR::getSensorHost(receiver) == body) return false;
        if (KoopaJrCastleFunction::isElectricActive()) {
            MR::sendMsgEnemyAttackElectric(sender, receiver);
            return true;
        }
    }
    if (MR::isMsgAutoRushBegin(msg) && !MR::isPlayerHipDropFalling()) {
        if (KoopaJrCastleFunction::isPlayerOnHead() || KoopaJrCastleFunction::isPlayerOnCapsule())
            return KoopaJrCastleFunction::tryBlowPlayerToStart(MR::getSensorHost(receiver), sender);
        return false;
    }
    KoopaJrCastle* castle = KoopaJrCastleFunction::getCastle();
    if (!MR::isDead(castle->mDominate)) {
        if (MR::isMsgIsRushTakeOver(msg)) return true;
        if (MR::isMsgRushCancel(msg)) {
            castle->mDominate->kill();
            return true;
        }
        if (MR::isMsgUpdateBaseMtx(msg)) {
            if (castle->mDominate->mBoundActor) {
                castle->mDominate->updateBaseMtx();
                return true;
            }
            return false;
        }
    }
    return false;
}

bool KoopaJrCastleStateBattle::isDemo() const {
    return isNerve(&NrvKoopaJrCastleStateBattle::KoopaJrCastleStateBattleNrvC::sInstance);
}

void KoopaJrCastleStateBattle::exe800D68C0() {
    if (MR::isFirstStep(this)) KoopaJrCastleFunction::resetBattleParts();
    if (!mPhaseState[0]->mIsDead) MR::updateActorState(this);
}

void KoopaJrCastleStateBattle::exe800D6910() {
    if (MR::isFirstStep(this)) {
        KoopaJrCastleFunction::resetBattleParts();
        KoopaJrCastleFunction::appearCapsuleStarPiece();
    }
    if (!mPhaseState[1]->mIsDead) MR::updateActorState(this);
}

void KoopaJrCastleStateBattle::exe800D6970() {
    if (MR::isFirstStep(this)) {
        KoopaJrCastleFunction::resetBattleParts();
        KoopaJrCastleFunction::appearCapsuleStarPiece();
    }
    if (!mPhaseState[2]->mIsDead) MR::updateActorState(this);
}

// NONMATCHING: target switch lowering retains one unreachable branch
void KoopaJrCastleStateBattle::exe800D69D0() {
    using namespace NrvKoopaJrCastleStateBattle;
    if (mPhaseState[3]->mIsDead) {
        switch (mPhase) {
        case 1:
            if (KoopaJrCastleFunction::tryStartDemo("DemoPowerUpKoopaJrCastle", nullptr))
                setNerve(&KoopaJrCastleStateBattleNrvE::sInstance);
            break;
        }
        return;
    }
    if (MR::updateActorState(this)) {
        if (mPhase == 0) kill();
        else if (mPhase == 1) {
            if (KoopaJrCastleFunction::tryStartDemo("DemoPowerUpKoopaJrCastle", nullptr))
                setNerve(&KoopaJrCastleStateBattleNrvE::sInstance);
        } else if (mPhase == 2) setNerve(&KoopaJrCastleStateBattleNrvB::sInstance);
        else setNerve(&KoopaJrCastleStateBattleNrvA::sInstance);
    }
}

void KoopaJrCastleStateBattle::exe800D6AD0() {
    if (MR::isFirstStep(this)) {
        KoopaJrCastleFunction::startPartsDemoActions("DemoPowerUpKoopaJrCastle");
        mHost->mBody->setNerveFall();
        mHost->mFireBar->updateChildShadows();
    } else if (KoopaJrCastleFunction::tryEndDemo("DemoPowerUpKoopaJrCastle", nullptr, false)) {
        MR::overlayWithPreviousScreen(4);
        MR::startStageBGM("MBGM_SMG2_BOSS_06B", false);
        KoopaJrCastleFunction::resetBodyOffsetAndShadows();
        setNerve(&NrvKoopaJrCastleStateBattle::KoopaJrCastleStateBattleNrvC::sInstance);
    }
}

KoopaJrCastleStateBattle::~KoopaJrCastleStateBattle() {}
