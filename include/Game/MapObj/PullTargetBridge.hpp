#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include "Game/LiveActor/PartsModel.hpp"
#include "JSystem/JGeometry/TMatrix.hpp"

class YoshiLockOnTarget;
class JointController;

// SMG2-only. Small helper (0x10) that switches the host between "Wait" and "PullStart" while it is pulled.
class PullTargetAnim {
public:
    PullTargetAnim(LiveActor* pHost);

    void reset();
    void update(f32 dist);

    /* 0x0 */ LiveActor* mHost;
    /* 0x4 */ f32 mPrevDist;
    /* 0x8 */ s32 mStillCount;
    /* 0xC */ bool mIsPulling;
};

// SMG2-only. Yoshi tongue target attached to a host actor (PartsModel "PullTarget").
class PullTarget : public PartsModel {
public:
    PullTarget(LiveActor* pHost);

    virtual ~PullTarget() {}
    virtual void control();
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);

    bool isFacingPlayer();

    /* 0x9F */ bool mIsEnable;
    /* 0xA0 */ YoshiLockOnTarget* mLockOnTarget;
};

class PullTargetBridge : public LiveActor {
public:
    PullTargetBridge(const char* pName);

    virtual ~PullTargetBridge() {}
    virtual void init(const JMapInfoIter& rIter);
    virtual void control();
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);

    void exePull();
    bool tryPull();

    /* 0x90 */ TVec3f mClippingCenter;
    /* 0x9C */ PullTarget* mPullTarget;
    /* 0xA0 */ TVec3f mPlayerPos;
    /* 0xAC */ f32 mPullDist;
    /* 0xB0 */ PullTargetAnim* mAnim;
};

class CollisionParts;
class JointControllerInfo;

class PullTargetDrawer : public LiveActor {
public:
    PullTargetDrawer(const char* pName);

    virtual ~PullTargetDrawer() {}
    virtual void init(const JMapInfoIter& rIter);
    virtual void calcAndSetBaseMtx();
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);

    void exeWait();
    void exePull();
    void exeExtend();
    void exeExtendWait();
    void exeShrink();
    bool calcJointBridge(TPos3f*, const JointControllerInfo&);
    void updateClipping(const TVec3f&);
    void initCollision();
    bool tryPull();
    void updateCollisionMtx();
    void updateCollisionValid();
    void pushPlayerFromBridge();

    /* 0x90 */ TVec3f mClippingCenter;
    /* 0x9C */ PullTarget* mPullTarget;
    /* 0xA0 */ TVec3f mPlayerPos;
    /* 0xAC */ f32 mPullDist;
    /* 0xB0 */ JointController* mJointCtrl;
    /* 0xB4 */ f32 mMaxLength;
    /* 0xB8 */ f32 mLength;
    /* 0xBC */ s32 mValidIndex;
    /* 0xC0 */ CollisionParts** mParts;
    /* 0xC4 */ s32 mPartsNum;
    /* 0xC8 */ TPos3f* mPartsMtx;
    /* 0xCC */ s32 mShrinkTime;
    /* 0xD0 */ PullTargetAnim* mAnim;
};
