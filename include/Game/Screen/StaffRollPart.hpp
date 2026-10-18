#pragma once

#include "Game/System/NerveExecutor.hpp"
#include <JSystem/JGeometry/TVec.hpp>

class LayoutActor;

class StaffRollPart : public NerveExecutor {
public:
    StaffRollPart(const wchar_t*, u32*);
    virtual ~StaffRollPart();
    virtual void appear();
    virtual void kill();
    virtual void movement();
    virtual void calcAnim();
    virtual void draw() const;

    void setDuration(s32);
    bool isEnd() const;

    s32 mType;
    TVec2f mPosition;
    LayoutActor* mLines[32];
    s32 mLineCount;
    const wchar_t* mMessage;
    f32 mScrollSpeed;
    s32 mWaitFrames;
    bool mHasEndMarker;
    s32 mElapsedFrames;
    s32 mDuration;
    s32 mScrollFrames;
    bool mIsDead;
};
