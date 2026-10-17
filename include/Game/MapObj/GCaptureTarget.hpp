#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include "Game/MapObj/BlueStarCupsulePlanet.hpp"

class MapPartsRailMover;
class ActorCameraInfo;

class GCaptureTarget : public LiveActor, public GCaptureTargetable {
public:
    GCaptureTarget(const char*);

    virtual void init(const JMapInfoIter&);
    virtual void initAfterPlacement();
    virtual void appear();
    virtual void makeActorAppeared();
    virtual void makeActorDead();
    virtual void startClipped();
    virtual void endClipped();
    virtual void control();

    void decidedTarget();
    void releasedTarget();
    void emitNerveEffect();
    void getTargetPosition(TVec3f*);
    bool isReleaseForce() const;
    f32 getPointableRange() const;
    f32 releaseDistance() const;

    void exeTryDemoAppear();
    void exeAppear();
    void exeWait();
    void exePointable();
    void exeHitPointer();
    void exeActive();

    /* 0x94 */ MapPartsRailMover* mRailMover;
    /* 0x98 */ f32 mStarAnimSpeed;
    /* 0x9C */ ActorCameraInfo* mCameraInfo;
    /* 0xA0 */ f32 mReleaseDistance;
    /* 0xA4 */ f32 mPointableRange;
    /* 0xA8 */ bool mFarAwayColor;
};
