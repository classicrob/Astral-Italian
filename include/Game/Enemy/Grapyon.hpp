#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>
#include <JSystem/JGeometry/TVec.hpp>

class AnimScaleController;
class HitSensor;
class PartsModel;
class WalkerStateBindStarPointer;

/// SMG2-only enemy. Offsets are SMG2 (LiveActor = 0x90). Written from the target asm.
class Grapyon : public LiveActor {
public:
    Grapyon(const char*);

    virtual ~Grapyon();
    virtual void init(const JMapInfoIter&);
    virtual void initAfterPlacement();
    virtual void makeActorAppeared();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveMsgEnemyAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveMsgPush(HitSensor*, HitSensor*);

    void exeWait();       // fn_80184F10
    void exeJump();       // fn_80184FF0
    void exeFall();       // fn_801851C0
    void exeLand();       // fn_80185370
    void exeGuard();      // fn_801854A0
    void exeAttackHit();  // fn_80185560
    void exeDown();       // fn_80185620
    void exeBlowDown();   // fn_801856D0
    void exeDead();       // inlined in nerve (fn_80186360)
    void exeSupportFreeze();  // inlined in nerve (fn_80186310)

    void calcMove();                // fn_80185C70
    bool isNormalNerve() const;     // fn_80185D10
    bool isSwitchChanged() const;   // fn_80185DC0
    bool isJumpStartNerve() const;  // fn_80185E20
    bool trySwitchFall();           // fn_80185E80
    void updateHeadBtp();           // fn_80185ED0
    bool tryGuard(HitSensor*, HitSensor*);  // fn_80185F10
    bool tryAttackHit();            // fn_80185FB0
    bool tryDown();                 // fn_80186050
    bool tryElectric();             // fn_801860C0
    bool tryBlowDown(HitSensor*, HitSensor*);  // fn_80186120

    /* 0x90 */ PartsModel* mHead;
    /* 0x94 */ TPos3f mMtx;
    /* 0xC4 */ AnimScaleController* mAnimScaleCtrl;
    /* 0xC8 */ WalkerStateBindStarPointer* mBindStar;
    /* 0xCC */ TVec3f mHomePos;
    /* 0xD8 */ TVec3f mFront;
    /* 0xE4 */ TVec3f mUp;
    /* 0xF0 */ f32 mScale;
    /* 0xF4 */ f32 mDirection;
    /* 0xF8 */ f32 mDirectionInit;
    /* 0xFC */ bool mSwitchState;
};
