#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class AnimScaleController;
class WalkerStateBindStarPointer;
class SpinHitController;

class SamboHead : public LiveActor {
public:
    SamboHead(const char*);
    static void updateBoundVelocity(LiveActor*);

    virtual ~SamboHead();
    virtual void init(const JMapInfoIter&);
    virtual void appear();
    virtual void kill();
    virtual void control();
    bool isNerveTypeDead() const;
    bool isStarPointerBindable() const;
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPush(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);

    void exeWaitUnderGround();
    void exeHide();
    void exeAppear();
    void exeWait();
    void exePrePursue();
    void exePursue();
    void exePursueEnd();
    void exeHitToPlayer();
    void exeStarPieceHit();
    void exeStampFall();
    void endStampFall();
    void exeStampDeath();
    void endStampDeath();
    void exeHitBlow();
    void endHitBlow();
    void exeBuryDeath();
    void exeBindStarPointer();
    void endBindStarPointer();
    void exeBindStarPointerEnd();

    /* 0x8C */ SpinHitController* mSpinCtrl;
    /* 0x90 */ AnimScaleController* mScaleController;
    /* 0x94 */ WalkerStateBindStarPointer* mBindStarPointer;
    /* 0x98 */ TVec3f mFront;
    /* 0xA4 */ TVec3f mHomePos;
    /* 0xB4 */ bool mAlwaysPursue;
};
