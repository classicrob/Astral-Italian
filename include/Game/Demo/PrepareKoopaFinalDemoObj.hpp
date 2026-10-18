#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class PrepareKoopaFinalDemoObj : public LiveActor {
public:
    PrepareKoopaFinalDemoObj(const char*);

    virtual void init(const JMapInfoIter&);
    virtual void initAfterPlacement();
    virtual void control();
    virtual ~PrepareKoopaFinalDemoObj();

    void exeWait();
    void exeShake();
    void exeStopShake();
    void requestWorldMapAfterWipe(s32);
    void setRandomBckFrame(s32);

    /* 0x90 */ LiveActor* mKoopaSign;
    /* 0x94 */ LiveActor* mTalkActor;
    /* 0x98 */ LiveActor* mFinalKoopa;
};
