#pragma once
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/LiveActor/LiveActorGroup.hpp"
class InfernoMario;
class InfernoMarioGroup : public LiveActorGroup {
public:
    InfernoMarioGroup(const char* name,s32 count) : LiveActorGroup(name,count) {}
    virtual ~InfernoMarioGroup();
    InfernoMario* getDeadMember() const;
};
class InfernoMarioGenerator : public LiveActor {
public:
    InfernoMarioGenerator(const char*);
    virtual ~InfernoMarioGenerator();
    virtual void init(const JMapInfoIter&);
    virtual void makeActorAppeared();
    virtual void makeActorDead();
    virtual void control();
    void end();
    void exeWait();
    void exePrepare();
    void exeDemo();
    void exeGenerate();
    void exeEnd();
    InfernoMarioGroup* mGroup;
    s32 mGenerated;
    s32 mCount;
    s32 mSpacing;
    s32 mFrame;
    int mAlreadyDoneFlag;
    bool mAlreadyDone;
};
