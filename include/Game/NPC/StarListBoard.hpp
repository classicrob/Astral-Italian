#pragma once
#include "Game/NPC/NPCActor.hpp"
class AllStarList;
class StarListBoard : public NPCActor {
public:
    StarListBoard(const char*);
    virtual ~StarListBoard();
    virtual void init(const JMapInfoIter&);
    bool eventFunc(u32);
    void exeWait();
    void exeOpen();
    AllStarList* mStarList;
};
