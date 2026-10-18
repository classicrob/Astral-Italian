#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class SupportTicoStop;

// SMG2-only unit (Foomin, rail-riding tico-carrier enemy). LiveActor = 0x90 bytes.
class Foomin : public LiveActor {
public:
    // SMG2-only functions (original names unknown; named by address)
    void fn_80181940();
    void fn_801819D0();
    void fn_80181A80();
    void fn_80181B00();
    void fn_80181C80();
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
