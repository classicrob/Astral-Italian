#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>

class TalkMessageCtrl;

class MeetKoopaDemoObj : public LiveActor {
public:
    MeetKoopaDemoObj(const char*);

    virtual ~MeetKoopaDemoObj();
    virtual void init(const JMapInfoIter&);
    virtual void kill();

    void exeWait();
    void exeFound();
    void exeFallObjects();
    void exeDemo();
    void exeWaitBack();

    /* 0x90 */ LiveActor* mKoopa;
    /* 0x94 */ LiveActor* mKoopaArm;
    /* 0x98 */ LiveActor* mPeach;
    /* 0x9C */ LiveActor* mTicoBaby;
    /* 0xA0 */ LiveActor** mFallObjs;
    /* 0xA4 */ TalkMessageCtrl* mTalkCtrl;
    /* 0xA8 */ TPos3f mMtx;
};
