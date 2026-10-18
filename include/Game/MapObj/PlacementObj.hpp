#pragma once
#include "Game/LiveActor/LiveActor.hpp"
class PlacementObj : public LiveActor {
public:
    PlacementObj(const char*);
    virtual ~PlacementObj();
    virtual void calcPosition(s32,s32,TVec3f*) = 0;
    s32 getPlacementNum() const;
    void calcPlacementPosition(s32,TVec3f*);
    s32 mPlacementNum;
};
