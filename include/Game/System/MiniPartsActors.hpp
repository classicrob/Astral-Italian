#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class WorldMapResultWorld;

// Mini actors placed on the world-map nodes (SMG2 only). Layouts are read from the constructors
// (0x804E33B0 ..); only the members used by the world-map holders are declared.

class MiniEarthenPipe : public LiveActor {
public:
    MiniEarthenPipe();

    virtual ~MiniEarthenPipe();
    virtual void control();

    bool isUsable() const;
    bool isUsableStrict() const;

    /* 0x90 */ u8 _90[4];
};

/// Warp point between worlds (vtable __vt__18MiniWorldWarpPoint, 0x9C bytes).
class MiniWorldWarpPoint : public LiveActor {
public:
    MiniWorldWarpPoint(WorldMapResultWorld*, s32 pointIndex, bool isForward, s32 targetWorld);

    bool updateDistance(s32 distance);
    void setEffectDrawOrder(s32 order);

    /* 0x90 */ WorldMapResultWorld* mWorld;
    /* 0x94 */ s32 mPointIndex;
    /* 0x98 */ u8 _98[4];
};

/// Warp point of the star road (vtable __vt__18MiniStarWorldPoint, 0xA0 bytes).
class MiniStarWorldPoint : public LiveActor {
public:
    MiniStarWorldPoint(WorldMapResultWorld*, s32 pointIndex, s32 targetWorld);

    /* 0x90 */ WorldMapResultWorld* mWorld;
    /* 0x94 */ s32 mPointIndex;
    /* 0x98 */ s32 mTargetWorld;
    /* 0x9C */ s32 _9C;
};

/// Star piece mine (vtable __vt__17MiniStarPieceMine, 0x9C bytes).
class MiniStarPieceMine : public LiveActor {
public:
    MiniStarPieceMine(s32 worldId, s32 pointIndex);

    void setEffectDrawOrder(s32 order);

    /* 0x90 */ u8 _90[4];
    /* 0x94 */ s32 mWorldId;
    /* 0x98 */ s32 mPointIndex;
};

/// Relay point marker on a map node (name "中継点", model "MiniRoutePoint", 0x9C bytes).
class MiniRoutePoint : public LiveActor {
public:
    MiniRoutePoint();

    bool isOpening() const;
    bool tryOpen();
    void setPosition(const TVec3f&);
    void setReachable(bool);
    bool isStarting() const;
    void start();
    bool startOpenSound();
    bool startCloseSound();

    /* 0x90 */ s32 mPointIndex;
    /* 0x94 */ u8 _94[4];
    /* 0x98 */ bool mNotReady;
    /* 0x99 */ bool mFlag11;
    /* 0x9A */ u8 _9A[2];
};
