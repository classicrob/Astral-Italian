#pragma once

#include "Game/LiveActor/ModelObj.hpp"

#include "Game/LiveActor/LiveActor.hpp"
#include "Game/LiveActor/Nerve.hpp"

class WorldMapResultWorld;
class LayoutActor;

// Mini actors placed on the world-map nodes (SMG2 only). Layouts are read from the constructors
// (0x804E33B0 ..); only the members used by the world-map holders are declared.

class MiniEarthenPipe : public ModelObj {
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

    virtual ~MiniWorldWarpPoint();
    virtual void init(const JMapInfoIter&);
    void makeActorAppeared();
    void makeActorDead();
    bool updateDistance(s32 distance);
    void setEffectDrawOrder(s32 order);

    /* 0x90 */ WorldMapResultWorld* mWorld;
    /* 0x94 */ s32 mPointIndex;
    /* 0x98 */ bool mIsForward;
    /* 0x99 */ u8 _99[3];
};

/// Warp point of the star road (vtable __vt__18MiniStarWorldPoint, 0xA0 bytes).
class MiniStarWorldPoint : public LiveActor {
public:
    MiniStarWorldPoint(WorldMapResultWorld*, s32 pointIndex, s32 targetWorld);

    virtual ~MiniStarWorldPoint();
    virtual void init(const JMapInfoIter&);
    virtual void makeActorAppeared();
    void control();

    /* 0x90 */ WorldMapResultWorld* mWorld;
    /* 0x94 */ s32 mPointIndex;
    /* 0x98 */ s32 mTargetWorld;
    /* 0x9C */ LayoutActor* mLayout;
};

/// Green comet on the star-road map (vtable __vt__14MiniGreenComet). Name "グリーン".
class MiniGreenComet : public LiveActor {
public:
    MiniGreenComet();

    virtual ~MiniGreenComet();
    virtual void init(const JMapInfoIter&);
    virtual void appear();
    virtual void control();
};

/// Falling comet shower around the green comet (vtable __vt__20MiniGreenCometShower). Name "降り注ぐ".
class MiniGreenCometShower : public LiveActor {
public:
    MiniGreenCometShower();

    virtual ~MiniGreenCometShower();
    virtual void init(const JMapInfoIter&);
    virtual void appear();
};

/// Star piece mine (vtable __vt__17MiniStarPieceMine, 0x9C bytes).
class MiniStarPieceMine : public LiveActor {
public:
    MiniStarPieceMine(s32 worldId, s32 pointIndex);

    virtual ~MiniStarPieceMine();
    virtual void init(const JMapInfoIter&);
    virtual void makeActorAppeared();
    virtual void attackSensor(HitSensor*, HitSensor*);
    void setEffectDrawOrder(s32 order);

    /* 0x90 */ NameObj** mPieces;  // 8 child objects (0xD8 each), built in init
    /* 0x94 */ s32 mWorldId;
    /* 0x98 */ s32 mPointIndex;
};

namespace NrvMiniStarPieceMine {
    class MiniStarPieceMineNrvWait : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;

        static MiniStarPieceMineNrvWait sInstance;
    };

    class MiniStarPieceMineNrvBreak : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;

        static MiniStarPieceMineNrvBreak sInstance;
    };
};  // namespace NrvMiniStarPieceMine

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
