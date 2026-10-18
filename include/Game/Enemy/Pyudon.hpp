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

    NO_INLINE void fn_801F3840();
    void exeFind();
    NO_INLINE void fn_801F3A70();
    NO_INLINE void fn_801F3C30();
    NO_INLINE void fn_801F3D70();
    NO_INLINE void fn_801F3DE0();
    NO_INLINE void fn_801F3F00();
    NO_INLINE void fn_801F3FB0();
    NO_INLINE void fn_801F4070();
    NO_INLINE void fn_801F40F0();
    NO_INLINE void fn_801F41F0();
    void exeSwoonEnd();
    NO_INLINE void fn_801F4310();
    NO_INLINE void fn_801F43B0();
    void exeStepDown();
    NO_INLINE void fn_801F44E0();
    void exeDamage();
    NO_INLINE void fn_801F4610();

    bool isHipDropNerve() const;
    bool isAttackNerve() const;
    NO_INLINE bool fn_801F4E10() const;
    bool isBindMapObjNerve() const;
    NO_INLINE bool fn_801F5000(u32, HitSensor*, HitSensor*);
    NO_INLINE bool fn_801F5150(u32, HitSensor*, HitSensor*);
    NO_INLINE bool fn_801F5270(u32, HitSensor*, HitSensor*);
    NO_INLINE bool fn_801F5330(HitSensor*, HitSensor*);
    bool receiveMsgYoshiTongue();
    NO_INLINE bool fn_801F5540();
    NO_INLINE void fn_801F5650(f32, f32);
    NO_INLINE void fn_801F5750();
    NO_INLINE void fn_801F5810();
    NO_INLINE void fn_801F58A0();
    NO_INLINE void fn_801F5920(LiveActor*, f32, f32);
    NO_INLINE bool fn_801F59D0();
    NO_INLINE bool fn_801F5A20();
    NO_INLINE bool fn_801F5AA0();

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
