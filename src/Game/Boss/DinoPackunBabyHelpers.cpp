#include "Game/Boss/DinoPackunBaby.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/ActorCameraInfo.hpp"
#include "Game/LiveActor/ActorParam.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/Util/ActorCameraUtil.hpp"
#include "Game/Util/ActorStateUtil.hpp"
#include "Game/Util/DemoUtil.hpp"
#include "Game/Util/EffectUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"

void DinoPackunBaby::chasePlayer(const MR::ActorMoveParam* pParam, bool bAttack) {
    MR::moveAndTurnToPlayer(this, &mFront, pParam->_0, pParam->_4, pParam->_8, pParam->_C);
    if (bAttack) {
        MR::sendMsgEnemyAttackToBindedSensor(this, getSensor("Message"));
    }
}

void DinoPackunBaby::escapeFromPlayer(const MR::ActorMoveParam* pParam, bool bAttack) {
    MR::escapeFromPlayer(this, &mFront, pParam->_0, pParam->_4, pParam->_8, pParam->_C);
    if (bAttack) {
        MR::sendMsgEnemyAttackToBindedSensor(this, getSensor("Message"));
    }
}

bool DinoPackunBaby::updateJumpState(ActorStateBaseInterface* pState) {
    bool isHit = false;
    if (MR::isBinded(this) && MR::sendMsgEnemyAttackToBindedSensor(this, getSensor("Body"))) {
        isHit = true;
    }
    MR::updateActorState(pState);
    if (isHit) {
        return false;
    }
    return MR::isOnGround(this);
}

bool DinoPackunBaby::isInSight(const f32* pSight) {
    return MR::isInSightFanPlayer(this, mFront, pSight[0], pSight[1], pSight[2]);
}

bool DinoPackunBaby::isPlayerInFront() {
    TVec3f dir = *MR::getPlayerPos() - mPosition;
    MR::normalizeOrZero(&dir);
    return dir.dot(mFront) > 0.0f;
}

void DinoPackunBaby::requestMovementOnParts() {
    MR::requestMovementOn(mShellLeft);
    MR::requestMovementOn(mShellRight);
    MR::requestMovementOn(mShellHip);
    MR::requestMovementOn(mShellCenter);
    mShellLeft->requestMovementOnModel();
    mShellRight->requestMovementOnModel();
    mShellHip->requestMovementOnModel();
    mShellCenter->requestMovementOnModel();
    if (!MR::isDead(mPetal)) {
        MR::requestMovementOn(mPetal);
    }
    if (!MR::isDead(mTail)) {
        MR::requestMovementOn(mTail);
    }
}

void DinoPackunBaby::startActionShells(const char* pName) {
    MR::startAction(mShellLeft, pName);
    MR::startAction(mShellRight, pName);
    MR::startAction(mShellHip, pName);
    MR::startAction(mShellCenter, pName);
}

bool DinoPackunBaby::breakShellLeft(HitSensor* pSensor) {
    if (MR::isDead(mShellLeft)) {
        return false;
    }
    if (pSensor == nullptr || MR::isEqualSensor(pSensor, this, "ShellLeft")) {
        mShellLeft->kill();
        MR::invalidateHitSensor(this, "ShellLeft");
        return true;
    }
    return false;
}

bool DinoPackunBaby::breakShellRight(HitSensor* pSensor) {
    if (MR::isDead(mShellRight)) {
        return false;
    }
    if (pSensor == nullptr || MR::isEqualSensor(pSensor, this, "ShellRight")) {
        mShellRight->kill();
        MR::invalidateHitSensor(this, "ShellRight");
        return true;
    }
    return false;
}

bool DinoPackunBaby::breakShellHip(HitSensor* pSensor) {
    if (MR::isDead(mShellHip)) {
        return false;
    }
    if (pSensor == nullptr || MR::isEqualSensor(pSensor, this, "ShellHip")) {
        mShellHip->kill();
        mTail->appear();
        MR::invalidateHitSensor(this, "ShellHip");
        return true;
    }
    return false;
}

bool DinoPackunBaby::breakAllShells() {
    bool isLeft = breakShellLeft(nullptr);
    bool isRight = breakShellRight(nullptr);
    bool isHip = breakShellHip(nullptr);
    mTail->kill();
    if (!MR::isDead(mShellCenter)) {
        mShellCenter->kill();
    }
    if (isLeft || isRight || isHip) {
        return true;
    }
    return false;
}

void DinoPackunBaby::startBreakShells(const char* pName) {
    mShellLeft->startBreak(pName);
    mShellRight->startBreak(pName);
    mShellHip->startBreak(pName);
    mShellCenter->startBreak(pName);
}

void DinoPackunBaby::updateShellsMtx() {
    mShellLeft->updateModelMtx();
    mShellRight->updateModelMtx();
    mShellHip->updateModelMtx();
    mShellCenter->updateModelMtx();
}

void DinoPackunBaby::restoreShells() {
    mShellLeft->appear();
    mShellRight->appear();
    mShellHip->appear();
    mShellCenter->appear();
    mTail->kill();
    MR::validateHitSensor(this, "ShellLeft");
    MR::validateHitSensor(this, "ShellRight");
    MR::validateHitSensor(this, "ShellHip");
}

bool DinoPackunBaby::isShellsAllAlive() {
    if (!MR::isDead(mShellLeft) && !MR::isDead(mShellRight) && !MR::isDead(mShellHip)) {
        return true;
    }
    return false;
}

bool DinoPackunBaby::isShellLeftDead() {
    return MR::isDead(mShellLeft);
}

bool DinoPackunBaby::isShellRightDead() {
    return MR::isDead(mShellRight);
}

bool DinoPackunBaby::isShellHipDead() {
    return MR::isDead(mShellHip);
}

bool DinoPackunBaby::isShellsAllDead() {
    if (MR::isDead(mShellLeft) && MR::isDead(mShellRight) && MR::isDead(mShellHip)) {
        return true;
    }
    return false;
}

bool DinoPackunBaby::isShellSensor(HitSensor* pSensor) {
    if (MR::isEqualSensor(pSensor, this, "ShellLeft") || MR::isEqualSensor(pSensor, this, "ShellRight") ||
        MR::isEqualSensor(pSensor, this, "ShellHip") || MR::isEqualSensor(pSensor, this, "ShellCenter")) {
        return true;
    }
    return false;
}

void DinoPackunBaby::appearPetalDownLand() {
    mPetal->makeActorAppeared();
    MR::startAction(mPetal, "DownLand");
    MR::hideMaterial(this, "DinoPackunBabyPetalMat1_v");
}

void DinoPackunBaby::appearPetalDeath() {
    mPetal->makeActorAppeared();
    MR::startAction(mPetal, "DemoDeath");
    MR::hideMaterial(this, "DinoPackunBabyPetalMat2_v");
}

#pragma dont_inline on
void DinoPackunBaby::killPetal() {
    mPetal->makeActorDead();
}
#pragma dont_inline off

#pragma dont_inline on
void DinoPackunBaby::hitReaction() {
    MR::zeroVelocity(this);
    mScaleController->startHitReaction();
}
#pragma dont_inline off

void DinoPackunBaby::initDemoCamera(const char* pName) {
    MR::initAnimCamera(this, mCameraInfo, pName);
}

bool DinoPackunBaby::tryStartDemoWithCamera(const char* pName, const char* pPlayerBck, s32 frame) {
    bool isStart;
    if (pPlayerBck != nullptr) {
        isStart = MR::tryStartDemoMarioPuppetable(this, pName);
    } else {
        isStart = MR::tryStartDemo(this, pName);
    }
    if (isStart) {
        MR::zeroVelocity(this);
        TPos3f pos;
        MR::findNamePos("ベビーディノパックンデモ位置", pos);
        mFront.set<f32>(pos.mMtx[0][2], pos.mMtx[1][2], pos.mMtx[2][2]);
        MR::setPosition(this, pos.mMtx[0][3], pos.mMtx[1][3], pos.mMtx[2][3]);
        if (pPlayerBck != nullptr) {
            MR::setPlayerPos("ベビーディノパックンデモ位置");
        } else {
            MR::setPlayerPos("プレイヤーデモ位置（ベビーディノパックン戦）");
        }
        MR::startAnimCameraTargetSelf(this, mCameraInfo, pName, frame, false, 1.0f);
        return true;
    }
    return false;
}

bool DinoPackunBaby::tryEndDemoWithCamera(const char* pName, bool isPuppetEnd) {
    bool isEnd;
    if (isPuppetEnd) {
        isEnd = MR::tryEndDemoAndAnimCamera(this, mCameraInfo, pName, 0);
    } else {
        isEnd = MR::endDemoAndAnimCameraIfActionEnd(this, mCameraInfo, pName, 0);
    }
    if (isEnd) {
        if (MR::isDemoEndWaitCameraInterpolating()) {
            MR::tryForceDeleteEffectAll(mShellLeft);
            MR::tryForceDeleteEffectAll(mShellRight);
            MR::tryForceDeleteEffectAll(mShellHip);
            MR::tryForceDeleteEffectAll(mShellCenter);
            MR::tryForceDeleteEffectAll(mPetal);
            MR::tryForceDeleteEffectAll(mTail);
            mShellLeft->killModel();
            mShellRight->killModel();
            mShellHip->killModel();
            mShellCenter->killModel();
        }
        return true;
    }
    return false;
}

void DinoPackunBaby::initMultiCamera(const char* pName) {
    MR::initMultiActorCameraNoInit(this, mCameraInfo, pName);
}

#pragma dont_inline on
void DinoPackunBaby::startMultiCamera(const char* pName) {
    MR::startMultiActorCameraTargetPlayer(this, mCameraInfo, pName, -1);
}
#pragma dont_inline off

#pragma dont_inline on
void DinoPackunBaby::endMultiCamera(const char* pName) {
    MR::endMultiActorCamera(this, mCameraInfo, pName, false, -1);
}
#pragma dont_inline off

