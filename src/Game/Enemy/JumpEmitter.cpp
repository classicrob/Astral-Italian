#include "Game/Enemy/JumpEmitter.hpp"
#include "Game/LiveActor/ActorCameraInfo.hpp"
#include "Game/LiveActor/ActorStateParamScale.hpp"
#include "Game/LiveActor/LiveActorGroup.hpp"
#include "Game/LiveActor/PartsModel.hpp"
#include "Game/Util.hpp"
#include "Game/Util/MultiEventCamera.hpp"

namespace {
    const f32 sRotateDegree = 3.0f;
    const f32 sCameraLimitLength = 6250000.0f;
    const f32 sRepeatMax = 360.0f;
    const f32 sRepeatMin = -180.0f;
};  // namespace

JumpEmitter::JumpEmitter(const char* pName) : LiveActor(pName) {
    _90 = 0;
    mHeadModel = nullptr;
    mStartPos.x = 0.0f;
    mStartPos.y = 0.0f;
    mStartPos.z = 0.0f;
    mIsEventCamera = false;
    mCameraInfo = nullptr;
    mMultEventCamera = nullptr;
    _94.identity();
}

void JumpEmitter::kill() {
    LiveActor::kill();
    endEventCamera();
}

void JumpEmitter::init(const JMapInfoIter& rIter) {
    mParamScale = new ActorStateParamScale(this);
    mParamScale->init(rIter);

    mCameraInfo = new ActorCameraInfo(rIter);
    if (mCameraInfo->mCameraSetID != -1) {
        mMultEventCamera = new MultiEventCamera();
        mMultEventCamera->setUp(mName, mCameraInfo, 2);
        mMultEventCamera->setEndCameraTypeSoon();
    }
}

void JumpEmitter::control() {
    mParamScale->calcHostVelocity();
}

void JumpEmitter::movement() {
    mParamScale->update();
    LiveActor::movement();
    mParamScale->resetHostVelocity();
}

void JumpEmitter::startEventCamera() {
    if (mMultEventCamera == nullptr) {
        return;
    }

    mMultEventCamera->start(0);
    mIsEventCamera = true;
    mStartPos.set(mPosition);
}

void JumpEmitter::updateEventCamera() {
    if (!mIsEventCamera) {
        return;
    }

    mMultEventCamera->update();

    if (mMultEventCamera->isActive(0) && !MR::isPlayerJumpRising()) {
        mMultEventCamera->nextForce();
    }

    if (sCameraLimitLength < mStartPos.squared(*MR::getPlayerCenterPos()) || MR::isOnGroundPlayer() || MR::isPlayerInRush()) {
        endEventCamera();
    }
}

void JumpEmitter::endEventCamera() {
    if (mMultEventCamera == nullptr) {
        return;
    }

    if (mIsEventCamera) {
        mIsEventCamera = false;
        mMultEventCamera->endForceSoon();
    }
}

void JumpEmitter::updateRotate() {
    PartsModel* head = mHeadModel;

    TRot3f mtx;
    mtx.identity();
    mtx.set(getBaseMtx());
    mtx.invert(mtx);

    TVec3f playerPos(*MR::getPlayerPos());
    mtx.mult(playerPos, playerPos);

    playerPos.y = 0.0f;
    if (MR::isNearZero(playerPos)) {
        return;
    }

    MR::normalize(&playerPos);

    f32 newAngle = MR::calcYawDegreeXZ(playerPos);

    f32 angleDiff = MR::repeat(newAngle - head->mRotation.y, sRepeatMin, sRepeatMax);

    if (MR::abs(angleDiff) < sRotateDegree) {
        head->mRotation.y = newAngle;
    } else {
        head->mRotation.y += sRotateDegree * MR::sign(angleDiff);
    }

    head->mRotation.y = MR::repeat(head->mRotation.y, sRepeatMin, sRepeatMax);
}

bool MR::enableGroupAttack(LiveActor* pActor, f32 arg2, f32 arg3) {
    LiveActorGroup* group = MR::getGroupFromArray(pActor);

    if (isValidSwitchA(pActor) && !isOnSwitchA(pActor)) {
        return false;
    }

    if (isValidSwitchB(pActor) && !isOnSwitchB(pActor)) {
        return false;
    }

    LiveActor* closest = pActor;
    if (group != nullptr) {
        for (int i = 0; i < group->getObjNum(); i++) {
            LiveActor* actor = group->getActor(i);
            if (MR::calcDistanceToPlayer(actor) < MR::calcDistanceToPlayer(closest)) {
                closest = actor;
            }
        }
    }

    return isNearPlayerPose(closest, arg2, arg3);
}
