#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include "Game/NameObj/NameObj.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>
#include <JSystem/JGeometry/TVec.hpp>

class AnimScaleController;
class FixedPosition;
class YoshiLockOnTarget;

// Counts how long the Popo stays far away from its home position.
class PopoAwayCounter {
public:
    PopoAwayCounter(LiveActor* pHost) : mHost(pHost), mCount(0) {
    }

    void update(const TVec3f&);  // fn_801EF410

    /* 0x00 */ LiveActor* mHost;
    /* 0x04 */ u32 mCount;
};

// SMG2-only unit. Offsets are SMG2 (LiveActor = 0x90). Written from the target asm.
class Popo : public LiveActor {
public:
    Popo(const char* pName);

    virtual ~Popo();
    virtual void init(const JMapInfoIter&);
    virtual void initAfterPlacement();
    virtual void appear();
    virtual void kill();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPush(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);

    void exeWait();         // fn_801ECDE0
    void exeFind();         // fn_801ECE70
    void exeWalk();         // fn_801ECF20
    void exeEscape();       // fn_801ED220
    void exeStop();         // fn_801ED2F0
    void exeRest();         // fn_801ED370
    void exeReturn();       // fn_801ED430
    void exePressDown();    // fn_801ED600
    void exeBlowDown();     // fn_801ED670
    void exeRecover();      // fn_801ED760
    void exeStick();        // fn_801ED7F0
    void exeStickStart();   // fn_801EDC10
    void exePressDownWait();  // fn_801EDC80
    void exeReaction();     // fn_801EDCE0
    void exeKicked();       // fn_801EDD50
    void exeTouch();        // fn_801EDE00
    void exeStickCancel();  // fn_801EDE90
    void exeStickWait();    // inlined in nerve
    void exeLick();         // inlined in nerve
    void endWait();         // inlined in nerve
    void endReturn();       // inlined in nerve
    void endTouch();        // fn_801EF6F0

    void tryShootOut() NO_INLINE;         // fn_801EDFF0
    void tryReaction() NO_INLINE;         // fn_801EE0D0
    bool isFree() const NO_INLINE;        // fn_801EE840
    void calcFloatVelocity() NO_INLINE;   // fn_801EE920
    void calcTurnVelocity(f32) NO_INLINE;  // fn_801EEA90
    f32 clampVelocity(f32) NO_INLINE;  // fn_801EEB20
    bool isStickable() const NO_INLINE;   // fn_801EEB50
    bool isAttackable() const NO_INLINE;  // fn_801EEBF0
    void tryReleaseYoshi() NO_INLINE;     // fn_801EECA0
    bool isBlowable() const NO_INLINE;    // fn_801EED00
    void calcBlowVelocity(s32, s32) NO_INLINE;  // fn_801EEDF0
    bool isBehind() const NO_INLINE;      // fn_801EEF10
    void calcReturnVelocity() NO_INLINE;   // fn_801EEF70
    void decelerate() NO_INLINE;          // fn_801EF020
    bool tryFindYoshi() NO_INLINE;        // fn_801EF050
    bool isStarPointerTargetable() const NO_INLINE;  // fn_801EF100
    void startBlowDown(const TVec3f&, bool) NO_INLINE;  // fn_801EF1F0
    void calcBlowDownVelocity() NO_INLINE;  // fn_801EF280
    bool isStarPieceReactable() const NO_INLINE;  // fn_801EF310

    /* 0x90 */ TVec3f mFrontVec;
    /* 0x9C */ u8 mReserved9C[0x10];
    /* 0xAC */ YoshiLockOnTarget* mYoshiLockOn;
    /* 0xB0 */ PopoAwayCounter* mAwayCounter;
    /* 0xB4 */ TVec3f mHomePos;
    /* 0xC0 */ AnimScaleController* mScaleController;
    /* 0xC4 */ TVec3f mLockOnOffset;
    /* 0xD0 */ bool mIsMarioBlow;
};

class PopoStickingManager : public NameObj {
public:
    PopoStickingManager(const char* pName);

    virtual ~PopoStickingManager();
    virtual void init(const JMapInfoIter&);
    virtual void initAfterPlacement();
    virtual void calcAnim();

    bool trySetStickPopo(Popo*) NO_INLINE;   // fn_801EFA10
    void releaseStickPopo(Popo*) NO_INLINE;  // fn_801EFA30
    bool isStick() const NO_INLINE;          // fn_801EFA50
    TPos3f* getMouthMtx() NO_INLINE;         // fn_801EFA60
    void tryShootOut() NO_INLINE;            // fn_801EFA70
    void tryReaction() NO_INLINE;            // fn_801EFA90

    /* 0x14 */ FixedPosition* mMouthPosition;
    /* 0x18 */ Popo* mStickPopo;
};

bool isPopoSticking();
void tryShootStickingPopo();
void tryReactStickingPopo();
