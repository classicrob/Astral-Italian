#pragma once

#include "Game/NameObj/NameObj.hpp"
#include "Game/Player/YoshiLockOnTarget.hpp"

class LiveActor;
class Yoshi;
class YoshiTargetMark;
class YoshiLockOnTarget;

class YoshiLockOnManager : public NameObj {
public:
    YoshiLockOnManager(Yoshi*);
    virtual ~YoshiLockOnManager();
    virtual void init(const JMapInfoIter&);
    virtual void movement();
    virtual void draw() const;
    void fn_80419EB0(Yoshi*);
    bool fn_8041A130(YoshiLockOnTarget*,s32,s32,u32,f32);
    bool fn_8041A290(YoshiLockOnTarget*,s32,f32);
    bool fn_8041A3E0(YoshiLockOnTarget*,s32,s32);
    YoshiLockOnTargetControl* fn_8041A4A0(YoshiLockOnTarget*);
    YoshiTargetMark* fn_8041A510();
    void fn_8041A590();
    void fn_8041A660();
    bool fn_8041A720(YoshiLockOnTarget*);
    bool fn_8041A770(YoshiLockOnTarget*);
    u32 fn_8041A010() const;
    u32 fn_8041A020(YoshiLockOnTargetDesc*, u32);
    bool fn_80419F90(YoshiLockOnTarget*);
    bool fn_80419FB0(YoshiLockOnTarget*);
    bool fn_80419FD0(YoshiLockOnTarget*, bool);
    bool fn_80419FF0(YoshiLockOnTarget*);
    bool fn_8041A000(YoshiLockOnTarget*);
    void fn_8041A0B0();
    void fn_8041E120(LiveActor*);
    void fn_8041E140();
    u32 _14;
    LiveActor* mContextActor;
    bool mContextActive;
    bool _1D;
    bool _1E;
    MR::BothDirList<YoshiLockOnTargetControl> mFree;
    MR::BothDirList<YoshiLockOnTargetControl> mFading;
    MR::BothDirList<YoshiLockOnTargetControl> mActive;
    YoshiTargetMark** mMarks;
    s32 mMarkCount;
    bool mSoundLatch;
    u8 _4D[3];
};
