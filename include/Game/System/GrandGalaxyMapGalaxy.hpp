#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class GrandGalaxyMapGalaxy : public LiveActor {
public:
    GrandGalaxyMapGalaxy(const char*, const char*);
    virtual ~GrandGalaxyMapGalaxy();
    virtual void control();

    /* 0x90 */ TVec2f mLayoutOffset;
    /* 0x98 */ f32 mBaseScale;
    /* 0x9C */ f32 mBoundingRadius;
    /* 0xA0 */ bool mIsRotating;
};
