#pragma once

#include <JSystem/JGeometry/TVec.hpp>
#include <revolution.h>

class RailGraphNode {
public:
    RailGraphNode();

    /* 0x00 */ TVec3f mPosition;
    /* 0x0C */ s32 mFirstEdge;
    /* 0x10 */ u32 mEdgeCount;
};
