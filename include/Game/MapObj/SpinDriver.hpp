#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>
class SpinDriverCamera;
class SpinDriverShootPath;

class SpinDriver : public LiveActor {
public:
    SpinDriver(const char*);

    virtual void init(const JMapInfoIter&);
    virtual void initAfterPlacement();
    virtual void appear();
    virtual void makeActorAppeared();
    virtual void makeActorDead();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);

    void initParamFromJMapInfo(const JMapInfoIter&);
    void initShootPath(const JMapInfoIter&);
    void initEventCamera(const JMapInfoIter&);
    bool tryStartShoot();
    bool tryEndCapture();
    bool tryShoot();
    bool tryEndShoot();
    bool tryForceCancel();
    bool tryEndCoolDown();
    bool trySwitchOff();

    void exeTryDemo();
    void exeNonActive();
    void exeAppear();
    void exeWait();
    void exeCapture();
    void exeShootStart();
    void exeShoot();
    void exeCoolDown();

    bool startBind(HitSensor*);
    void cancelBind();
    void updateBindPosition();
    void moveBindPosToCenter();
    void updateBindActorMatrix(f32);
    void calcBindActorMatrix();
    void calcParabolicBindPose();
    void turnBindHead(const TVec3f&, f32);
    void calcShootMotionTime();
    void startCamera();
    void updateCamera();
    void cancelCamera() NO_INLINE;
    void endCamera() NO_INLINE;
    bool canStartBind() const NO_INLINE;
    bool canBind(HitSensor*) const;
    bool isValidFollow(s32) const;

    /* 0x90 */ LiveActor* mBindActor;
    /* 0x94 */ SpinDriverShootPath* mShootPath;
    /* 0x98 */ SpinDriverCamera* mSpinDriverCamera;
    /* 0x9C */ TPos3f mFollowMtx;  // new in SMG2 (0x30 bytes), base matrix followed by the bind target
    /* 0xCC */ TQuat4f mBindStartQuat;
    /* 0xDC */ TQuat4f mBindQuat;
    /* 0xEC */ TVec3f mBindPos;
    /* 0xF8 */ TVec3f mBindVelocity;
    /* 0x104 */ TVec3f mFrontDir;
    /* 0x110 */ TVec3f mSideDir;
    /* 0x11C */ TVec3f mUpDir;
    /* 0x128 */ TVec3f mBindStartPos;
    /* 0x134 */ f32 mJumpSpeed;
    /* 0x138 */ f32 mRotateYAngle;
    /* 0x13C */ f32 mRotateYSpeed;
    /* 0x140 */ TVec3f mShadowCenter;
    /* 0x14C */ f32 mClippingRadius;
    /* 0x150 */ f32 mBindRate;
    /* 0x154 */ f32 mShadowLength;
    /* 0x158 */ s32 mShootFrames;
    /* 0x15C */ s32 mFlyLoopFrame;
    /* 0x160 */ s32 mFlyEndFrame;
    /* 0x164 */ s32 mFallFrame;
    /* 0x168 */ f32 mInitPoseParam;
    /* 0x16C */ bool mIsCaptureEnabled;
    /* 0x170 */ s32 mRushTimer;
    /* 0x174 */ bool mIsRushActive;
    /* 0x175 */ bool mIsCaptureArmed;
    /* 0x176 */ bool mIsSilentAppear;
};
