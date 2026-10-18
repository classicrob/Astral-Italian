#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class JMapInfoIter;

// Object that MiniMeister holds at +0xA4; its +0x50 is the LiveActor the meister talks for.
class MiniMeisterOwner {
public:
    /* 0x00 */ u8 mPad[0x50];
    /* 0x50 */ LiveActor* mTalkActor;
};

// Talking mini-meister on the world map (SMG2 only).
class MiniMeister : public LiveActor {
public:
    MiniMeister();
    MiniMeister(MiniMeisterOwner*);
    virtual ~MiniMeister();
    virtual void init(const JMapInfoIter&);
    virtual void appear();
    virtual void makeActorAppeared();
    virtual void makeActorDead();
    virtual void control();
    virtual void calcAndSetBaseMtx();

    void appearAtScreenPos(const TVec2f&);
    void setNerveTalkEndWait();
    void setNerveTalk();
    s32 isTalkStarting() const;
    s32 isTalkEnding() const;
    void calcPositionFromCamera(const TVec3f&);
    void updateTalkScreenPos(f32);
    bool startTalkAction(u32);

    void exeTalkStart();
    void exeTalkStartWait();
    void exeWait();
    void exeTalk();
    void exeTalkEndWait();
    void exeTalkEnd();
    void exeDisappear();

    /* 0x90 */ f32 mUnk90;
    /* 0x94 */ f32 mUnk94;
    /* 0x98 */ f32 mUnk98;
    /* 0x9C */ TVec2f mScreenPos;
    /* 0xA4 */ MiniMeisterOwner* mOwner;
};
