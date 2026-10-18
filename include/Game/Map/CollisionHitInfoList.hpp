#pragma once

#include "Game/Map/HitInfo.hpp"

class CollisionHitInfoList {
public:
    CollisionHitInfoList();

    u32 getNearPolyOnLineSort(const TVec3f&, const TVec3f&, const TVec3f&, const HitSensor*);
    bool getSortedPoly(TVec3f*, Triangle*, u32) const;

    HitInfo mHits[32];
    u32 mCount;
};
