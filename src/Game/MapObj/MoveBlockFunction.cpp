#include "Game/MapObj/MoveBlockFunction.hpp"
#include "Game/Boss/KoopaJrRobot.hpp"
#include "Game/Util.hpp"
MoveBlockFunction::MoveBlockFunction() : NameObj("ブロック移動関数"), mRotation(0,0,0,1), mInitialRotation(0,0,0,1), mPosition(0.0f,0.0f,0.0f), mInitialPosition(0.0f,0.0f,0.0f), mMoveVec(0.0f,0.0f,0.0f), mRotateVec(0.0f,0.0f,0.0f), mRate(0.0f), mMoveStep(60), mReturnStep(1), mStep(0), mForward(true) {}
MoveBlockFunction::~MoveBlockFunction() {}
void MoveBlockFunction::init(const JMapInfoIter& iter) {
    TVec3f rotate(0.0f,0.0f,0.0f);
    MR::getJMapInfoRotate(iter, &rotate);
    MR::makeQuatRotateDegree(&mInitialRotation, rotate);
    mRotation = mInitialRotation;
    MR::getJMapInfoTrans(iter, &mInitialPosition);
    mPosition.set<f32>(mInitialPosition);
}
void MoveBlockFunction::setMoveVec(const TVec3f& vec) { mMoveVec.set<f32>(vec); }
void MoveBlockFunction::setRotateVec(const TVec3f& vec) { mRotateVec.set<f32>(vec); }
void MoveBlockFunction::setMoveStep(s32 step) { if (step < 0) step = 0; mMoveStep = step; }
void MoveBlockFunction::setReturnStep(s32 step) { if (step < 1) step = 1; mReturnStep = step; }
void MoveBlockFunction::calcMoveVec(TVec3f* out) const { if (out) mInitialRotation.transform(mMoveVec, *out); }
void MoveBlockFunction::setForward(bool forward) {
    mForward = forward;
    if (mForward) {
        s32 step = mStep + 1;
        s32 returnStep = mReturnStep;
        mStep = step;
        if (step > returnStep) mStep = returnStep;
    }
    else { if (--mStep < 0) mStep = 0; }
}
// NONMATCHING: equivalent branch layout and rotation temporary register allocation.
bool MoveBlockFunction::update() {
    f32 delta = 1.0f / mMoveStep;
    if (mForward) {} else delta = -delta;
    mRate += delta;
    bool result = false;
    f32 limit = static_cast<f32>(mStep) / mReturnStep;
    if (mForward) {
        if (mRate > limit) { mRate = limit; result = true; }
    } else if (mRate < limit) { mRate = limit; result = true; }
    TVec3f position(mInitialPosition);
    TVec3f offset;
    mInitialRotation.transform(mMoveVec, offset);
    MR::vecScaleAdd(&position, &offset, mRate);
    mPosition.set<f32>(position);
    f32 z = 0.017453292f * (mRotateVec.z * mRate);
    f32 y = 0.017453292f * (mRotateVec.y * mRate);
    f32 x = 0.017453292f * (mRotateVec.x * mRate);
    z = KoopaJrRobotKillerLauncher::normalizeAngle(z);
    y = KoopaJrRobotKillerLauncher::normalizeAngle(y);
    x = KoopaJrRobotKillerLauncher::normalizeAngle(x);
    mRotation.setEuler(x,y,z);
    PSQUATMultiply(&mInitialRotation, &mRotation, &mRotation);
    return result;
}
bool MoveBlockFunction::isEndStep() const { return mStep == mReturnStep; }
bool MoveBlockFunction::isStartStep() const { return mStep == 0; }
bool MoveBlockFunction::canMove(bool forward) const {
    if (forward) { if (mStep >= mReturnStep) return false; }
    else if (mStep <= 0) return false;
    return true;
}
