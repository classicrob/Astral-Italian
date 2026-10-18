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
    s32 _164;
    bool mHideAfterIntrusiveUnlock;
};
