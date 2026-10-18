#pragma once

#include "Game/NPC/NPCActor.hpp"

class Rabbit : public NPCActor {
public:
    enum SeVersion { SeVersion_NORMAL = 0, SeVersion_WATER = 1 };
    enum Behavior { Behavior_Unknown = 0, Behavior_UpJump = 1, Behavior_LongJump = 2 };
    Rabbit(const char*);
    virtual ~Rabbit();
    virtual void init(const JMapInfoIter&);
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void makeActorAppeared();
    virtual bool isReactionNerve() const;
    void exeAppear();
    void exeAppearLand();
    void exeReaction();
    void exeTalk();
    void exeJumpV();
    void exeJumpH();
    /* 0x164 */ s32 mBehavior;
    /* 0x168 */ f32 _168;
};
