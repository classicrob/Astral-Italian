#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class DemoObj;
class DemoAstroPlanet;

class NormalEndingDemoObj : public LiveActor {
public:
    NormalEndingDemoObj(const char*);

    virtual void init(const JMapInfoIter&);
    virtual void initAfterPlacement();
    virtual void kill();
    virtual ~NormalEndingDemoObj();

    void exeA();
    void exeB();
    void exeC();
    void checkStaffRollStart();
    LiveActor* createDemoModel(const char*, const char*, MtxPtr);
    LiveActor* createDemoModel(const char*, const char*, const char*, MtxPtr);
    LiveActor* createDemoModelWithAnimArc(const char*, const char*, const char*, MtxPtr);
    LiveActor* createDemoModel(const char*, const char*);
    static void setEndingAreaIndex(s32);

    /* 0x90 */ s32 mField90;
    /* 0x94 */ LiveActor* mActorA;
    /* 0x98 */ LiveActor* mActorB;
    /* 0x9C */ LiveActor* mActorC;
    /* 0xA0 */ LiveActor* mStar;
    /* 0xA4 */ LiveActor* mActorD;
    /* 0xA8 */ LiveActor* mActorE;
    /* 0xAC */ LiveActor* mActorF;
    /* 0xB0 */ LiveActor* mActorG;
    /* 0xB4 */ LiveActor* mActorH;
    /* 0xB8 */ DemoAstroPlanet* mPlanet;
    /* 0xBC */ LiveActor* mActorI;
    /* 0xC0 */ LiveActor* mActorJ;
};
