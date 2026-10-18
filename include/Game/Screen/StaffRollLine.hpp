#pragma once

#include "Game/Screen/LayoutActor.hpp"

struct StaffRollLineRegionInfo {
    f32 mScale;
    f32 mTitleLinePitch;
    f32 mNameLinePitch;
};

class StaffRollLine : public LayoutActor {
public:
    StaffRollLine(s32, s32);
    virtual ~StaffRollLine();
    virtual void init(const JMapInfoIter&);
    virtual void appear();
    virtual void kill();

    void setDuration(s32);
    const char* getPaneName() const;
    void exeAppear();
    void exeWait();
    void exeDisappear();
    static const StaffRollLineRegionInfo* getRegionInfo();

    s32 mType;
    TVec2f mPosition;
    s32 mLineIndex;
    s32 mWaitFrames;
};
