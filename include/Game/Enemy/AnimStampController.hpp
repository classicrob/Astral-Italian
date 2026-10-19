#pragma once

#include "Game/System/NerveExecutor.hpp"
#include <JSystem/JGeometry.hpp>

class AnimStampController : public NerveExecutor {
public:
    AnimStampController();

    void init(s32, f32, f32, f32, f32, f32, f32);
    void startAnim(const TVec3f&);
    void exeAnim();
    void exeAnimEnd();
    bool isAnimEnd() const;

    /* 0x8  */ TVec3f mDirection;
    /* 0x14 */ TPos3f mScaleMtx;
    /* 0x44 */ s32 mDuration;
    /* 0x48 */ TVec3f mUnusedVec48;  // unused
    /* 0x54 */ TVec3f mUnusedVec54;  // unused
};
