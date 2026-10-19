#pragma once

#include "Game/Screen/LayoutActor.hpp"

class StaffRollPart;
class StaffRollDemoCamera;

class StaffRoll : public LayoutActor {
public:
    StaffRoll();
    virtual ~StaffRoll();
    virtual void init(const JMapInfoIter&);
    virtual void appear();
    virtual void movement();
    virtual void draw() const;
    virtual void calcAnim();
    virtual void kill();

    bool isEnd() const;
    bool isCurrentPartEnd() const;
    bool isLastPartEnd() const;
    s32 getCurrentPartIndex() const NO_INLINE;
    s32 getElapsedFrame() const;
    bool isDemoCameraStartFinished() const NO_INLINE;
    bool isDemoCameraEndFinished() const NO_INLINE;
    void exePrepareBgm();
    void exePart();
    void exePaneAppear();
    void exePaneWait();
    void exePaneDisappear();
    void exeWait();
    void exeWaitDemo();
    void exeDemoStart();
    void exeDemoEnd();
    void exeEnd();

    const wchar_t* mMessage;
    u32 mMessageLineCount;
    u32 mMessageLine;
    TVec2f mTitleLogoPosition;
    TVec2f mEndLogoPosition;
    TVec2f mAllRightsPosition;
    s32 mLastPartIndex;
    StaffRollPart* mParts[32];
    s32 mPartCount;
    StaffRollPart** mCurrentPart;
    s32 mPartDuration;
    u32 mStartFrame;
    StaffRollDemoCamera* mDemoCamera;
};

namespace MR {
    StaffRoll* getStaffRoll();
}
