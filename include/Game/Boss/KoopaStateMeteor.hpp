#pragma once

#include "Game/LiveActor/ActorStateBase.hpp"

class KoopaBattle;

// SMG2 final Bowser: meteor attack (casts the attack rocks). Not derived from KoopaStateBase.
class KoopaStateMeteor : public ActorStateBase< KoopaBattle > {
public:
    KoopaStateMeteor(KoopaBattle*);

    virtual ~KoopaStateMeteor();
    virtual void appear();

    void setMode(s32);
    void exeStart();
    void exeLoop();
    void exeEnd();

    /* 0x10 */ s32 mMode;
    /* 0x14 */ s32 mCastInterval;
    /* 0x18 */ s32 mEndStep;
    /* 0x1C */ s32 mEndFrameMax;
};
