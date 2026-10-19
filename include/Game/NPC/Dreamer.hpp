#pragma once

#include "Game/NPC/NPCActor.hpp"

class DreamerButtonGuidance;

class Dreamer : public NPCActor {
public:
    Dreamer(const char*);
    virtual ~Dreamer();

    virtual void init(const JMapInfoIter&);
    virtual void appear();
    virtual void makeActorAppeared();
    virtual void control();

    void exeAppear();
    void exeStartHelp();
    void exeInHelp();
    void exeEndHelp();

    void initForLuigi(const JMapInfoIter&, NPCActorCaps&);
    void initForPlaying(const JMapInfoIter&, NPCActorCaps&);
    void initForNpc(const JMapInfoIter&, NPCActorCaps&);
    void initButtonGuidance();
    bool isMakeAppear() const;
    bool eventFunc(u32);

    /* 0x164 */ DreamerButtonGuidance* mButtonGuidance;
    /* 0x168 */ s32 mStartId;
    /* 0x16C */ s32 mMissNum;
    /* 0x170 */ bool mHasEndMessage;
    /* 0x174 */ s32 mUnusedCounter0;
    /* 0x178 */ s32 mUnusedCounter1;
    /* 0x17C */ bool mIsLuigi;
};
