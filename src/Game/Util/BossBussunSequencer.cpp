#define JGEOMETRY_VEC3_MUL_NOINLINE
#include "Game/Util/BossBussun.hpp"
#include "Game/Enemy/WalkerStateBindStarPointer.hpp"
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include "Game/Util/ActorCameraUtil.hpp"
#include "Game/Util/ActorStateUtil.hpp"
#include "Game/Util/JointController.hpp"

BossBussunSequencer::BossBussunSequencer(BossBussun* pHost, const JMapInfoIter& rIter)
    : mHost(pHost), mCenterPos(0.0f, 0.0f, 0.0f), mUpVec(0.0f, 1.0f, 0.0f), mSideVec(0.0f, 0.0f, 1.0f), mCamFront(0.0f, 0.0f, 1.0f),
      mCamUp(0.0f, 1.0f, 0.0f), mCamPos(0.0f, 0.0f, 0.0f), mIsNear(false) {
    MR::initActorCameraProgrammable(pHost);
    TPos3f zoneMtx;
    zoneMtx.setInline(MR::getZonePlacementMtx(rIter));
    zoneMtx.mult33(mUpVec, mUpVec);
    zoneMtx.mult(mCenterPos, mCenterPos);
}

void BossBussunSequencer::initCameraState() {
    mCamUp.set(MR::getCamYdir());
    mCamFront.set(-MR::getCamZdir());
    mIsNear = isNearHeight(0.0f);
    MR::calcVecFromPlayerH(&mSideVec, mHost, &mUpVec);
}

void BossBussunSequencer::startCamera() {
    initCameraState();
    MR::startActorCameraProgrammable(mHost, -1);
}

void BossBussunSequencer::endCamera() {
    MR::endActorCameraProgrammable(mHost, -1, true);
}

// NONMATCHING: register/stack or instruction/branch lowering (FP operand order/grouping checked).
inline TVec3f scaleBussunCameraFront(const TVec3f& direction) {
    TVec3f front(direction);
    front *= 1700.0f;
    return front;
}

void BossBussunSequencer::updateCamera() {
    TVec3f rotA;
    TVec3f rotB;
    calcFrameAxes(&rotA, &rotB);

    TQuat4f quatA;
    MR::makeQuatFrontUp(&quatA, mCamFront, mCamUp);
    TQuat4f quatB;
    MR::makeQuatFrontUp(&quatB, rotB, rotA);
    TQuat4f quat;
    quat.x = quatA.x;
    quat.y = quatA.y;
    quat.z = quatA.z;
    quat.w = quatA.w;
    quat.slerp(quatB, 0.03f);
    quat.getYDir(mCamUp);
    quat.getZDir(mCamFront);

    const TVec3f& hostPos = mHost->mPosition;
    TVec3f blendPos;
    MR::vecBlend(&blendPos, *MR::getPlayerPos(), hostPos, 0.1f);
    f32 dot = calcPlayerDot();
    f32 offset;
    if (mIsNear) {
        offset = 100.0f * MR::normalize(dot, 0.0f, 80.0f);
    } else {
        offset = 100.0f * -(1.0f - MR::normalize(dot, -80.0f, 0.0f));
    }

    TVec3f vec(mUpVec);
    vec *= offset;
    MR::vecBlend(&vec, vec, mCamPos, 0.03f);
    mCamPos = vec;
    blendPos += mCamPos;
    TVec3f eye = blendPos + scaleBussunCameraFront(mCamFront);
    MR::setProgrammableCameraParam(mHost, blendPos, eye, mCamUp);
}

namespace {
    inline TVec3f calcNormalizedFrameDirection(const TVec3f& toPlayer, f32 dist) {
        TVec3f dir(toPlayer);
        dir *= 1.0f / dist;
        return dir;
    }

    inline void calcNormalizedFrameAxis(TVec3f* axis, const TVec3f& direction, const TVec3f& up) {
        PSVECCrossProduct(&direction, &up, axis);
    }
}

void BossBussunSequencer::calcFrameAxes(TVec3f* pOutA, TVec3f* pOutB) {
    const TVec3f& gravity = mHost->mGravity;
    bool isPlayerBelow = gravity.dot(*MR::getPlayerGravity()) > 0.0f;
    bool isNear = isNearHeight(80.0f);
    TVec3f toPlayer;
    MR::calcVecFromPlayerHorizon(&toPlayer, mHost, &mUpVec);
    f32 turn;
    f32 dist = PSVECMag(&toPlayer);
    f32 angle;
    if (isPlayerBelow) {
        angle = 70.0f;
    } else {
        angle = MR::getEaseInOutValue(dist, 0.0f, 2500.0f, 0.0f, 60.0f);
    }

    TVec3f up(isNear ? mUpVec : -mUpVec);
    TVec3f axis;
    calcNormalizedFrameAxis(&axis, calcNormalizedFrameDirection(toPlayer, dist), up);
    if (MR::normalizeOrZero(&axis)) {
        MR::makeAxisVerticalZX(&axis, up);
    }

    TVec3f prevSide(mSideVec);
    if (mIsNear != isNear) {
        MR::rotateVecDegree(&mSideVec, prevSide, -axis, 180.0f);
        JMAVECScaleAdd(&mUpVec, &mSideVec, &mSideVec, -mUpVec.dot(mSideVec));
        MR::normalize(&mSideVec);
        prevSide.set(mSideVec);
    } else {
        if (isPlayerBelow) {
            turn = 5.0f;
        } else {
            turn = MR::getEaseInOutValue(dist, 500.0f, 1500.0f, 0.0f, 5.0f);
        }
        if (turn > 0.0f) {
            TVec3f target(mSideVec.dot(toPlayer) > 0.0f ? toPlayer : -toPlayer);
            target *= 1.0f / dist;
            MR::turnVecToVecDegree(&mSideVec, mSideVec, target, turn, TVec3f(0, 1, 0));
            JMAVECScaleAdd(&mUpVec, &mSideVec, &mSideVec, -mUpVec.dot(mSideVec));
            MR::normalize(&mSideVec);
        }
    }

    MR::rotateVecDegree(pOutA, prevSide, axis, angle);
    MR::rotateVecDegree(pOutB, up, axis, angle);
    mIsNear = isNear;
}

f32 BossBussunSequencer::calcPlayerDot() const {
    return mUpVec.dot(*MR::getPlayerPos() - mCenterPos);
}

bool BossBussunSequencer::isNearHeight(f32 threshold) const {
    f32 dot = calcPlayerDot();
    if (__fabs(dot) < threshold) {
        return mIsNear;
    }
    return dot > 0.0f;
}
