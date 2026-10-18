#pragma once
#include "Game/NPC/NPCActor.hpp"
class TakeOutStar;
class Moc : public NPCActor {
public:
    Moc(const char*);
    virtual ~Moc();
    virtual void init(const JMapInfoIter&);
    bool branchFunc(u32);
    bool eventFunc(u32);
    TakeOutStar* mTakeOutStar;
    bool mAllCometsComplete;
    s32 mFaceIndex;
};
