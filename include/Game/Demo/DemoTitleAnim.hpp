#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class ActorCameraInfo;
class DemoTitleAnimModel;
class DemoTitleAnim;

// One CSV row of EntryActorList (0x14 bytes). Filled by MR::getCsvDataStr* in DemoTitleAnim::createSubs.
struct DemoTitleAnimParam {
    DemoTitleAnimParam();

    /* 0x00 */ const char* mArcName;
    /* 0x04 */ const char* mJointName;
    /* 0x08 */ const char* mActionName;
    /* 0x0C */ const char* mLoopActionName;
    /* 0x10 */ const char* mSelectingActionName;
};

// Sub-actor that plays an arc model (0x9C bytes). Sourced in DemoTitleAnimSub.cpp.
class DemoTitleAnimModel : public LiveActor {
public:
    DemoTitleAnimModel(DemoTitleAnim*, DemoTitleAnimParam*);

    virtual void init(const JMapInfoIter&);
    virtual void calcAndSetBaseMtx();
    virtual void startSub();
    virtual void loopSub();
    virtual void selectSub();
    virtual void endSub();

    void exeAction();

    /* 0x90 */ DemoTitleAnim* mParent;
    /* 0x94 */ DemoTitleAnimParam* mParam;
    /* 0x98 */ MtxPtr mBaseMtx;
};

// Sub-actor that drives the player instead of a model.
class DemoTitleAnimPlayer : public DemoTitleAnimModel {
public:
    DemoTitleAnimPlayer(DemoTitleAnim*, DemoTitleAnimParam*);

    virtual void init(const JMapInfoIter&);
    virtual void control();
    virtual void startSub();
    virtual void loopSub();
    virtual void selectSub();
    virtual void endSub();
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
    /* 0x94 */ DemoTitleAnimModel** mSubs;
    /* 0x98 */ s32 mSubCount;
    /* 0x9C */ const s32* mBgmStartStep;
    /* 0xA0 */ const s32* mTitleLogoStep;
    /* 0xA4 */ s32 mStep;
};
