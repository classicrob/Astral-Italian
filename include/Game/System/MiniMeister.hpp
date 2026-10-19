#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class JMapInfoIter;

class WorldMapCameraHolder;

// Talking mini-meister on the world map (SMG2 only).
class MiniMeister : public LiveActor {
public:
    MiniMeister();
    MiniMeister(WorldMapCameraHolder*);
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

    /* 0x90 */ TVec3f mFront;
    /* 0x9C */ TVec2f mScreenPos;
    /* 0xA4 */ WorldMapCameraHolder* mOwner;
};
