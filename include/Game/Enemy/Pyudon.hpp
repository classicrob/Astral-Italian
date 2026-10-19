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

    NO_INLINE void exePyudonWait();
    void exeFind();
    NO_INLINE void exeTrack();
    NO_INLINE void exePyudonLockOn();
    NO_INLINE void exePyudonAttack();
    NO_INLINE void exePyudonHipDrop();
    NO_INLINE void exeHipDropLand();
    NO_INLINE void exePyudonSwoon();
    NO_INLINE void exeStarPieceReactionAir();
    NO_INLINE void exePyudonStarPieceReaction();
    NO_INLINE void exePyudonSwoonLand();
    void exeSwoonEnd();
    NO_INLINE void exeRecovery();
    NO_INLINE void exeRecoveryAir();
    void exeStepDown();
    NO_INLINE void exePyudonBlowDown();
    void exeDamage();
    NO_INLINE void exeDamageAir();

    bool isHipDropNerve() const;
    bool isAttackNerve() const;
    NO_INLINE bool isStarPointBindable() const;
    bool isBindMapObjNerve() const;
    NO_INLINE bool tryStepDown(u32, HitSensor*, HitSensor*);
    NO_INLINE bool tryBlowDownSpin(u32, HitSensor*, HitSensor*);
    NO_INLINE bool tryBlowDownKick(u32, HitSensor*, HitSensor*);
    NO_INLINE bool tryPyudonStarPieceReaction(HitSensor*, HitSensor*);
    bool receiveMsgYoshiTongue();
    NO_INLINE bool tryStartStarPointBind();
    NO_INLINE void updateVelocityToTarget(f32, f32);
    NO_INLINE void updateTargetPos();
    NO_INLINE void updateBlownVelocity();
    NO_INLINE void updatePyudonClipping();
    NO_INLINE void dampVelocity(LiveActor*, f32, f32);
    NO_INLINE bool updateGroundActor();
    NO_INLINE bool tryReleaseGroundActor();
    NO_INLINE bool releaseGroundActor();

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
    /* 0xE0 */ u8 mIsSwitchActive;
    /* 0xE4 */ LiveActor* mGroundActor;
    /* 0xE8 */ MapObjConnector* mConnector;
    /* 0xEC */ YoshiLockOnTarget* mYoshiLockOn;
};
