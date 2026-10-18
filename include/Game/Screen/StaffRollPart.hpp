#pragma once

#include "Game/System/NerveExecutor.hpp"
#include <JSystem/JGeometry/TVec.hpp>
#include <JSystem/JGeometry/TBox.hpp>

class LayoutActor;
class StaffRollLine;

class StaffRollPart : public NerveExecutor {
public:
    StaffRollPart(const wchar_t*, u32*);
    virtual ~StaffRollPart();
    virtual void appear();
    virtual void kill();
    virtual void movement();
    virtual void calcAnim();
    virtual void draw() const;

    void addLinePosition(const TVec2f&);
    void alignLines(s32);
    TVec2f getPosition() const;
    void setLineDuration(s32);
    bool isAllLinesDead() const;
    void calcTextRect(JGeometry::TBox2<f32>*) const;
    void exeWait();
    void exeWaitBeforePart();
    void exePart();
    void exeEnd();
    void setDuration(s32);
    bool isEnd() const;

    s32 mType;
    TVec2f mPosition;
    StaffRollLine* mLines[32];
    s32 mLineCount;
    const wchar_t* mMessage;
    f32 mScrollSpeed;
    s32 mWaitFrames;
    bool mHasEndMarker;
    s32 mElapsedFrames;
    s32 mDuration;
    s32 mScrollFrames;
    bool mIsAlive;
};
