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


namespace {
    extern const f32 sMaxMoveSpeed;
    extern const f32 sCenterRadius;
}

namespace NrvKoopaJrCastleStateBodyMove { KoopaJrCastleStateBodyMoveNrvSel KoopaJrCastleStateBodyMoveNrvSel::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleStateBodyMove { KoopaJrCastleStateBodyMoveNrvTurnA KoopaJrCastleStateBodyMoveNrvTurnA::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleStateBodyMove { KoopaJrCastleStateBodyMoveNrvTurnB KoopaJrCastleStateBodyMoveNrvTurnB::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleStateBodyMove { KoopaJrCastleStateBodyMoveNrvMoveA KoopaJrCastleStateBodyMoveNrvMoveA::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleStateBodyMove { KoopaJrCastleStateBodyMoveNrvRotA KoopaJrCastleStateBodyMoveNrvRotA::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleStateBodyMove { KoopaJrCastleStateBodyMoveNrvRotB KoopaJrCastleStateBodyMoveNrvRotB::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleStateBodyMove { KoopaJrCastleStateBodyMoveNrvRotC KoopaJrCastleStateBodyMoveNrvRotC::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleStateBodyMove { KoopaJrCastleStateBodyMoveNrvMoveB KoopaJrCastleStateBodyMoveNrvMoveB::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleStateBodyMove { KoopaJrCastleStateBodyMoveNrvStop KoopaJrCastleStateBodyMoveNrvStop::sInstance ATTRIBUTE_WEAK; }

KoopaJrCastleStateBodyMove::KoopaJrCastleStateBodyMove(KoopaJrCastleBody* pHost) : ActorStateBase< KoopaJrCastleBody >("\x88\xda\x93\xae", pHost) {
    mDirection.x = 0.0f;
    mDirection.y = 0.0f;
    mDirection.z = 0.0f;
    mTurnRate = 0.0f;
    mMode = 0;
    mTurnDirection = 1.0f;
    mHasCompletedTurn = 0;
}

KoopaJrCastleStateBodyMove::~KoopaJrCastleStateBodyMove() {}


void KoopaJrCastleStateBodyMove::init() {
    initNerve(&NrvKoopaJrCastleStateBodyMove::KoopaJrCastleStateBodyMoveNrvSel::sInstance, 0);
    kill();
    mDirection = mHost->mPosition;
    mMode = 2;
}

void KoopaJrCastleStateBodyMove::appear() {
    mIsDead = false;
    setNerve(&NrvKoopaJrCastleStateBodyMove::KoopaJrCastleStateBodyMoveNrvSel::sInstance);
    mHost->validateCollisionA();
    KoopaJrCastleFunction::validateBodyAndHeadSensors();
    mTurnDirection = 1.0f;
    mMoveStep = 0;
    mHasCompletedTurn = 0;
}

void KoopaJrCastleStateBodyMove::setMode0() {
    mMode = 0;
}

void KoopaJrCastleStateBodyMove::setMode1() {
    mMode = 1;
}

void KoopaJrCastleStateBodyMove::setMode2() {
    mMode = 2;
}

void KoopaJrCastleStateBodyMove::kill() {
    MR::zeroVelocity(KoopaJrCastleFunction::getCastle());
    mIsDead = true;
    mHost->validateCollisionB();
    MR::setBckRate(mHost, 1.0f);
    MR::setBtkRate(mHost, 0.0f);
}

bool KoopaJrCastleStateBodyMove::isMoving() const {
    bool ret = false;
    if (isNerve(&NrvKoopaJrCastleStateBodyMove::KoopaJrCastleStateBodyMoveNrvSel::sInstance) ||
        isNerve(&NrvKoopaJrCastleStateBodyMove::KoopaJrCastleStateBodyMoveNrvTurnA::sInstance)) {
        ret = true;
    }
    return ret;
}

bool KoopaJrCastleStateBodyMove::getFlag() const {
    return mHasCompletedTurn;
}

void KoopaJrCastleStateBodyMove::exeSelectMove() {
    mHost->validateCollisionA();
    if (MR::isFirstStep(this)) {
        MR::startAction(mHost, "MoveStart");
    }
    if (MR::isGreaterStep(this, 90)) {
        if (mMode == 1) {
            if (MR::calcDistanceToPlayerH(mHost) < 1000.0f) {
                setNerve(&NrvKoopaJrCastleStateBodyMove::KoopaJrCastleStateBodyMoveNrvRotA::sInstance);
            } else {
                setNerve(&NrvKoopaJrCastleStateBodyMove::KoopaJrCastleStateBodyMoveNrvTurnA::sInstance);
            }
        } else if (mMode == 2) {
            mTurnDirection = MR::isPlayerLeftSide(mHost) ? -1.0f : 1.0f;
            if (isOutsideCenter()) {
                setNerve(&NrvKoopaJrCastleStateBodyMove::KoopaJrCastleStateBodyMoveNrvRotA::sInstance);
            } else {
                setNerve(&NrvKoopaJrCastleStateBodyMove::KoopaJrCastleStateBodyMoveNrvTurnB::sInstance);
            }
        } else if (mMode == 0) {
            setNerve(&NrvKoopaJrCastleStateBodyMove::KoopaJrCastleStateBodyMoveNrvTurnA::sInstance);
        }
    }
}

// NONMATCHING: clamp floating-point register/order differences; literal pool ordering reported
void KoopaJrCastleStateBodyMove::exeTurnToPlayer() {
    KoopaJrCastle* pCastle = KoopaJrCastleFunction::getCastle();
    if (MR::isFirstStep(this)) {
        mTurnRate = 0.0f;
        KoopaJrCastleFunction::fireBarOff();
    }
    if (MR::isPlayerLeftSide(mHost)) {
        if (startBtkKeepFrame("MoveR")) {
            MR::setBtkRate(mHost, 1.0f);
        }
    } else {
        if (startBtkKeepFrame("MoveL")) {
            MR::setBtkRate(mHost, 1.0f);
        }
    }
    MR::startActionSound(mHost, "MoveRotate", -1, -1, -1);
    mTurnRate += 0.025f;
    f32 rate = MR::clamp(mTurnRate, 0.0f, 0.5f);
    mTurnRate = rate;
    if (MR::turnDirectionToPlayerDegreeHorizon(pCastle, &pCastle->mFrontVec, rate)) {
        KoopaJrCastleFunction::fireBarOn();
        setNerve(&NrvKoopaJrCastleStateBodyMove::KoopaJrCastleStateBodyMoveNrvTurnB::sInstance);
        return;
    }
    if (MR::isGreaterStep(this, 150)) {
        KoopaJrCastleFunction::fireBarOn();
        setNerve(&NrvKoopaJrCastleStateBodyMove::KoopaJrCastleStateBodyMoveNrvTurnB::sInstance);
        return;
    }
    setMoveRate(MR::calcNerveEaseOutValue(this, 10, 0.1f, 1.8f));
    mHost->validateCollisionA();
}

void KoopaJrCastleStateBodyMove::exeMoveStart() {
    if (MR::isFirstStep(this)) {
        startBtkKeepFrame("MoveLR");
    }
    setMoveRate(MR::calcNerveEaseOutValue(this, 10, 0.1f, 1.8f));
    if (MR::isGreaterStep(this, 10)) {
        if (static_cast< u32 >(mMode - 1) <= 1) {
            setNerve(&NrvKoopaJrCastleStateBodyMove::KoopaJrCastleStateBodyMoveNrvMoveB::sInstance);
        } else {
            setNerve(&NrvKoopaJrCastleStateBodyMove::KoopaJrCastleStateBodyMoveNrvMoveA::sInstance);
        }
    }
    mHost->validateCollisionA();
}

void KoopaJrCastleStateBodyMove::exeMoveStraight() {
    setMoveRate(MR::calcNerveEaseOutValue(this, 60, 0.1f, 1.8f));
    mHost->startMoveSound();
    KoopaJrCastle* pCastle = KoopaJrCastleFunction::getCastle();
    TVec3f vel(pCastle->mFrontVec);
    vel *= 0.7f;
    MR::addVelocity(pCastle, vel);
    if (MR::calcVelocityLength(pCastle) > 14.0f) {
        MR::normalize(&pCastle->mVelocity);
        MR::attenuateVelocity(pCastle, 14.0f);
    }
    MR::attenuateVelocity(pCastle, 0.93f);
    if (isTurnEnd(0.7f)) {
        MR::zeroVelocity(pCastle);
        setNerve(&NrvKoopaJrCastleStateBodyMove::KoopaJrCastleStateBodyMoveNrvStop::sInstance);
    }
    if (MR::isGreaterStep(this, 180) || MR::isPlayerDamaging()) {
        MR::killVelocityVertical(pCastle);
        setNerve(&NrvKoopaJrCastleStateBodyMove::KoopaJrCastleStateBodyMoveNrvStop::sInstance);
    }
    mHost->validateCollisionA();
}

static inline void limitSpeed(LiveActor* pActor, f32 lim) {
    if (MR::calcVelocityLength(pActor) > lim) {
        MR::normalize(&pActor->mVelocity);
        MR::attenuateVelocity(pActor, lim);
    } else if (MR::calcVelocityLength(pActor) < -lim) {
        MR::normalize(&pActor->mVelocity);
        MR::attenuateVelocity(pActor, -lim);
    }
}

// NONMATCHING: tail branch layout.
void KoopaJrCastleStateBodyMove::exeMoveChase() {
    setMoveRate(MR::calcNerveEaseOutValue(this, 60, 0.1f, 1.8f));
    mHost->startMoveSound();
    KoopaJrCastle* pCastle = KoopaJrCastleFunction::getCastle();
    TVec3f vel(pCastle->mFrontVec);
    vel *= 1.0f;
    MR::addVelocity(pCastle, vel);
    limitSpeed(pCastle, sMaxMoveSpeed);
    MR::attenuateVelocity(pCastle, 0.93f);
    mHost->validateCollisionA();
    if (mMode == 2) {
        if (++mMoveStep > 900 || MR::isPlayerDamaging()) {
            MR::killVelocityVertical(pCastle);
            setNerve(&NrvKoopaJrCastleStateBodyMove::KoopaJrCastleStateBodyMoveNrvStop::sInstance);
        } else if (isTurnEnd(1.0f)) {
            MR::zeroVelocity(pCastle);
            setNerve(&NrvKoopaJrCastleStateBodyMove::KoopaJrCastleStateBodyMoveNrvRotA::sInstance);
        }
    } else {
        if (isTurnEnd(1.0f) || MR::isPlayerDamaging()) {
            MR::zeroVelocity(pCastle);
            setNerve(&NrvKoopaJrCastleStateBodyMove::KoopaJrCastleStateBodyMoveNrvStop::sInstance);
        }
        if (MR::isGreaterStep(this, 540) || MR::isPlayerDamaging()) {
            MR::killVelocityVertical(pCastle);
            setNerve(&NrvKoopaJrCastleStateBodyMove::KoopaJrCastleStateBodyMoveNrvStop::sInstance);
        }
        mHost->validateCollisionA();
    }
}

// NONMATCHING: vector-copy argument load order; literal pool ordering reported
void KoopaJrCastleStateBodyMove::exeTurnAroundCenter() {
    KoopaJrCastle* pCastle = KoopaJrCastleFunction::getCastle();
    TVec3f target(0.0f);
    calcTargetDir(&target);
    setMoveRate(MR::calcNerveEaseOutValue(this, 10, 0.1f, 1.8f));
    TVec3f axis((-pCastle->mGravity) * mTurnDirection);
    f32 angle = MR::turnVecToVecDegree(&pCastle->mFrontVec, pCastle->mFrontVec, target, 0.5f, axis);
    if (mTurnDirection > 0.0f) {
        if (startBtkKeepFrame("MoveR")) {
            MR::setBtkRate(mHost, 1.0f);
        }
    } else {
        if (startBtkKeepFrame("MoveL")) {
            MR::setBtkRate(mHost, 1.0f);
        }
    }
    MR::startActionSound(mHost, "MoveRotate", -1, -1, -1);
    if (angle < 4.0f) {
        setNerve(&NrvKoopaJrCastleStateBodyMove::KoopaJrCastleStateBodyMoveNrvRotB::sInstance);
    } else if (mMoveStep > 900 || MR::isPlayerDamaging()) {
        ++mMoveStep;
        setNerve(&NrvKoopaJrCastleStateBodyMove::KoopaJrCastleStateBodyMoveNrvStop::sInstance);
    } else {
        ++mMoveStep;
        mHost->validateCollisionA();
    }
}

// NONMATCHING: stack slot layout of TVec3f temporaries
void KoopaJrCastleStateBodyMove::exeMoveAroundCenter() {
    if (MR::isFirstStep(this)) {
        startBtkKeepFrame("MoveLR");
    }
    KoopaJrCastle* pCastle = KoopaJrCastleFunction::getCastle();
    setMoveRate(MR::calcNerveEaseOutValue(this, 60, 0.1f, 1.8f));
    mHost->startMoveSound();
    TVec3f target(0.0f);
    calcTargetDir(&target);
    TVec3f axis((-pCastle->mGravity) * mTurnDirection);
    if (MR::turnVecToVecDegree(&pCastle->mFrontVec, pCastle->mFrontVec, target, 0.5f, axis) < 4.0f) {
        KoopaJrCastle* pCastle2 = KoopaJrCastleFunction::getCastle();
        TVec3f vel(pCastle2->mFrontVec);
        vel *= 1.0f;
        MR::addVelocity(pCastle2, vel);
        if (MR::calcVelocityLength(pCastle2) > 17.0f) {
            MR::normalize(&pCastle2->mVelocity);
            MR::attenuateVelocity(pCastle2, 14.0f);
        }
    }
    MR::attenuateVelocity(pCastle, 0.93f);
    if (++mMoveStep > 900 || MR::isPlayerDamaging()) {
        setNerve(&NrvKoopaJrCastleStateBodyMove::KoopaJrCastleStateBodyMoveNrvStop::sInstance);
    }
    if (MR::calcDistanceToPlayerH(mHost) < 4000.0f && MR::isFaceToPlayerDegree(mHost, pCastle->mFrontVec, 60.0f)) {
        setNerve(&NrvKoopaJrCastleStateBodyMove::KoopaJrCastleStateBodyMoveNrvStop::sInstance);
        mHasCompletedTurn = 1;
    } else {
        mHost->validateCollisionA();
    }
}

// NONMATCHING: stack slot layout of TVec3f temporaries
void KoopaJrCastleStateBodyMove::exeMoveBackAroundCenter() {
    if (MR::isFirstStep(this)) {
        startBtkKeepFrame("MoveBack");
    }
    KoopaJrCastle* pCastle = KoopaJrCastleFunction::getCastle();
    setMoveRate(MR::calcNerveEaseOutValue(this, 60, 0.1f, 1.8f));
    mHost->startMoveSound();
    TVec3f target(0.0f);
    calcTargetDir(&target);
    TVec3f axis((-pCastle->mGravity) * mTurnDirection);
    if (MR::turnVecToVecDegree(&pCastle->mFrontVec, pCastle->mFrontVec, target, 0.5f, axis) < 4.0f) {
        KoopaJrCastle* pCastle2 = KoopaJrCastleFunction::getCastle();
        TVec3f vel((-pCastle2->mFrontVec) * 1.0f);
        MR::addVelocity(pCastle2, vel);
        if (MR::calcVelocityLength(pCastle2) > 17.0f) {
            MR::normalize(&pCastle2->mVelocity);
            MR::attenuateVelocity(pCastle2, 14.0f);
        }
    }
    MR::attenuateVelocity(pCastle, 0.93f);
    if (++mMoveStep > 900 || MR::isPlayerDamaging()) {
        setNerve(&NrvKoopaJrCastleStateBodyMove::KoopaJrCastleStateBodyMoveNrvStop::sInstance);
    }
    mHost->validateCollisionA();
}

void KoopaJrCastleStateBodyMove::exeStop() {
    KoopaJrCastle* pCastle = KoopaJrCastleFunction::getCastle();
    setMoveRate(1.8f * (1.0f - MR::calcNerveEaseOutRate(this, 60)));
    mHost->startMoveSound();
    MR::attenuateVelocity(pCastle, 0.93f);
    if (MR::isGreaterStep(this, 60)) {
        MR::zeroVelocity(pCastle);
        kill();
    }
}

// NONMATCHING: stack slot layout
void KoopaJrCastleStateBodyMove::calcTargetDir(TVec3f* pDst) {
    KoopaJrCastle* pCastle = KoopaJrCastleFunction::getCastle();
    f32 dist;
    TVec3f dir;
    MR::separateScalarAndDirection(&dist, &dir, mDirection - pCastle->mPosition);
    TVec3f front(pCastle->mFrontVec);
    if (MR::isNearZero(dist, 0.001f)) {
        dir = pCastle->mFrontVec;
    }
    TPos3f mtx;
    MR::makeMtxUpSidePos(&mtx, -pCastle->mGravity, dir, pCastle->mPosition);
    MR::extractMtxZDir(mtx.toMtxPtr(), &front);
    TVec3f axis = -pCastle->mGravity;
    MR::rotateVecDegree(&front, axis, 7.0f * mTurnDirection);
    front *= mTurnDirection;
    *pDst = front;
}

bool KoopaJrCastleStateBodyMove::startBtkKeepFrame(const char* pName) {
    f32 frame = MR::getBtkFrame(mHost);
    bool ret = MR::tryStartActionNotPlaying(mHost, pName);
    if (ret) {
        frame = MR::getBtkFrameMax(mHost) - frame;
        MR::setBckFrame(mHost, frame);
        MR::setBtkFrame(mHost, frame);
    }
    return ret;
}

bool KoopaJrCastleStateBodyMove::isTurnEnd(f32 dir) {
    KoopaJrCastle* pCastle = KoopaJrCastleFunction::getCastle();
    f32 dist;
    TVec3f unit;
    MR::separateScalarAndDirection(&dist, &unit, mDirection - pCastle->mPosition);
    if (dist > 3200.0f) {
        if (dir > 0.0f) {
            if (unit.dot(pCastle->mFrontVec) < 0.0f) {
                return true;
            }
        } else if (unit.dot(pCastle->mFrontVec) > 0.0f) {
            return true;
        }
    }
    return false;
}

// NONMATCHING: floating-point register assignment and constant-load order
bool KoopaJrCastleStateBodyMove::isOutsideCenter() {
    f32 dist;
    TVec3f dir;
    MR::separateScalarAndDirection(&dist, &dir, mDirection - KoopaJrCastleFunction::getCastle()->mPosition);
    f32 centerRadius = sCenterRadius;
    f32 margin = 50.0f;
    return dist > centerRadius - margin;
}

void KoopaJrCastleStateBodyMove::setMoveRate(f32 rate) {
    mHost->mRate = rate;
    MR::setBckRate(mHost, rate);
    MR::setBtkRate(mHost, rate);
}

namespace {
    const f32 sMaxMoveSpeed = 17.0f;
    const f32 sCenterRadius = 3200.0f;
}
