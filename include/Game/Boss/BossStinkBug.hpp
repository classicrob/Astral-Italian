#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>

class ActorCameraInfo;
class ActorJointCtrl;
class BossStinkBugActionSequencer;
class BossStinkBugBombHolder;
class CollisionParts;
class GroundChecker;
class PartsModel;

class BossStinkBug : public LiveActor {
public:
    enum SeVersion {
        SeVersion_NORMAL = 0,
        SeVersion_WATER = 1,
        SeVersion_ANGRY = 2,
        SeVersion_ANGRY_WATER = 3,
    };

    BossStinkBug(const char*);

    virtual void init(const JMapInfoIter&);
    virtual void calcAnim();
    virtual void makeActorAppeared();
    virtual void kill();
    virtual void makeActorDead();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver);

    void start();
    void initCamera(const JMapInfoIter&);
    void initSensor();
    void initCollision();
    void disposeBomb();
    void reuestMovementOnParts();
    LiveActor* getWingModel();
    LiveActor* getBombLauncher();
    void calcUpVec(TVec3f*) const;
    void updateAction();
    void updatePose();
    void setPose(MtxPtr);
    void updateCamera();
    bool isValidFollowId(s32) const;
    void validateCollisionGround();
    void validateCollisionFly();
    void validateCollisionHitOnly();
    void invalidateCollision();
    void onBindLeg();
    void offBindLeg();
    bool isLegInWater() const;
    void updateJointControl();
    void appearStarPiece(s32);
    void startEventCamera(const char*);
    void endEventCamera(const char*);
    bool isSensorBody(const HitSensor*) const;
    bool throwBomb(f32, f32);

    /* 0x90 */ CollisionParts** mCollisionParts;
    /* 0x94 */ GroundChecker** mLegGroundCheckers;
    /* 0x98 */ BossStinkBugBombHolder* mBombHolder;
    /* 0x9C */ ActorJointCtrl* mJointCtrl;
    /* 0xA0 */ TPos3f mFollowMtx;
    /* 0xD0 */ ActorCameraInfo* mCameraInfo;
    /* 0xD4 */ BossStinkBugActionSequencer* mActionSequencer;
    /* 0xD8 */ PartsModel* mWingModel;
    /* 0xDC */ PartsModel* mBombLauncher;
    /* 0xE0 */ TQuat4f mPoseQuat;
    /* 0xF0 */ TVec3f mFront;
    /* 0xFC */ TVec3f mCameraTargetPos;
    /* 0x108 */ s32 mCameraRegisterId;
    /* 0x10C */ f32 mTurnRate;
    /* 0x110 */ f32 mPoseBlendRate;
    /* 0x114 */ bool mIsPoseUpdated;
    /* 0x115 */ bool mAreLegsBound;
};
