#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include "Game/Util/Array.hpp"

/// World-map scenery with owned pointer storage for the stage's ChildObj actors (0x98 bytes).
class WorldMapModel : public LiveActor {
public:
    WorldMapModel(const char* pName);

    virtual ~WorldMapModel();
    virtual void init(const JMapInfoIter&);
    virtual void initAfterPlacement();

    void appearAll();  // fn_804E7900: makeActorAppeared on every child, then on itself, then starts all animations
    void killAll();    // fn_804E7990

    /* 0x90 */ MR::AssignableArray< LiveActor* > mChildren;
};
