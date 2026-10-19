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
    /* 0x164 */ TakeOutStar* mTakeOutStar;
    /* 0x168 */ bool mAllCometsComplete;
    /* 0x16C */ s32 mFaceIndex;
};
