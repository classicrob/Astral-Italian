#define JGEOMETRY_VEC3_WMR_OUTLINE
#define JGEOMETRY_VEC3_SET_NOINLINE
#define JGEOMETRY_VEC3_WMR_INLINE_OPS
#include "Game/System/GrandGalaxyMapStateMove.hpp"
#include "Game/System/GrandGalaxyMap.hpp"
#include "Game/System/MiniPlayerRocket.hpp"
#include "Game/System/WorldMapCameraHolder.hpp"
#include "Game/Util.hpp"

namespace NrvGrandGalaxyMapStateMove {
    NEW_NERVE(GrandGalaxyMapStateMoveNext, GrandGalaxyMapStateMove, Move);
    NEW_NERVE(GrandGalaxyMapStateMovePrevious, GrandGalaxyMapStateMove, Move);
};  // namespace NrvGrandGalaxyMapStateMove

GrandGalaxyMapStateMove::GrandGalaxyMapStateMove(GrandGalaxyMap* pHost)
    : ActorStateBase< GrandGalaxyMap >("State[移動]", pHost), mAccel(100.0f), mMaxSpeed(400.0f), mFriction(0.85f), mVelocity(0.0f, 0.0f),
      mPos(0.0f, 0.0f), mStartPos(0.0f, 0.0f), mCameraStartPos(0.0f, 0.0f), mFrom(-1), mTo(-1), mStep(0) {}

void GrandGalaxyMapStateMove::init() {
    initNerve(&NrvGrandGalaxyMapStateMove::GrandGalaxyMapStateMoveNext::sInstance, 0);
    mIsDead = true;
}

void GrandGalaxyMapStateMove::appear() {
    mIsDead = false;
    mFrom = mHost->getCamera()->mCurrent;
    mTo = mFrom;
    mStep = 0;
    MiniPlayerRocket* pRocket = mHost->getRocket();
    mPos.set(pRocket->mPosition);
    mStartPos.set(pRocket->mPosition);
    mVelocity.set(pRocket->mVelocity);
    mAccel = 100.0f;
    mMaxSpeed = 400.0f;
    mFriction = 0.85f;
    mHost->changeRocketAction("Wait");
    if (mFrom < mHost->getPath(mHost->getCamera()->mNext, mStep)) {
        mHost->requestNextEnd();
        setNerve(&NrvGrandGalaxyMapStateMove::GrandGalaxyMapStateMoveNext::sInstance);
    } else {
        mHost->requestPreviousEnd();
        setNerve(&NrvGrandGalaxyMapStateMove::GrandGalaxyMapStateMovePrevious::sInstance);
    }
}

void GrandGalaxyMapStateMove::kill() {
    if (isNerve(&NrvGrandGalaxyMapStateMove::GrandGalaxyMapStateMoveNext::sInstance)) {
        mHost->startNextAppear();
    } else {
        mHost->startPreviousAppear();
    }
    mIsDead = true;
}

// NONMATCHING: register/stack or instruction/branch lowering (FP operand order/grouping checked).
void GrandGalaxyMapStateMove::exeMove() {
    WorldMapCameraHolder* pCamera = mHost->getCamera();
    MiniPlayerRocket* pRocket = pCamera->mRocket;
    pRocket->resetAction();
    mHost->requestAppearWorldName();

    bool isLast = false;
    if (MR::isFirstStep(this)) {
        MR::startActionSound(pRocket, "MoveInGMap", -1, -1, -1);
        mFrom = mTo;
        GrandGalaxyMap::getNodePosition(&mCameraStartPos, mHost, mTo);
        mTo = mHost->getPath(pCamera->mNext, mStep);
        mStep++;
        if (mStep >= mHost->getPathDistance(pCamera->mNext)) {
            isLast = true;
        }
        mHost->startAreaCamera(mTo, -1);
        if (isLast) {
            mPos.set(pRocket->mPosition);
            mStartPos.set(pRocket->mPosition);
            mVelocity.set(pRocket->mVelocity);
            MR::zeroVelocity(pRocket);
        }
    }

    if (mStep >= mHost->getPathDistance(pCamera->mNext)) {
        isLast = true;
    }

    TVec3f target(0.0f);
    GrandGalaxyMap::getNodePosition(&target, mHost, mTo);
    if (isLast) {
        TVec3f accel(0.0f);
        MR::calcVelocityMoveToTarget3D(&accel, pRocket, target, mAccel);
        mVelocity.add(accel);
        mVelocity.scale(mFriction);
        mPos.add(mVelocity);
        f32 rate = MR::calcNerveEaseOutRate(this, 26);
        TVec3f goal = target * rate + mStartPos * (1.0f - rate);
        pRocket->setMoveDirection(mVelocity);
        pRocket->mPosition = goal * rate + mPos * (1.0f - rate);
    } else {
        MR::addVelocityMoveToTarget3D(pRocket, target, mAccel);
        MR::attenuateVelocity(pRocket, mFriction);
        MR::restrictVelocity(pRocket, mMaxSpeed);
        pRocket->setMoveDirection(pRocket->mVelocity);
    }

    f32 rate = MR::calcNerveEaseInOutRate(this, 20);
    TVec3f cameraPos = target * rate + mCameraStartPos * (1.0f - rate);
    MR::setPosition(pCamera->mCameraTarget, cameraPos);

    if ((isLast && MR::isGreaterEqualStep(this, 26)) || (MR::isNear(pRocket, target, 10.0f) && (MR::isVelocityGreaterEqual(pRocket, 1.0f) ? false : true))
        || (!isLast && MR::isNear(pRocket, target, 1000.0f))) {
        if (!isLast) {
            if (isNerve(&NrvGrandGalaxyMapStateMove::GrandGalaxyMapStateMoveNext::sInstance)) {
                setNerve(&NrvGrandGalaxyMapStateMove::GrandGalaxyMapStateMoveNext::sInstance);
            } else {
                setNerve(&NrvGrandGalaxyMapStateMove::GrandGalaxyMapStateMovePrevious::sInstance);
            }
        } else {
            MR::startActionSound(pRocket, "MoveClick", -1, -1, -1);
            mHost->warpRocketToNode(-1);
            kill();
        }
    }
}

GrandGalaxyMapStateMove::~GrandGalaxyMapStateMove() {}
