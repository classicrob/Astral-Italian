#pragma once
#include "Game/LiveActor/LiveActor.hpp"
class YoshiLockOnTarget;
class LightFruit : public LiveActor {
public:
    LightFruit(const char*);
    virtual ~LightFruit();
    virtual void init(const JMapInfoIter&);
    virtual void control();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);
    void exeAppear();
    void exeWait();
    void exeReaction();
    void exeHipDropCrash();
    void exeHide();
    bool isEatable() const;
    bool isPushable() const;
    TQuat4f mOriginalRotation;
    TVec3f mOriginalPosition;
    const char* mReaction;
    YoshiLockOnTarget* mYoshiTarget;
};
