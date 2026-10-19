#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include <JSystem/JGeometry/TBox.hpp>
#include <JSystem/JGeometry/TMatrix.hpp>

class ActorCameraInfo;
class ModelObj;

class BlackHole : public LiveActor {
public:
    BlackHole(const char*);

    virtual ~BlackHole();
    virtual void init(const JMapInfoIter&);
    virtual void makeActorAppeared();
    virtual void makeActorDead();
    virtual void kill();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor*, HitSensor*);

    bool tryStartDemoCamera();
    void initMapToolInfo(const JMapInfoIter&);
    void initModel();
    void initCubeBox();
    bool isInCubeBox(const TVec3f&) const;
    void updateModelScale(f32, f32);
    void exeWait();
    void exeDemo();
    void exeDisappear();

    /* 0x090 */ ModelObj* mBlackHoleModel;
    /* 0x094 */ TVec3f _94;  // never written after the ctor zero-init
    /* 0x0A0 */ f32 mModelScale;
    /* 0x0A4 */ f32 mSensorRadius;
    /* 0x0A8 */ TBox3f* mCubeBounds;
    /* 0x0AC */ TPos3f mCubeMtx;
    /* 0x0DC */ TPos3f mEffectMtx;
    /* 0x10C */ ActorCameraInfo* mCameraInfo;
};
