#pragma once

#include "Game/LiveActor/LiveActor.hpp"

/// Route point of the world map (0x9A bytes, ctor 0x804E53B0). Owns a shadow LiveActor (mShadow).
class MiniRoutePoint : public LiveActor {
public:
    MiniRoutePoint();

    virtual ~MiniRoutePoint();
    virtual void init(const JMapInfoIter&);
    virtual void makeActorAppeared();
    virtual void makeActorDead();
    virtual void control();

    bool isOpening() const;
    bool tryOpen();
    void setPosition(const TVec3f& rPos);
    void setReachable(bool isReachable);
    bool isStarting() const;
    bool start();
    bool startOpenSound();
    bool startCloseSound();

    /* 0x90 */ s32 mPointIndex;   // world-map node this mark stands on
    /* 0x94 */ LiveActor* mShadow;
    /* 0x98 */ bool mNotReady;    // node not ready yet: the mark is drawn small
    /* 0x99 */ bool mIsTicoBuild;  // node's TicoBuild flag: "TicoBuild" brk instead of "Normal"
};
