#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class TalkMessageCtrl;

class MiniStarCheckPoint : public LiveActor {
public:
    MiniStarCheckPoint(const char* pName) : LiveActor(pName) {}
    virtual ~MiniStarCheckPoint();
    virtual void init(const JMapInfoIter&);
    virtual void makeActorAppeared();
    virtual void makeActorDead();
    virtual void kill();
    virtual void control();
    virtual void calcAndSetBaseMtx();

    s32 mKind;                // 0x90
    s32 mStarNum;             // 0x94
    s32 mUnk98;               // 0x98
    s32 mUnk9C;               // 0x9C
    s32 mUnkA0;               // 0xA0
    LiveActor* mChild;        // 0xA4
    TalkMessageCtrl* mTalkCtrl;  // 0xA8
    bool mIsDone;             // 0xAC
};
