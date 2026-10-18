#pragma once

#include "Game/LiveActor/LiveActor.hpp"

struct WorldMapModelChildren {
    WorldMapModelChildren() ALWAYS_INLINE : mArr(0), mMaxSize(0) {}
    ~WorldMapModelChildren() NO_INLINE;
    LiveActor** mArr;
    s32 mMaxSize;
};

/// The world-map scenery: a LiveActor owning the child objects named by the stage's ChildObj list (0x98 bytes).
class WorldMapModel : public LiveActor {
public:
    WorldMapModel(const char* pName);

    virtual ~WorldMapModel();
    virtual void init(const JMapInfoIter&);
    virtual void initAfterPlacement();

    void appearAll();  // fn_804E7900: makeActorAppeared on every child, then on itself, then starts all animations
    void killAll();    // fn_804E7990

    /* 0x90 */ WorldMapModelChildren mChildren;
};
