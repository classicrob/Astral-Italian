#pragma once
#include "Game/NPC/NPCActor.hpp"
class LuigiTalkNpc : public NPCActor {
public:
    LuigiTalkNpc(const char*);
    virtual ~LuigiTalkNpc();
    virtual void init(const JMapInfoIter&);
    virtual void makeActorAppeared();
    virtual void control();
    void exeTurnPlayer();
    void exeInformation();
    void dummyDemoFunc();
    bool eventFunc(u32);
    bool branchFuncTalkNpc(u32);
    bool branchFuncIntrusively(u32);
    void pushTurnPlayer();
    /* 0x164 */ s32 mUnusedCounter;
    /* 0x168 */ bool mHideAfterIntrusiveUnlock;
};
