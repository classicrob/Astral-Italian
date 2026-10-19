#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>
#include <JSystem/JGeometry/TVec.hpp>
#include "Game/Util/Color.hpp"

#include "Game/LiveActor/VolumeDrawer.hpp"

class Unbaba : public LiveActor {
public:
    bool updateUnbabaRise();
    bool updateUnbabaSink();
    void updateUnbabaLaunchMatrix();
    Unbaba(const char*);
    virtual ~Unbaba();

    virtual void init(const JMapInfoIter&);
    virtual void makeActorAppeared();
    virtual void makeActorDead();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);

    void exeWait();
    void exeAppear();
    void exeAppearStep();
    void exeAttackSign();
    void exeAttack();
    void exeLaunch();
    void exeRestart();

    /* 0x90 */ VolumeDrawer* mVolumeDrawer;
    /* 0x94 */ TPos3f mVolumeMatrix;
    /* 0xC4 */ TPos3f mLaunchMtx;
    /* 0xF4 */ TVec3f mHomePosition;
    /* 0x100 */ TVec3f mHomeRotation;
    /* 0x10C */ TVec3f mUpVec;
    /* 0x118 */ f32 mHeight;
    /* 0x11C */ f32 mDistance;
    /* 0x120 */ f32 mUnusedRiseTime;
    /* 0x124 */ f32 mRiseProgress;
    /* 0x128 */ s32 mStepAttackSign;
    /* 0x12C */ s32 mStepAttack;
    /* 0x130 */ bool mIsScaleUp;
    /* 0x131 */ bool mIsAttackBlocked;
};

// unnamed by address (calcUnbabaHeightCurve): pure easing helper, no object argument.
f32 calcUnbabaHeightCurve(f32, f32, f32, f32, f32, f32, f32);

f32 calcUnbabaHermiteSegment(f32, f32, f32, f32, f32, f32, f32);
