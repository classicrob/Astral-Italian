#pragma once
#include "Game/LiveActor/LiveActor.hpp"
class RailMoveObj;
class RailMoveObjGenerator : public LiveActor {
public:
    RailMoveObjGenerator(const char*);
    virtual ~RailMoveObjGenerator();
    virtual void init(const JMapInfoIter&);
    virtual void kill();
    // Optional initialization hooks at vtable slots 0x70/0x74, called before argument reads.
    // Both return immediately; KoopaJrLv2LiftGenerator inherits these implementations.
    virtual void fn_802EB5B0();
    virtual void fn_802EB5C0();
    virtual void initRailMoveObjs(const JMapInfoIter&) = 0;
    virtual void beforeGenerate();
    void exeWait();
    void exeRepeat();
    RailMoveObj* getDeadRailMoveObj();
    s32 mObjectCount;
    s32 mGenerateInterval;
    s32 mInitialDelay;
    s32 _9C;
    RailMoveObj** mObjects;
};
class KoopaJrLv2LiftGenerator : public RailMoveObjGenerator {
public:
    KoopaJrLv2LiftGenerator(const char* name) : RailMoveObjGenerator(name) {}
    virtual ~KoopaJrLv2LiftGenerator();
    virtual void initRailMoveObjs(const JMapInfoIter&);
};
