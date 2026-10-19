#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class ActorCameraInfo;
class DemoTitleAnimSub;
class DemoTitleAnim;

// One CSV row of EntryActorList (0x14 bytes). Filled by MR::getCsvDataStr* in DemoTitleAnim::createSubs.
struct DemoTitleAnimParam {
    DemoTitleAnimParam();  // was fn_8013CF30 (constructor has no source yet)

    /* 0x00 */ const char* mArcName;
    /* 0x04 */ const char* mJointName;
    /* 0x08 */ const char* mActionName;
    /* 0x0C */ const char* mLoopActionName;
    /* 0x10 */ const char* mSelectingActionName;
};

// LiveActor-derived sub-actor (0x9C bytes). The ctors live in an unsplit area (fn_8013CA10 / fn_8013CC80).
// Only the four virtuals after LiveActor's table (slots 0x70..0x7C) are used here.
class DemoTitleAnimSub : public LiveActor {
public:
    virtual void startSub();
    virtual void loopSub();
    virtual void selectSub();
    virtual void endSub();
};

// Sub-actor kinds created by DemoTitleAnim::createSubs (constructors have no source yet; 0x9C bytes each).
class DemoTitleAnimPlayer : public DemoTitleAnimSub {
public:
    DemoTitleAnimPlayer(DemoTitleAnim*, DemoTitleAnimParam*);  // was fn_8013CC80

    u8 _90[0xC];
};

class DemoTitleAnimModel : public DemoTitleAnimSub {
public:
    DemoTitleAnimModel(DemoTitleAnim*, DemoTitleAnimParam*);  // was fn_8013CA10

    u8 _90[0xC];
};

class DemoTitleAnim : public LiveActor {
public:
    DemoTitleAnim(const char*);

    virtual ~DemoTitleAnim();
    virtual void init(const JMapInfoIter&);
    virtual void control();

    void start();
    void setLoop();
    void exeStart();
    void exeLoop();
    void exeSelecting();
    void endAll();
    bool isBgmStartStep() const;
    bool isTitleLogoStep() const;
    void createSubs();

    /* 0x90 */ ActorCameraInfo* mCameraInfo;
    /* 0x94 */ DemoTitleAnimSub** mSubs;
    /* 0x98 */ s32 mSubCount;
    /* 0x9C */ const s32* mBgmStartStep;
    /* 0xA0 */ const s32* mTitleLogoStep;
    /* 0xA4 */ s32 mStep;
};
