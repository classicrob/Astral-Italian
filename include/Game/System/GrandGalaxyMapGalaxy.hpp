#pragma once

#include "Game/LiveActor/LiveActor.hpp"

/// Miniature galaxy model shown on the grand galaxy map (0xA4 bytes).
class GrandGalaxyMapGalaxy : public LiveActor {
public:
    GrandGalaxyMapGalaxy(const char* pName, const char* pModelName);
    virtual ~GrandGalaxyMapGalaxy();
    virtual void control();

    void emitEffect(bool isNew);
    void stopRotate(const TVec3f& rRotate);
    void setFitRadius(f32 radius);

    /* 0x90 */ TVec2f mLayoutOffset;
    /* 0x98 */ f32 mBaseScale;
    /* 0x9C */ f32 mBoundingRadius;
    /* 0xA0 */ bool mIsRotating;
};
