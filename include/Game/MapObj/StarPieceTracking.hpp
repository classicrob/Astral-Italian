#pragma once
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/LiveActor/LiveActorGroup.hpp"
#include "JSystem/J3DGraphBase/J3DStruct.hpp"

class StarPieceTrackingPiece : public LiveActor {
public:
    StarPieceTrackingPiece(const char*);
    virtual ~StarPieceTrackingPiece();
    virtual void init(const JMapInfoIter&);
    virtual void attackSensor(HitSensor*, HitSensor*);
    void appearFromActor(const LiveActor*, const TVec3f&, const TVec3f&, f32, f32, f32);
    void appear(const TVec3f&, const TVec3f&, const TVec3f&, f32, f32, f32);
    void appearScattered(const TVec3f&, const TVec3f&, f32, f32);
    void updateTrackingVelocity(bool);
    void exeFly();
    void exeFlyWorldMap();
    void exeTracking();
    void exeTrackingWorldMap();
    void calcLaunchVelocity();
    J3DGXColor mColor;
    TVec3f mDirection;
    TVec3f mUp;
    f32 mSpeed;
    f32 mDistance;
    TVec3f mTrackingVelocity;
    TVec3f mTrackingPosition;
    TVec3f mTrackingStartPosition;
};

class StarPieceTracking : public LiveActorGroup {
public:
    StarPieceTracking(const char* name, s32 count) : LiveActorGroup(name, count) {}
    virtual ~StarPieceTracking() {}
    void appear(const TVec3f&, const TVec3f&, const TVec3f&, const TVec3f&, f32, f32, f32, f32);
};
