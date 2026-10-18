#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include "Game/Util/JointController.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include <JSystem/JGeometry/TVec.hpp>

class AnimScaleController;
class WalkerStateBindStarPointer;
class MapObjConnector;
class YoshiLockOnTarget;
class HitSensor;
class Nerve;

/// @brief Pyudon (a fuzzy enemy that swoons, is struck by HipDrop/Kick and comes back).
/// Layout from the SB4E01 ctor (LiveActor is 0x90 bytes in SMG2).
class Pyudon : public LiveActor {
public:
    Pyudon(const char*);
    virtual ~Pyudon();

    virtual void init(const JMapInfoIter&);
    virtual void makeActorAppeared();
    virtual void kill();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);
    virtual bool receiveMsgPush(HitSensor*, HitSensor*);

    void exeWait();
    void exeFind();
    void exeTrack();
    void exeLockOn();
    void exeAttack();
    void exeJump();
    void exeHipDropLand();
    void exeSwoon();
    void exeStarPieceReactionAir();
    void exeStarPieceReaction();
    void exeSwoonLand();
    void exeSwoonEnd();
    void exeRecovery();
    void exeRecoveryAir();
    void exeStepDown();
    void exeBlowDown();
    void exeDamage();
    void exeDamageAir();

    bool isHipDropNerve() const;
    bool isAttackNerve() const;
    bool isFindNerve() const;
    bool isBindMapObjNerve() const;
    void receiveMsgPlayerTrample(u32, HitSensor*);
    bool receiveMsgSpinAttack(u32, HitSensor*, HitSensor*);
    bool receiveMsgKick(u32, HitSensor*, HitSensor*);
    bool receiveMsgStarPiece(HitSensor*, HitSensor*);
    bool receiveMsgYoshiTongue();
    void updateBindStarPointer();
    void reboundVelocityFromTargetDir(f32, f32);
    void updateTargetPos();
    void updateVelocityOnGround();
    void updateClipping();
    void damperVelocity(f32, f32);
    LiveActor* tryFindGroundActor();
    bool isGroundActorAlive();
    void clearGroundActor();

    /* 0x90 */ AnimScaleController* mAnimScaleCtrl;
    /* 0x94 */ WalkerStateBindStarPointer* mBindStar1;
    /* 0x98 */ WalkerStateBindStarPointer* mBindStar2;
    /* 0x9C */ s32 mCounter;
    /* 0xA0 */ TVec3f mStartPos;
    /* 0xAC */ TVec3f mFront;
    /* 0xB8 */ TVec3f mBasePos;
    /* 0xC4 */ TVec3f mGroundPos;
    /* 0xD0 */ TVec3f mTargetPos;
    /* 0xDC */ f32 mSpeedRate;
    /* 0xE0 */ s8 mIsSwitchActive;
    /* 0xE4 */ LiveActor* mGroundActor;
    /* 0xE8 */ MapObjConnector* mConnector;
    /* 0xEC */ YoshiLockOnTarget* mYoshiLockOn;
};
