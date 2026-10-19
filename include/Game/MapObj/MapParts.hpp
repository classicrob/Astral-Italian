#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class MapParts : public LiveActor {
public:
    MapParts(const char*);

    virtual ~MapParts() {};

    virtual void init(const JMapInfoIter&);
    virtual void appear();
    virtual s32 getSensorNumMax() const;
    virtual void connectToScene();

    static s32 getMoveStartSignalTime();
    static s32 getMoveStartSignalTimeForRail();
    void initModelAndCollision(const JMapInfoIter&);

    /* 0x90 */ TVec3f _8C;
    /* 0x9C */ TVec3f _9C;
};
