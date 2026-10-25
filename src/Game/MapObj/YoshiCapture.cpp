#pragma auto_inline off
#include "Game/MapObj/YoshiCapture.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Map/CollisionParts.hpp"
#include "Game/Map/HitInfo.hpp"
#include "Game/Util.hpp"
#include "Game/Util/YoshiUtil.hpp"

namespace NrvYoshiCapture {
    NEW_NERVE(YoshiCaptureNrvWait, YoshiCapture, Wait);
    NEW_NERVE(YoshiCaptureNrvCatch, YoshiCapture, Catch);
    NEW_NERVE(YoshiCaptureNrvRelease, YoshiCapture, Release);
}

YoshiCapture::YoshiCapture(const char* name) : LiveActor(name), mGround(nullptr),
    mLockOnTarget(nullptr), mClippingPosition(0.0f, 0.0f, 0.0f), mFixedDirection(false) {
    mGroundRelativePose.identity();
}
void YoshiCapture::init(const JMapInfoIter& iter) {
    MR::getJMapInfoArg0NoInit(iter, &mFixedDirection);
    MR::initActor(this, iter, "YCaptureTarget", false);
    mGroundRelativePose.set(getBaseMtx());
    mLockOnTarget = MR::createYoshiLockOnTarget(this, (MtxPtr)nullptr, TVec3f(0.0f));
    initNerve(&NrvYoshiCapture::YoshiCaptureNrvWait::sInstance);
    MR::useStageSwitchAwake(this, iter);
    bool noShadow = false;
    MR::getJMapInfoArg1NoInit(iter, &noShadow);
    if (noShadow) MR::invalidateShadow(this, nullptr);
    MR::useStageSwitchSyncAppear(this, iter);
}
// NONMATCHING: equivalent TVec3::scale arithmetic is inlined instead of called.
void YoshiCapture::initAfterPlacement() {
    MR::setClippingRangeIncludeShadow(this, &mClippingPosition, 150.0f);
    TVec3f gravity;
    MR::calcGravityVector(this, &gravity, nullptr, 0);
    Triangle triangle;
    TVec3f hit;
    TVec3f ray(gravity);
    f32 rayLength = 500.0f;
    ray.scale(rayLength);
    if (MR::getFirstPolyOnLineToMap(&hit, &triangle, mPosition, ray)) {
        mGround = triangle.mParts;
        mGroundRelativePose.mMtx[0][3] = mPosition.x;
        mGroundRelativePose.mMtx[1][3] = mPosition.y;
        mGroundRelativePose.mMtx[2][3] = mPosition.z;
        TPos3f inverse;
        inverse.set(mGround->mBaseMatrix);
        inverse.invert(inverse);
        mGroundRelativePose.concat(inverse, mGroundRelativePose);
    }
}
void YoshiCapture::appear() {
    LiveActor::appear();
    setNerve(&NrvYoshiCapture::YoshiCaptureNrvWait::sInstance);
    MR::tryHitReactionAppear(this);
}
void YoshiCapture::kill() { LiveActor::kill(); }
void YoshiCapture::exeWait() {
    if (MR::isFirstStep(this)) {
        if (mFixedDirection) MR::startAction(this, "Wait");
        else MR::startAction(this, "WaitFree");
    }
    MR::requestYoshiCaptureGuidance();
}
void YoshiCapture::exeCatch() {
    if (MR::isFirstStep(this)) {
        if (!mFixedDirection) facePlayer();
        MR::startAction(this, "Catch");
    }
}
void YoshiCapture::exeRelease() {
    if (MR::isFirstStep(this)) MR::startAction(this, "Release");
    if (MR::isActionEnd(this)) setNerve(&NrvYoshiCapture::YoshiCaptureNrvWait::sInstance);
}
void YoshiCapture::control() {
    if (mGround) {
        TPos3f pose;
        pose.set(mGround->mBaseMatrix);
        pose.concat(mGroundRelativePose);
        setPose(pose);
    }
    YoshiFunction::tryYoshiCapture(mLockOnTarget, mFixedDirection);
}
bool YoshiCapture::receiveOtherMsg(u32 msg, HitSensor*, HitSensor*) {
    if (MR::isMsgYoshiStartTongue(msg)) {
        setNerve(&NrvYoshiCapture::YoshiCaptureNrvCatch::sInstance);
        return true;
    }
    if (MR::isMsgYoshiReleaseEaten(msg)) {
        setNerve(&NrvYoshiCapture::YoshiCaptureNrvRelease::sInstance);
        return true;
    }
    return false;
}
// NONMATCHING: matrix rotation uses different floating-point registers and instruction scheduling.
void YoshiCapture::facePlayer() {
    TPos3f inverse;
    TPos3f pose;
    if (mGround) {
        inverse.set(mGround->mBaseMatrix);
        pose.set(mGround->mBaseMatrix);
        pose.concat(mGroundRelativePose);
    }
    else {
        inverse.set(getBaseMtx());
        pose.set(getBaseMtx());
    }
    inverse.invert(inverse);
    TVec3f front;
    pose.getZDir(front);
    TVec3f up;
    pose.getYDir(up);
    TVec3f position;
    pose.getTrans(position);
    TVec3f toPlayer(*MR::getPlayerPos());
    toPlayer.sub(position);
    f32 angle = MR::diffAngleSignedHorizontal(front, toPlayer, up);
    TPos3f rotation;
    rotation.makeRotate(TVec3f(0.0f, 1.0f, 0.0f), angle);
    pose.concat(rotation);
    if (mGround) mGroundRelativePose.concat(inverse, pose);
    else setPose(pose);
}
void YoshiCapture::setPose(const TPos3f& pose) {
    pose.getTrans(mPosition);
    pose.getEulerXYZ(mRotation);
    mRotation.x *= 57.2957763671875f;
    mRotation.y *= 57.2957763671875f;
    mRotation.z *= 57.2957763671875f;
}
YoshiCapture::~YoshiCapture() {}
