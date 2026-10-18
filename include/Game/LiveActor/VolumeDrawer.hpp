#pragma once

#include "Game/NameObj/NameObj.hpp"
#include <JSystem/JGeometry/TVec.hpp>
#include <revolution/types.h>

class Color8;

// Minimal declaration (SMG2 only, not ported yet): enough for GliBird.
class VolumeDrawer : public NameObj {
public:
    void setColor(const Color8&);
    void setDrawPasses(s32);
    void setScale(const TVec3f&);
    void setScaleUpRate(s32);
    void setScaleDownRate(s32);
};

namespace MR {
    VolumeDrawer* createVolumeDrawer(const char*, const char*, MtxPtr);
};
