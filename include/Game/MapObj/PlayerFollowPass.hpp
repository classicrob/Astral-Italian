#pragma once
#include "Game/NameObj/NameObj.hpp"
#include <JSystem/JGeometry/TVec.hpp>
class PlayerFollowPass : public NameObj {
public:
    PlayerFollowPass();
    virtual ~PlayerFollowPass();
    virtual void initAfterPlacement();
    virtual void movement();
    void clear();
    void reset();
    void calcPlayerPosition(TVec3f*);
    void update();
    void calcPosition(TVec3f*, f32);
    TVec3f* mPositions;
    f32* mDistances;
    s32 mCapacity;
    s32 mCount;
    s32 mCurrent;
    s32 mDelay;
    bool mPaused;
};
namespace MR {
    void resetPlayerFollowPass();
    void clearPlayerFollowPass();
    void pausePlayerFollowPass();
    void resumePlayerFollowPass();
}
