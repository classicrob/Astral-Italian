#pragma once
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/LiveActor/LiveActorGroup.hpp"

// The tracking projectile is allocated with 0xD8 bytes in GliderBalloon::init.
class StarPieceTrackingPiece : public LiveActor {
public:
    StarPieceTrackingPiece(const char*);
    void appearFromActor(const LiveActor*, const TVec3f&, const TVec3f&, f32, f32, f32);
    s32 _90;
    TVec3f mDirection; // 0x94
    TVec3f mUp; // 0xA0
    f32 mSpeed; // 0xAC
    f32 mDistance; // 0xB0
    TVec3f _B4;
    TVec3f _C0;
    TVec3f _CC;
};
class StarPieceTracking : public LiveActorGroup {
public:
    StarPieceTracking(const char* name, s32 count) : LiveActorGroup(name, count) {}
    virtual ~StarPieceTracking() {}
    void appear(const TVec3f&, const TVec3f&, const TVec3f&, const TVec3f&, f32, f32, f32, f32);
};
