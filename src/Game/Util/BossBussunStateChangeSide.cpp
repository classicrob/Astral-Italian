#include "Game/Util/BossBussun.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"

namespace NrvBossBussunStateChangeSide {
    NEW_NERVE(BossBussunStateChangeSideNrvTurnToEdge, BossBussunStateChangeSide, TurnToEdge);
    NEW_NERVE(BossBussunStateChangeSideNrvMoveToEdge, BossBussunStateChangeSide, MoveToEdge);
    NEW_NERVE(BossBussunStateChangeSideNrvTurnFromCenter, BossBussunStateChangeSide, TurnFromCenter);
    NEW_NERVE(BossBussunStateChangeSideNrvMoveFromCenter, BossBussunStateChangeSide, MoveFromCenter);
    NEW_NERVE(BossBussunStateChangeSideNrvFlip, BossBussunStateChangeSide, Flip);
    NEW_NERVE(BossBussunStateChangeSideNrvMoveToHome, BossBussunStateChangeSide, MoveToHome);
    NEW_NERVE(BossBussunStateChangeSideNrvMoveAcross, BossBussunStateChangeSide, MoveAcross);
};  // namespace NrvBossBussunStateChangeSide

BossBussunStateChangeSide::BossBussunStateChangeSide(BossBussun* pHost)
    : ActorStateBase< BossBussun >("ボスブッスン、表裏移動", pHost), mDirFromCenter(0.0f, 0.0f, 0.0f), mAxis(0.0f, 0.0f, 0.0f), mUpDir(0.0f, 0.0f, 0.0f),
      mStartPos(0.0f, 0.0f, 0.0f), mHeight(0.0f), mIsUpSide(false) {
    initNerve(GET_NERVE(BossBussunStateChangeSide, BossBussunStateChangeSideNrvTurnFromCenter), 0);
}

void BossBussunStateChangeSide::appear() {
    mIsDead = false;
    setNerve(GET_NERVE(BossBussunStateChangeSide, BossBussunStateChangeSideNrvTurnFromCenter));
    mHost->calcNearestHomePos(&mHost->mTargetPos);
    if (MR::calcDistanceHorizontal(mHost, mHost->mTargetPos, mHost->mUpVec) < 200.0f) {
        setNerve(GET_NERVE(BossBussunStateChangeSide, BossBussunStateChangeSideNrvTurnFromCenter));
        return;
    }
    setNerve(GET_NERVE(BossBussunStateChangeSide, BossBussunStateChangeSideNrvTurnToEdge));
}

void BossBussunStateChangeSide::exeTurnToEdge() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mHost, "Turn");
    }
    mHost->updateVelocityOnGround();
    if (MR::turnDirectionToTargetDegreeHorizon(mHost, &mHost->mFrontDir, mHost->mTargetPos, 2.0f)) {
        setNerve(GET_NERVE(BossBussunStateChangeSide, BossBussunStateChangeSideNrvMoveToEdge));
    }
}

void BossBussunStateChangeSide::exeMoveToEdge() {
    MR::turnDirectionToTargetDegreeHorizon(mHost, &mHost->mFrontDir, mHost->mTargetPos, 2.0f);
    MR::addVelocityMoveToDirection(mHost, mHost->mFrontDir, 3.0f);
    mHost->updateVelocityOnGround();
    if (MR::calcDistanceHorizontal(mHost, mHost->mTargetPos, mHost->mUpVec) < 200.0f) {
        setNerve(GET_NERVE(BossBussunStateChangeSide, BossBussunStateChangeSideNrvTurnFromCenter));
    }
}

void BossBussunStateChangeSide::exeTurnFromCenter() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mHost, "Turn");
    }
    mHost->updateVelocityOnGround();
    if (MR::turnDirectionFromTargetDegreeHorizon(mHost, &mHost->mFrontDir, mHost->mHomePos, 2.0f)) {
        setNerve(GET_NERVE(BossBussunStateChangeSide, BossBussunStateChangeSideNrvMoveFromCenter));
    }
}

void BossBussunStateChangeSide::exeMoveFromCenter() {
    MR::turnDirectionFromTargetDegreeHorizon(mHost, &mHost->mFrontDir, mHost->mHomePos, 2.0f);
    MR::addVelocityMoveToDirection(mHost, mHost->mFrontDir, 3.0f);
    mHost->updateVelocityOnGround();
    BossBussun* pHost = mHost;
    if (MR::calcDistanceHorizontal(pHost, pHost->mHomePos, pHost->mUpVec) > pHost->getAttackDistanceLimit()) {
        setNerve(GET_NERVE(BossBussunStateChangeSide, BossBussunStateChangeSideNrvFlip));
    }
}

// NONMATCHING: regalloc/stack layout
void BossBussunStateChangeSide::exeFlip() {
    if (MR::isFirstStep(this)) {
        TVec3f up;
        MR::calcUpVec(&up, mHost);
        mIsUpSide = mHost->mUpVec.dot(up) >= 0.0f;
        TVec3f neg;
        const TVec3f* pDir;
        if (mIsUpSide) {
            pDir = &mHost->mUpVec;
        } else {
            neg.negate(mHost->mUpVec);
            pDir = &neg;
        }
        mUpDir.set(*pDir);
        MR::calcVecFromTargetPosH(&mDirFromCenter, mHost, mHost->mCenterPos, &mHost->mUpVec);
        PSVECCrossProduct(&mUpDir, &mDirFromCenter, &mAxis);
        mHeight = __fabs(MR::calcDistanceVertical(mHost, mHost->mCenterPos, mHost->mUpVec));
        TVec3f offset = mUpDir * mHeight;
        mStartPos.set(mHost->mPosition - offset);
        MR::zeroVelocity(mHost);
        MR::startAction(mHost, "ChangeSide");
        MR::offBind(mHost);
    }
    TVec3f rotated;
    MR::rotateVecDegree(&rotated, mUpDir, mAxis, MR::calcNerveValue(this, 90, 0.0f, 180.0f));
    TVec3f scaled = rotated * mHeight;
    mHost->mPosition.set(mStartPos + scaled);
    if (MR::isGreaterEqualStep(this, 90)) {
        MR::onBind(mHost);
        setNerve(GET_NERVE(BossBussunStateChangeSide, BossBussunStateChangeSideNrvMoveToHome));
    }
}

void BossBussunStateChangeSide::exeMoveToHome() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mHost, "Walk");
    }
    MR::turnDirectionToTargetDegreeHorizon(mHost, &mHost->mFrontDir, mHost->mHomePos, 2.0f);
    MR::addVelocityMoveToDirection(mHost, mHost->mFrontDir, 3.0f);
    mHost->updateVelocityOnGround();
    BossBussun* pHost = mHost;
    if (MR::calcDistanceHorizontal(pHost, pHost->mHomePos, pHost->mUpVec) < pHost->getDashDistanceLimit()) {
        kill();
    }
}

void BossBussunStateChangeSide::exeMoveAcross() {
    TVec3f up;
    MR::calcUpVec(&up, mHost);
    f32 dot = mHost->mUpVec.dot(up);
    if (MR::isFirstStep(this)) {
        MR::startAction(mHost, "Walk");
        mIsUpSide = dot >= 0.0f;
    }
    f32 speed;
    if (mIsUpSide ? dot < 0.8f : dot > -0.8f) {
        speed = 0.5f;
    } else {
        speed = 3.0f;
    }
    MR::addVelocityMoveToDirection(mHost, mHost->mFrontDir, speed);
    mHost->updateVelocityOnGround();
    if ((mIsUpSide && dot < -0.98f) || (!mIsUpSide && dot > 0.98f)) {
        kill();
    }
}

BossBussunStateChangeSide::~BossBussunStateChangeSide() {
}
