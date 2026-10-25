#include <algorithm>
#ifdef GALAXY_NATIVE
#include <petari/game_compat.hpp>
#else
#include <functional.hpp>
#endif
#include "Game/Boss/Sandy.hpp"
#include "Game/Util.hpp"
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/Enemy/WalkerStateBindStarPointer.hpp"
#include "Game/LiveActor/ActorCameraInfo.hpp"
#include "Game/LiveActor/ActorJointCtrl.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/ModelObj.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/LiveActor/Spine.hpp"
#include "Game/Util/FixedPosition.hpp"
#include <cstdio>

// 800FDB10
// NONMATCHING: register allocation (string/new pointer registers swapped).
SandyMini* Sandy::createMini(s32 index, const TVec3f& rPos, const TVec3f& rRot) {
    const char* pName = index == 0 ? "\203T\203\223\203f\203B\201[\203q\203_\203\212\201[" : "\203T\203\223\203f\203B\201[\203~\203M\201[";
    SandyMini* pMini = new SandyMini(this, pName, new FixedPosition(mMtx, rPos, rRot));
    pMini->initWithoutIter();
    return pMini;
}

// 800FDBC0
SandyHand* Sandy::createHand(s32 index, const TVec3f& rPos, const TVec3f& rRot) {
    bool isLeft = index == 0;
    TVec3f pos(rPos);
    TVec3f rot(rRot);
    if (!isLeft) {
        pos.z *= -1.0f;
    }
    SandyHand* pHand = new SandyHand(this, isLeft, mHolder, new FixedPosition(MR::getJointMtx(this, "Spine2"), pos, rot));
    pHand->initWithoutIter();
    pHand->makeActorDead();
    return pHand;
}

// 800FDCC0
SandyBall* SandyMini::takeBall() {
    return mSandy->mHolder->getBall(this, 0);
}

// 800FDCE0
// NONMATCHING: initializes discarded offset rotation for native safety; matrix-copy outlining and register scheduling differ.
void attachBallToJoint(const LiveActor* pActor, SandyBall* pBall, const char* pJoint, const TVec3f& rOffset) {
    TPos3f joint;
    joint.set(MR::getJointMtx(pActor, pJoint));
    TPos3f offset;
#ifdef GALAXY_NATIVE
    // Initialize the rotation that the Wii routine leaves unspecified.
    offset.makeTrans(rOffset);
#else
    offset.setTrans(rOffset);
#endif
    TPos3f result;
    result.concat(joint, offset);
    TVec3f pos;
    result.getTrans(pos);
    MR::setPosition(pBall, pos);

    result.mMtx[0][3] = 0.0f;
    result.mMtx[1][3] = 0.0f;
    result.mMtx[2][3] = 0.0f;
    result.mMtx[0][0] = 1.0f;
    result.mMtx[0][1] = 0.0f;
    result.mMtx[0][2] = 0.0f;
    result.mMtx[1][0] = 0.0f;
    result.mMtx[1][1] = 1.0f;
    result.mMtx[1][2] = 0.0f;
    result.mMtx[2][0] = 0.0f;
    result.mMtx[2][1] = 0.0f;
    result.mMtx[2][2] = 1.0f;

    result.getQuat(pBall->mQuat);
}

// 800FDDE0
bool Sandy::isSensorBack(HitSensor* pSensor) const {
    TVec3f front;
    MR::calcFrontVec(&front, this);
    TVec3f dir = *MR::getSensorPos(pSensor) - mPosition;
    if (MR::normalizeOrZero(&dir)) {
        return false;
    }
    return front.dot(dir) < 0.0f;
}

// 800FDE70
void Sandy::copyQuat() {
    mTargetQuat.x = mQuat.x;
    mTargetQuat.y = mQuat.y;
    mTargetQuat.z = mQuat.z;
    mTargetQuat.w = mQuat.w;
}

// 800FDEA0
void turnToPlayer(TQuat4f* pQuat, LiveActor* pActor, f32 degree) {
    TVec3f dir = *MR::getPlayerPos() - pActor->mPosition;
    MR::vecKillElement(dir, pActor->mGravity, &dir);
    MR::normalize(&dir);
    TVec3f front;
    pQuat->getZDir(front);
    MR::turnDirectionToPlayerDegreeHorizon(pActor, &front, degree);
    TVec3f up = -pActor->mGravity;
    MR::makeQuatUpFront(pQuat, up, front);
}

// 800FDFB0
// NONMATCHING: operand order of the final add (register allocation).
bool Sandy::isTotalCount(s32 count) const {
    s32 rightHand = getRightHand()->mThrowCount;
    s32 leftHand = getLeftHand()->mThrowCount;
    s32 leftMini = getLeftMini()->mBallCount;
    s32 rightMini = getRightMini()->mBallCount;
    return count == leftMini + leftHand + (rightMini + rightHand);
}

// 800FE030
void Sandy::resetCount() {
    SandyHand* pLeftHand = getLeftHand();
    pLeftHand->mThrowCount = 0;
    SandyHand* pRightHand = getRightHand();
    pRightHand->mThrowCount = 0;
    SandyMini* pLeftMini = getLeftMini();
    pLeftMini->mBallCount = 0;
    SandyMini* pRightMini = getRightMini();
    pRightMini->mBallCount = 0;
}

// 800FE090
void Sandy::validateHands() {
    SandyHand* pLeft = getLeftHand();
    pLeft->mIsThrower = true;
    SandyHand* pRight = getRightHand();
    pRight->mIsThrower = true;
}

// 800FE0E0
void Sandy::invalidateHands() {
    SandyHand* pLeft = getLeftHand();
    pLeft->mIsThrower = false;
    SandyHand* pRight = getRightHand();
    pRight->mIsThrower = false;
}

// 800FE130
void Sandy::forceWaitHands() {
    getLeftHand()->forceWait();
    getRightHand()->forceWait();
}

// 800FE170
void Sandy::setWaitHands() {
    getLeftHand()->setWait();
    getRightHand()->setWait();
}

// 800FE1B0
void Sandy::setHandsStep(s32 step) {
    getLeftHand()->mWaitStep = step;
    getRightHand()->mWaitStep = step;
}

// 800FE200
void Sandy::setHandsBallType0() {
    getLeftHand()->setBallType0();
    getRightHand()->setBallType0();
}

// 800FE240
void Sandy::setHandsBallType3() {
    getLeftHand()->setBallType3();
    getRightHand()->setBallType3();
}

// 800FE280
void Sandy::setMinisNext() {
    getLeftMini()->setNextNerve();
    getRightMini()->setNextNerve();
}

// 800FE2C0
bool SandyMini::tryStartNext() {
    if (!isNerveStop()) {
        return false;
    }
    resume();
    return true;
}

// 800FE310
bool Sandy::isMinisDownOrDead() const {
    bool ret = false;
    if (getLeftMini()->isDownOrDead()) {
        if (getRightMini()->isDownOrDead()) {
            ret = true;
        }
    }
    return ret;
}

// 800FE370
void Sandy::showAll() {
    MR::showModel(this);
    MR::showModel(getLeftHand());
    MR::showModel(getRightHand());
}

// 800FE3C0
void Sandy::startActionAll(const char* pName, const char* pHandName) {
    MR::startAction(this, pName);
    getLeftHand()->setDamageAction(pHandName != nullptr ? pHandName : pName);
    getRightHand()->setDamageAction(pHandName != nullptr ? pHandName : pName);
}

// 800FE440
bool Sandy::tryStartBlur() {
    return mBlur->tryStart();
}

// 800FE450
void Sandy::startScaleHitReaction() {
    mScaleController->startHitReaction();
}

// 800FE460
AnimScaleParam* createScaleParam() {
    AnimScaleParam* pParam = new AnimScaleParam();
    pParam->mHitReactionSpringStrength = 0.3f;
    pParam->mHitReactionDamping = 0.7f;
    return pParam;
}
