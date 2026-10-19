#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include "Game/NameObj/NameObj.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>

class ActorCameraInfo;
class CameraTargetMtx;
class JointController;
struct JointControllerInfo;
class PetitPorter;
class PetitPorterExitPoint;
class PetitPorterWarpPoint;

class PetitPorter : public LiveActor {
public:
    PetitPorter(const char*);

    virtual ~PetitPorter();
    virtual void init(const JMapInfoIter&);
    virtual void makeActorAppeared();
    virtual void calcAndSetBaseMtx();
    virtual void updateHitSensor(HitSensor*);
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);

    void exeAppear();
    void exeWait();
    void exeEnterDemo();
    void exeEnterDemoEnd();
    void exeLeaveDemo();
    void exeLeaveWait();
    void exeLeaveCamera();
    void exeLeaveCameraEnd();
    void exeLeaveFinish();

    bool calcJointMtx(TPos3f*, const JointControllerInfo&);
    bool isDemo() const;

    /* 0x90 */ TPos3f mBaseMtx;
    /* 0xC0 */ LiveActor* mExitPoint;
    /* 0xC4 */ LiveActor* mWarpPoint;
    /* 0xC8 */ s32 mStepLimit;
    /* 0xCC */ f32 mAngle;
    /* 0xD0 */ f32 mAngleSpeed;
    /* 0xD4 */ LiveActor* mBindTarget;
    /* 0xD8 */ TPos3f mMtxD8;
    /* 0x108 */ TPos3f mMtx108;
    /* 0x138 */ ActorCameraInfo* mCameraInfo;
    /* 0x13C */ CameraTargetMtx* mCameraTargetMtx;
    /* 0x140 */ JointController* mJointController;
    /* 0x144 */ const char* mSavedStageBgmName;
    /* 0x148 */ s32 mBgmHandle2;
};

class PetitPorterWarpPoint : public LiveActor {
public:
    PetitPorterWarpPoint(const char*);
    virtual ~PetitPorterWarpPoint();

    virtual void init(const JMapInfoIter&);
    virtual MtxPtr getBaseMtx() const;

    /* 0x90 */ TPos3f mBaseMtx;
};

class PetitPorterExitPoint : public LiveActor {
public:
    PetitPorterExitPoint(const char*);
    virtual ~PetitPorterExitPoint();

    virtual void init(const JMapInfoIter&);
    virtual MtxPtr getBaseMtx() const;

    /* 0x90 */ TPos3f mBaseMtx;
};

struct PetitPorterLink {
    /* 0x00 */ PetitPorter* mPorter;
    /* 0x04 */ PetitPorterExitPoint* mExitPoint;
    /* 0x08 */ PetitPorterWarpPoint* mWarpPoint;
    /* 0x0C */ s32 mId;
};

class PetitPorterMediator : public NameObj {
public:
    PetitPorterMediator();

    virtual ~PetitPorterMediator();
    virtual void initAfterPlacement();

    void registerPorter(PetitPorter*, PetitPorterExitPoint*, PetitPorterWarpPoint*, const JMapInfoIter&);
    bool isAnyPorterDemo() const;

    /* 0x14 */ s32 mCount;
    /* 0x18 */ PetitPorterLink* mLinks;
};

namespace PetitPorterFunction {
    bool isDemo();
};  // namespace PetitPorterFunction
