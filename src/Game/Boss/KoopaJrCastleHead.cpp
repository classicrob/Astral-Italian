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

namespace NrvKoopaJrCastleHead { KoopaJrCastleHeadNrvA KoopaJrCastleHeadNrvA::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleHead { KoopaJrCastleHeadNrvB KoopaJrCastleHeadNrvB::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleHead { KoopaJrCastleHeadNrvC KoopaJrCastleHeadNrvC::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleHead { KoopaJrCastleHeadNrvD KoopaJrCastleHeadNrvD::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleHead { KoopaJrCastleHeadNrvE KoopaJrCastleHeadNrvE::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleHead { KoopaJrCastleHeadNrvF KoopaJrCastleHeadNrvF::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleHead { KoopaJrCastleHeadNrvG KoopaJrCastleHeadNrvG::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleHead { KoopaJrCastleHeadNrvH KoopaJrCastleHeadNrvH::sInstance ATTRIBUTE_WEAK; }



KoopaJrCastleHead::KoopaJrCastleHead(LiveActor* pHost, const char* pName) : PartsModel(pHost, pName, "KoopaJrCastleHead", 0, 0x13, false) {
    mCannonJointCtrl = nullptr;
    mForceJointCallback = true;
    PSMTXIdentity(mCannonMtx);
}



// fn_800D5440: head init: joint controller on "Cannon1", inverse rest matrix, then the 'Wait' action.
// NONMATCHING: target builds a derived JointController (vtable lbl_80674F10 with ptmf callback fields); base-class version emitted
void KoopaJrCastleHead::init(const JMapInfoIter& rIter) {
    mCannonJointCtrl = MR::createJointDelegatorWithNullChildFunc(this, &KoopaJrCastleHead::calcJointCannon, "Cannon1");
    MR::initJointTransform(this);
    MtxPtr rootMtx = MR::getJointMtx(this, "JointRoot");
    Mtx inv;
    PSMTXInverse(rootMtx, inv);
    PSMTXConcat(inv, MR::getJointMtx(this, "Cannon1"), mCannonMtx);
    PartsModel::init(rIter);
    initNerve(&NrvKoopaJrCastleHead::KoopaJrCastleHeadNrvD::sInstance, 0);
    MR::startAction(this, "Wait");
    makeActorAppeared();
    MR::invalidateHitSensor(this, "Panic");
    mForceJointCallback = false;
}

// fn_800D55B0: forwards an attack-sensor pair to the host unless the actor is in its C nerve.
void KoopaJrCastleHead::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (!isNerve(&NrvKoopaJrCastleHead::KoopaJrCastleHeadNrvC::sInstance)) {
        LiveActor* pHost = mHost;
        pHost->attackSensor(pSender, pReceiver);
    }
}

// fn_800D5620: forwards receiveMsgPlayerAttack to the host unless the actor is in its C nerve.
bool KoopaJrCastleHead::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(&NrvKoopaJrCastleHead::KoopaJrCastleHeadNrvC::sInstance)) {
        return false;
    }
    LiveActor* pHost = mHost;
    return pHost->receiveMsgPlayerAttack(msg, pSender, pReceiver);
}

// fn_800D56A0: forwards receiveOtherMsg to the host unless the actor is in its C nerve.
bool KoopaJrCastleHead::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(&NrvKoopaJrCastleHead::KoopaJrCastleHeadNrvC::sInstance)) {
        return false;
    }
    LiveActor* pHost = mHost;
    return pHost->receiveOtherMsg(msg, pSender, pReceiver);
}

namespace KoopaJrCastleFunction {
    MtxPtr getCannon3Mtx(LiveActor* pHead) {
        return MR::getJointMtx(pHead, "Cannon3");
    }
};  // namespace KoopaJrCastleFunction

void KoopaJrCastleHead::setNerveA() {
    setNerve(&NrvKoopaJrCastleHead::KoopaJrCastleHeadNrvA::sInstance);
}

void KoopaJrCastleHead::setNerveB() {
    setNerve(&NrvKoopaJrCastleHead::KoopaJrCastleHeadNrvB::sInstance);
}

void KoopaJrCastleHead::setNerveC() {
    setNerve(&NrvKoopaJrCastleHead::KoopaJrCastleHeadNrvC::sInstance);
}

// breakCastleCapsule: break the capsule (Break + Wait animations), then invalidate its collision.
void breakCastleCapsule(LiveActor* pActor) {
    MR::invalidateCollisionParts(pActor);
    KoopaJrCastle* pCastle = KoopaJrCastleFunction::getCastle();
    MR::startBva(pCastle->mCapsule, "Break");
    MR::startBrk(pCastle->mCapsule, "Wait");
    MR::tryHitReactionBreak(pCastle->mCapsule);
    MR::invalidateCollisionParts(pCastle->mCapsule);
}

void KoopaJrCastleHead::setNerveD() {
    setNerve(&NrvKoopaJrCastleHead::KoopaJrCastleHeadNrvD::sInstance);
}

void KoopaJrCastleHead::resetWaitUnlessDamaged() {
    if (isNerve(&NrvKoopaJrCastleHead::KoopaJrCastleHeadNrvB::sInstance)) {
        return;
    }
    if (isNerve(&NrvKoopaJrCastleHead::KoopaJrCastleHeadNrvC::sInstance)) {
        return;
    }
    setNerve(&NrvKoopaJrCastleHead::KoopaJrCastleHeadNrvA::sInstance);
}

bool KoopaJrCastleHead::tryStartPanic() {
    if (canStartAction()) {
        setNerve(&NrvKoopaJrCastleHead::KoopaJrCastleHeadNrvE::sInstance);
        return true;
    }
    return false;
}

bool KoopaJrCastleHead::tryStartSpinSign() {
    if (canStartAction()) {
        setNerve(&NrvKoopaJrCastleHead::KoopaJrCastleHeadNrvG::sInstance);
        return true;
    }
    return false;
}

bool KoopaJrCastleHead::isPanicOrSpin() const {
    bool ret = false;
    if (isNerve(&NrvKoopaJrCastleHead::KoopaJrCastleHeadNrvE::sInstance) || isNerve(&NrvKoopaJrCastleHead::KoopaJrCastleHeadNrvF::sInstance) ||
        isNerve(&NrvKoopaJrCastleHead::KoopaJrCastleHeadNrvG::sInstance) || isNerve(&NrvKoopaJrCastleHead::KoopaJrCastleHeadNrvH::sInstance)) {
        ret = true;
    }
    return ret;
}

// fn_800D5960: first-step setup of the head's 'Wait' state (castle capsule and body follow).
// NONMATCHING: string table addressing: target uses lbl_80674E30 + offset (addi r4,r30,52), literals here
void KoopaJrCastleHead::exe800D5960() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Wait");
        MR::deleteEffect(this, "Spin");
        MR::invalidateHitSensor(this, "Panic");
        KoopaJrCastle* pCastle = KoopaJrCastleFunction::getCastle();
        MR::startBrk(pCastle->mCapsule, "Wait");
        MR::startAction(pCastle->mKoopaJr, "Wait");
    }
}

// fn_800D59F0: first-step setup of the head's 1st damage state.
void KoopaJrCastleHead::exe800D59F0() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "1stDamage");
        KoopaJrCastle* pCastle = KoopaJrCastleFunction::getCastle();
        MR::startAction(pCastle->mKoopaJr, "Damage");
        MR::startBva(pCastle->mCapsule, "Crack1");
        MR::startBrk(pCastle->mCapsule, "Wait");
        MR::tryHitReaction(pCastle->mCapsule, "\x82\xd0\x82\xd1\x8a\x84\x82\xea");
    }
}

// fn_800D5A80: first-step setup of the head's 2nd damage state.
void KoopaJrCastleHead::exe800D5A80() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "2ndDamage");
        KoopaJrCastle* pCastle = KoopaJrCastleFunction::getCastle();
        MR::startAction(pCastle->mKoopaJr, "Damage");
        MR::startBva(pCastle->mCapsule, "Crack2");
        MR::startBrk(pCastle->mCapsule, "Wait");
        MR::tryHitReaction(pCastle->mCapsule, "\x82\xd0\x82\xd1\x8a\x84\x82\xea");
    }
}

// fn_800D5B10: 'Wait' phase of the head: animates, tracks the player, and returns to nerve D when the player leaves.
// NONMATCHING: string table addressing: target uses lbl_80674E30 + offset, literals here
void KoopaJrCastleHead::exe800D5B10() {
    KoopaJrCastle* pCastle = KoopaJrCastleFunction::getCastle();
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Wait");
        MR::startAction(pCastle->mKoopaJr, "PanicStart");
    }
    if (MR::isBckOneTimeAndStopped(pCastle->mKoopaJr)) {
        MR::startAction(pCastle->mKoopaJr, "Panic");
    }
    if (!KoopaJrCastleFunction::isPlayerOnCastle() && MR::isOnGroundPlayer()) {
        MR::startAction(pCastle->mKoopaJr, "Wait");
        setNerve(&NrvKoopaJrCastleHead::KoopaJrCastleHeadNrvD::sInstance);
        return;
    }
    KoopaJrCastleFunction::fn_800D5040(0.4f);
    if (MR::isGreaterEqualStep(this, 0x78)) {
        setNerve(&NrvKoopaJrCastleHead::KoopaJrCastleHeadNrvF::sInstance);
    }
}

// fn_800D5BF0: spin-sign sequence and the F/G/H follow-up of the head's panic phase.
// NONMATCHING: string table addressing: target uses lbl_80674E30 + offset, literals here
void KoopaJrCastleHead::exe800D5BF0() {
    if (MR::isFirstStep(this)) {
        KoopaJrCastle* pCastle = KoopaJrCastleFunction::getCastle();
        MR::startAction(this, "SpinSign");
        MR::startAction(pCastle->mKoopaJr, "SpinSign");
        MR::startBrk(pCastle->mCapsule, "SpinSign");
    }
    MR::startActionSound(this, "EmLvWaterbazPreStorm", -1, -1, -1);
    if (isNerve(&NrvKoopaJrCastleHead::KoopaJrCastleHeadNrvF::sInstance)) {
        if (!KoopaJrCastleFunction::isPlayerOnCastle() && MR::isOnGroundPlayer()) {
            setNerve(&NrvKoopaJrCastleHead::KoopaJrCastleHeadNrvD::sInstance);
            return;
        }
    }
    KoopaJrCastleFunction::fn_800D5040(0.4f);
    if (isNerve(&NrvKoopaJrCastleHead::KoopaJrCastleHeadNrvG::sInstance)) {
        if (MR::isGreaterStep(this, 0x59)) {
            setNerve(&NrvKoopaJrCastleHead::KoopaJrCastleHeadNrvH::sInstance);
        }
    } else if (MR::isGreaterStep(this, 0x78)) {
        setNerve(&NrvKoopaJrCastleHead::KoopaJrCastleHeadNrvH::sInstance);
    }
}

// fn_800D5D20: 'E' phase: eases the head's rest matrix back over the nerve step, with the burst sound.
// NONMATCHING: rest-matrix and rotation axis not recovered (target passes an unset register to rotateVecDegree)
// NONMATCHING: register/scheduling only (target keeps &_A4 in r31)
void KoopaJrCastleHead::exe800D5D20() {
    if (MR::isFirstStep(this)) {
        KoopaJrCastle* pCastle = KoopaJrCastleFunction::getCastle();
        MR::startAction(this, "Spin");
        MR::startAction(pCastle->mKoopaJr, "Spin");
        MR::validateHitSensor(this, "Panic");
    }
    if (MR::isStep(this, 0x5A)) {
        MR::invalidateHitSensor(this, "Panic");
    }
    TVec3f pos;
    pos.set< f32 >(mCannonMtx[0][3], mCannonMtx[1][3], mCannonMtx[2][3]);
    TVec3f side;
    side.set< f32 >(mCannonMtx[0][0], mCannonMtx[1][0], mCannonMtx[2][0]);
    f32 ease = MR::calcNerveEaseOutValue(this, 0x96, 25.0f, 0.0f);
    MR::rotateVecDegree(&side, mGravity, ease);
    TVec3f up = -mGravity;
    MR::makeMtxSideUpPos(reinterpret_cast< TPos3f* >(&mCannonMtx), side, up, pos);
    MR::startActionSound(this, "EmLvWaterbazStorm", static_cast<s32>(100.0f * ease / 25.0f), -1, -1);
    if (MR::isGreaterStep(this, 0x96)) {
        setNerve(&NrvKoopaJrCastleHead::KoopaJrCastleHeadNrvD::sInstance);
    }
}

// canStartAction: true when the head is idle enough to take a new action (not in A/B/C and not in the E..H group).
bool KoopaJrCastleHead::canStartAction() {
    if (isNerve(&NrvKoopaJrCastleHead::KoopaJrCastleHeadNrvA::sInstance) || isNerve(&NrvKoopaJrCastleHead::KoopaJrCastleHeadNrvB::sInstance) ||
        isNerve(&NrvKoopaJrCastleHead::KoopaJrCastleHeadNrvC::sInstance)) {
        return false;
    }
    return !isPanicOrSpin();
}

// fn_800D5F10: refreshes the base matrix and registers the joint controller callback.
void KoopaJrCastleHead::calcAndSetBaseMtx() {
    PartsModel::calcAndSetBaseMtx();
    mCannonJointCtrl->registerCallBack();
}

// fn_800D5F50: writes the head's pose matrix from the given matrix's translation (idle head only).
// NONMATCHING: joint concat order and setMtxTrans argument shape inferred from the m2c read only
bool KoopaJrCastleHead::calcJointCannon(TPos3f* pMtx, const JointControllerInfo&) {
    if (!mForceJointCallback && isNerve(&NrvKoopaJrCastleHead::KoopaJrCastleHeadNrvC::sInstance)) {
        return false;
    }
    TPos3f mtx;
    PSMTXIdentity(mtx);
    PSMTXConcat(MR::getJointMtx(this, "JointRoot"), mCannonMtx, mtx);
    TVec3f trans(0.0f);
    MR::extractMtxTrans(pMtx->toMtxPtr(), &trans);
    MR::setMtxTrans(mtx, trans.x, trans.y, trans.z);
    pMtx->setInline(mtx);
    return true;
}


KoopaJrCastleHead::~KoopaJrCastleHead() {}
