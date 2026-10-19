#pragma once

#include "Game/Enemy/JumpEmitter.hpp"

class RingBeam;
class AnimScaleController;
class WalkerStateBindStarPointer;

class JumpBeamer : public JumpEmitter {
public:
    void exePointBindWhileHopping();
    JumpBeamer(const char*);

    virtual void init(const JMapInfoIter&);
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);

    bool isStarPointerBindable() const;
    void syncSwitchOffB();
    void exeHide();
    void exeUp();
    void exeWait();
    void exeDown();
    void exeHopStart();
    void exeHopWait();
    void exeHopJump();
    void exeHopEnd();
    void exePreOpen();
    void exeOpen();
    void exeClose();
    void exeInter();

    /* 0xE4 */ AnimScaleController* mAnimScaleController;
    /* 0xE8 */ WalkerStateBindStarPointer* mBindStarPointer;
    /* 0xEC */ RingBeam** mBeams;
};
