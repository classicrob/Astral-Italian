#include "Game/Player/CloudMarioHat.hpp"
#include "Game/Player/MarioMtxPos.hpp"
#include <JSystem/JMath/JMATrigonometric.hpp>
#include <cmath>
#include "Game/Util/ActorInitUtil.hpp"
#include "Game/Util/ActionSoundUtil.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/System/GameDataFunction.hpp"
#include "Game/Util/JointUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/SoundUtil.hpp"

namespace {
    struct HatPointInfo { const char* joint; TVec3f offset; };
    HatPointInfo sPoints[] = {
        {"AllRoot", TVec3f(0.0f, 30.0f, -8.0f)},
        {"Cap1", TVec3f(0.0f, 0.0f, 0.0f)},
        {"Cap2", TVec3f(0.0f, 0.0f, 0.0f)},
        {"Cap3", TVec3f(0.0f, 0.0f, 0.0f)},
        {"Cap3", TVec3f(0.0f, 0.0f, -20.0f)}
    };
    const s32 sSpringPairs[][2] = {{0,1}, {1,2}, {2,3}, {3,4}};
    struct HatJointInfo { const char* name; s32 spring; };
    const HatJointInfo sJoints[] = {{"Cap1",1}, {"Cap2",2}, {"Cap3",3}};
    JointControlDelegator<CloudMarioHat>::func sCalcJoint = &CloudMarioHat::calcJoint;
}

CloudMarioHat::CloudMarioHat(const char* name) : LiveActor(name), _90(0), _91(0) {}
CloudMarioHat::~CloudMarioHat() {}

void CloudMarioHat::init(const JMapInfoIter& iter) {
    if (GameDataFunction::isLuigiPlayer()) MR::initActor(this, iter, "CloudLuigiCap", false);
    else MR::initActor(this, iter, "CloudMarioCap", false);
    MR::invalidateClipping(this);
    initDynamics();
}

void CloudMarioHat::initAfterPlacement() {
    MR::calcPlayerJointMtx(&mHatMatrix, "CapPosition");
    mHatMatrix.getTrans(mPosition);
    mHatMatrix.getEuler(mRotation);
    mRotation.x *= 57.295776f;
    mRotation.y *= 57.295776f;
    mRotation.z *= 57.295776f;
    MR::calcAnimDirect(this);
    resetDynamics();
    _90 = 1;
    makeActorDead();
}

void CloudMarioHat::calcAnim() { LiveActor::calcAnim(); }

void CloudMarioHat::appear() {
    LiveActor::appear();
    if (_90) {
        MR::calcPlayerJointMtx(&mHatMatrix, "CapPosition");
        mHatMatrix.getTrans(mPosition);
        mHatMatrix.getEuler(mRotation);
        mRotation.x *= 57.295776f;
        mRotation.y *= 57.295776f;
        mRotation.z *= 57.295776f;
        updateDynamics();
    }
}

bool CloudMarioHat::calcJoint(TPos3f* matrix, const JointControllerInfo& info) {
    s32 index = findJoint(info);
    if (index == -1) return false;
    updateJoint(matrix, index);
    return true;
}

void CloudMarioHat::control() {
    _91 = 1;
    MR::startActionSound(this, "MokuMoku", -1, -1, -1);
}

void CloudMarioHat::calcAndSetBaseMtx() {
    if (_90) {
        MR::concatWithPlayerJointMtx(&mHatMatrix, "CapPosition");
        TVec3f z, y, x;
        mHatMatrix.getYDir(y);
        mHatMatrix.getZDir(z);
        PSVECCrossProduct(&y, &z, &x);
        mHatMatrix.setXDir(x);
        mHatMatrix.getTrans(mPosition);
        mHatMatrix.getEuler(mRotation);
        mRotation.x *= 57.295776f;
        mRotation.y *= 57.295776f;
        mRotation.z *= 57.295776f;
        if (_91) {
            updateDynamics();
            _91 = 0;
        }
        for (u32 i = 0; i < 3; ++i) mControllers[i]->registerCallBack();
    }
    LiveActor::calcAndSetBaseMtx();
}

// NONMATCHING: callback/table register choices and pooled-data relocations.
void CloudMarioHat::initDynamics() {
    mHatMatrix.identity();
    mPoints = new CloudHatPoint*[5];
    for (u32 i = 0; i < 5; ++i) mPoints[i] = new CloudHatPoint();
    mSprings = new CloudHatSpring*[4];
    for (u32 i = 0; i < 4; ++i) mSprings[i] = new CloudHatSpring(&mHatMatrix, mPoints[sSpringPairs[i][0]], mPoints[sSpringPairs[i][1]]);
    mControllers = new JointController*[3];
    mJoints = new J3DJoint*[3];
    for (u32 i = 0; i < 3; ++i) {
        JointController* controller = MR::createJointDelegatorWithNullChildFunc<CloudMarioHat>(this, sCalcJoint, sJoints[i].name);
        mControllers[i] = controller;
        mJoints[i] = MR::getJoint(this, sJoints[i].name);
    }
}

#pragma push
#pragma opt_common_subs off
void CloudMarioHat::resetDynamics() {
    MarioMtxPos matrix(mHatMatrix);
    for (u32 i = 0; i < 5; ++i) {
        TVec3f position;
        MR::copyJointPos(this, sPoints[i].joint, &position);
        TVec3f offset(sPoints[i].offset);
        matrix.fn_804096E0(&offset);
        position.add(offset);
        mPoints[i]->mPosition.set<f32>(position);
    }
    for (u32 i = 0; i < 4; ++i) mSprings[i]->reset();
}

#pragma pop

void CloudMarioHat::updateDynamics() {
    mHatMatrix.getTrans(mPoints[0]->mPosition);
    TVec3f offset(sPoints[0].offset);
    mHatMatrix.mult33(offset, offset);
    mPoints[0]->mPosition.add(offset);
    TVec3f y, z;
    mHatMatrix.getYDir(y);
    mHatMatrix.getZDir(z);
    TVec3f direction(y);
    direction.sub(z);
    MR::normalizeOrZero(&direction);
    for (u32 i = 0; i < 4; ++i) {
        mSprings[i]->mAnchor = mPoints[0]->mPosition;
        mSprings[i]->mDirection = direction;
    }
    const TVec3f& gravityDirection = mGravity;
    for (u32 i = 0; i < 4; ++i) {
        TVec3f gravity(gravityDirection);
        gravity.scale(0.05f);
        mSprings[i]->update(gravity);
    }
}

s32 CloudMarioHat::findJoint(const JointControllerInfo& info) {
    for (s32 i = 0; i < 3; ++i) if (mJoints[i] == info.mJoint) return i;
    return -1;
}

void CloudMarioHat::updateJoint(TPos3f* matrix, s32 index) {
    CloudHatSpring* spring = mSprings[sJoints[index].spring];
    TVec3f start(spring->mStart->mPosition);
    TVec3f end(spring->mEnd->mPosition);
    TVec3f direction(end);
    direction.sub(start);
    if (!MR::normalizeOrZero(&direction)) {
        TVec3f x;
        matrix->getXDir(x);
        MarioMtxPos original(*matrix);
        MarioMtxPos rotation;
        rotation.fn_80409710(x, direction);
        rotation.fn_80409650(original);
        MarioMtxPos& result = rotation;
        matrix->set(*result.fn_vf0());
    }
    matrix->setTrans(start);
}

TMtx34f* MarioMtxPos::fn_vf0() { return &mMtx; }

CloudHatPoint::CloudHatPoint() : mPosition(0.0f) {
    mVelocity.x = 0.0f;
    mVelocity.y = 0.0f;
    mVelocity.z = 0.0f;
    mAcceleration.x = 0.0f;
    mAcceleration.y = 0.0f;
    mAcceleration.z = 0.0f;
}

void CloudHatPoint::update() {
    mVelocity.add(mAcceleration);
    mVelocity.scale(0.9f);
    mAcceleration.zero();
}

CloudHatSpring::CloudHatSpring(TPos3f* matrix, CloudHatPoint* start, CloudHatPoint* end) {
    mMatrix = matrix;
    mStart = start;
    mEnd = end;
    mAnchor.x = 0.0f;
    mAnchor.y = 0.0f;
    mAnchor.z = 0.0f;
    mDirection.x = 0.0f;
    mDirection.y = 0.0f;
    mDirection.z = 0.0f;
    mRestOffset.zero();
    mLength = 0.0f;
    mPreviousDirection.x = 1.0f;
    mPreviousDirection.y = 0.0f;
    mPreviousDirection.z = 0.0f;
}

void CloudHatSpring::update(const TVec3f& acceleration) {
    MarioMtxPos matrix(mMatrix);
    TVec3f rest(mRestOffset);
    matrix.fn_804096E0(&rest);
    const TVec3f* startPosition = &mStart->mPosition;
    TVec3f difference(mEnd->mPosition);
    difference.sub(*startPosition);
    PSVECMag(&difference);
    TVec3f direction(difference);
    if (MR::normalizeOrZero(&direction)) direction = mPreviousDirection;
    else mPreviousDirection = direction;
    f32 angle = rest.angle(direction);
    if (angle > 0.0f) {
        TVec3f axis;
        PSVECCrossProduct(&direction, &rest, &axis);
        MR::normalize(&axis);
        TVec3f correction;
        PSVECCrossProduct(&axis, &direction, &correction);
        angle *= 0.6f;
        correction.scale(angle);
        mEnd->mAcceleration.add(correction);
    }
    mEnd->mAcceleration.add(acceleration);
    mEnd->update();
    CloudHatPoint* velocityPoint = mEnd;
    f32 projection = direction.dot(velocityPoint->mVelocity);
    velocityPoint->mVelocity.sub(direction * projection);
    CloudHatPoint* predictedPoint = mEnd;
    TVec3f predicted(predictedPoint->mPosition);
    predicted.add(predictedPoint->mVelocity);
    TVec3f relative(predicted);
    relative.sub(mAnchor);
    f32 planeDistance = mDirection.dot(relative);
    if (planeDistance < 0.0f) {
        mEnd->mVelocity.sub(mDirection * planeDistance);
    }
    mEnd->mPosition.add(mEnd->mVelocity);
    const TVec3f* offsetStart = &mStart->mPosition;
    TVec3f offset(mEnd->mPosition);
    offset.sub(*offsetStart);
    MR::normalize(&offset);
    offset.scale(mLength);
    CloudHatPoint* finalPoint = mEnd;
    finalPoint->mPosition.set<f32>(mStart->mPosition + offset);
}

void CloudHatSpring::reset() {
    const TVec3f* startPosition = &mStart->mPosition;
    TVec3f difference(mEnd->mPosition);
    difference.sub(*startPosition);
    mLength = PSVECMag(&difference);
    mPreviousDirection = difference;
    MR::normalize(&mPreviousDirection);
    MarioMtxPos matrix(mMatrix);
    matrix.invert();
    mRestOffset = difference;
    matrix.fn_804096E0(&mRestOffset);
}
