#pragma once
#include "Game/LiveActor/LiveActor.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>
class J3DJoint;
class VolumeDrawer;
class YoshiArrow : public LiveActor {
public:
    YoshiArrow(const char*);
    virtual ~YoshiArrow();
    virtual void init(const JMapInfoIter&);
    virtual void appear();
    virtual void kill();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    void fn_80414040();
    void fn_80413DC0(const TVec3f&, const TVec3f&, const TVec3f&);
    void fn_80413FB0();
    TPos3f mArrowMtx;
    f32 mTargetLength;
    f32 mLength;
    J3DJoint* mArrowJoint;
    VolumeDrawer* mVolume;
    TPos3f mVolumeMtx;
    TVec3f mUp;
};
