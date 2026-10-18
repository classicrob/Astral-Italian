#pragma once
#include "Game/NPC/NPCActor.hpp"
class TogepinScorer;
class TakeOutStar;
class TogepinAttackMan : public NPCActor {
public:
    TogepinAttackMan(const char*);
    virtual ~TogepinAttackMan();
    virtual void init(const JMapInfoIter&);
    virtual void control();
    bool branchFunc(u32);
    bool eventFunc(u32);
    bool animeFunc(u32);
    TogepinScorer* mScorer;
    TakeOutStar* mTakeOutStar;
    TalkMessageCtrl* mFoulTalkCtrl;
    bool mFoulTalkStarted;
};
