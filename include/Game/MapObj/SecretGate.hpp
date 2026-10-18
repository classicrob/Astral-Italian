#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include "Game/LiveActor/LiveActorGroup.hpp"

class CollectCounter;

class SecretGate : public LiveActor {
public:
    SecretGate(const char*);

    virtual ~SecretGate();
    virtual void init(const JMapInfoIter&);
    virtual void attackSensor(HitSensor*, HitSensor*);

    void exePass();
    bool isNerveWaitAfterPass() const;

    /* 0x90 */ f32 mScaleA;
    /* 0x94 */ f32 mScaleB;
    /* 0x98 */ bool mIsOffsetUp;
    /* 0x9C */ TVec3f mUnk9C;
    /* 0xA8 */ TVec3f mEffectTrans;
};

class SecretGateGroup : public LiveActorGroup {
public:
    SecretGateGroup(const char*, int);

    virtual ~SecretGateGroup();
};

class SecretGateCounter : public LiveActor {
public:
    SecretGateCounter(const char*);

    virtual ~SecretGateCounter();
    virtual void init(const JMapInfoIter&);

    void exeCount();
    void exeMovementOn();

    /* 0x90 */ LiveActorGroup* mGroup;
    /* 0x94 */ s32 mPassCount;
    /* 0x98 */ CollectCounter* mCounter;
};
