#pragma once
#include "Game/LiveActor/LiveActor.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>
class VolumeDrawer;
class MagmaSeaWave : public LiveActor {
public:
    MagmaSeaWave(const char*);
    virtual ~MagmaSeaWave();
    virtual void init(const JMapInfoIter&);
    virtual void control();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual void kill();
    void exeMove();
    VolumeDrawer* mDrawer;
    TPos3f mVolumeMtx;
    TVec3f mSensorPosition;
    TVec3f mRailClipPosition;
    f32 mSpeed;
    s32 mSoundFrame;
};
