#pragma once
#include "Game/NPC/NPCActor.hpp"

class TakeOutStar;

class PlayAttackMan : public NPCActor {
public:
    PlayAttackMan(const char*);
    virtual ~PlayAttackMan();
    virtual void init(const JMapInfoIter&);
    static void initialize(NPCActor*, const JMapInfoIter&, bool);

    bool eventFunc(u32);
    TakeOutStar* mTakeOutStar;
};
