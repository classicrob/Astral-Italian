#pragma once
#include "Game/NPC/NPCActor.hpp"
class MameMuimuiScorer;
class TakeOutStar;
class MameMuimuiAttackMan : public NPCActor {
public:
    MameMuimuiAttackMan(const char*);
    virtual ~MameMuimuiAttackMan();
    virtual void init(const JMapInfoIter&);
    void facePlayerAfterGame();
    bool branchFunc(u32);
    bool eventFunc(u32);
    bool animeFunc(u32);
    MameMuimuiScorer* mScorer;
    TakeOutStar* mTakeOutStar;
};
