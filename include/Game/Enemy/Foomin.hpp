#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class SupportTicoStop;

// SMG2-only unit (Foomin, rail-riding tico-carrier enemy). LiveActor = 0x90 bytes.
class Foomin : public LiveActor {
public:
    Foomin(const char* pName);

    virtual ~Foomin();
    virtual void init(const JMapInfoIter&);
    virtual void makeActorAppeared();
    virtual void control();
    virtual void calcAndSetBaseMtx();

    void exeWait();
    void exeWaitTouch();
    void exeAttackSign();
    void exeAttack();

public:
    /* 0x90 */ SupportTicoStop* mSupportTico;
    /* 0x94 */ TVec3f mFront;
    /* 0xA0 */ s32 mArg1;
    /* 0xA4 */ s32 mArg0;
};
