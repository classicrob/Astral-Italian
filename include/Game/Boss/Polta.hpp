#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include "Game/LiveActor/LiveActorGroup.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>

class ActorCameraInfo;
class BombTeresa;
class ModelObj;
class PoltaActionSequencer;
class PoltaArm;
class PoltaGroundRockHolder;
class PoltaRockHolder;
class PoltaSensorCtrl;

class BombTeresaHolder : public DeriveActorGroup< BombTeresa > {
public:
    /// @brief Creates a new `BombTeresaHolder`.
    BombTeresaHolder();

    void disperseAll();
};

/// @brief Bouldergeist
class Polta : public LiveActor {
public:
    /// @brief Creates a new `Polta`.
    /// @param pName A pointer to the null-terminated name of the object.
    Polta(const char* pName);

    virtual void init(const JMapInfoIter& rIter);
    virtual void calcAnim();
    virtual void makeActorAppeared();
    virtual void kill();
    virtual void makeActorDead();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver);

    void initArm();
    void initPartsModel();
    void initSensor();
    void initBombTeresaHolder();
    void initCamera(const JMapInfoIter&);
    void appearBreakModel(const TVec3f&, const char*);
    void appearBreakModelFirst(const TVec3f&);
    void appearBreakModelMiddle(const TVec3f&);
    void appearBreakModelLast(const TVec3f&);
    void appearStarPiece(s32);
    bool isEndBreakModel() const;
    void killBreakModel();
    void rotateToPlayer();
    f32 calcToPlayerRotateSpeed() const;
    void updateAction() NO_INLINE;
    void updatePose(f32, f32);
    void start();
    void setStartPose();

    /* 0x90 */ PoltaSensorCtrl* mSensorCtrl;
    /* 0x94 */ PoltaArm* mLeftArm;
    /* 0x98 */ PoltaArm* mRightArm;
    /* 0x9C */ PoltaActionSequencer* mSequencer;
    /* 0xA0 */ BombTeresaHolder* mBombTeresaHolder;
    /* 0xA4 */ PoltaRockHolder* mRockHolder;
    /* 0xA8 */ PoltaGroundRockHolder* mGroundRockHolder;
    /* 0xAC */ ActorCameraInfo* mCameraInfo;
    /* 0xB0 */ ModelObj* mBreakModel;
    /* 0xB4 */ ModelObj* mFormationModel;
    /* 0xB8 */ TQuat4f mPoseQuat;
    /* 0xC8 */ TVec3f mFront;
    /* 0xD4 */ TQuat4f mInitialQuat;
    /* 0xE4 */ TVec3f mInitialPos;
    /* 0xF0 */ f32 mTurnSpeed;
};
