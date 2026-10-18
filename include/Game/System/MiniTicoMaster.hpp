#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class TalkMessageCtrl;

class MiniTicoMaster : public LiveActor {
public:
    MiniTicoMaster(const char* pName) : LiveActor(pName) {}
    virtual ~MiniTicoMaster();
    virtual void init(const JMapInfoIter&);
    virtual void makeActorAppeared();
    virtual void makeActorDead();
    virtual void control();

    s32 mUnk90;                  // 0x90
    s32* mUnk94;                 // 0x94 (*mUnk94 is the count)
    s32 mUnk98;                  // 0x98
    s32 mUnk9C;                  // 0x9C
    s32 mUnkA0;                  // 0xA0
    s32 mUnkA4;                  // 0xA4
    s32 mUnkA8;                  // 0xA8
    LiveActor* mModelA;          // 0xAC
    LiveActor* mModelB;          // 0xB0
    LiveActor* mChild;           // 0xB4
    TalkMessageCtrl* mTalkCtrl;  // 0xB8
    LiveActor* mShadow;          // 0xBC
};
