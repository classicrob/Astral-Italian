#pragma once
#include "Game/MapObj/MapPartsRotator.hpp"
class MapPartsSeesaw2AxisRollerRotator : public MapPartsRotatorBase {
public:
    MapPartsSeesaw2AxisRollerRotator(LiveActor*, const char*, f32);
    virtual ~MapPartsSeesaw2AxisRollerRotator();
    virtual void init(const JMapInfoIter&);
    virtual bool isWorking() const;
    virtual void start();
    virtual void end();
    virtual const TMtx34f& getRotateMtx() const;
    virtual bool isMoving() const;
    void rotate();
    void calcRotateMtx();
    void exeMove();
    void exeStay();
    f32 mInertia;
    TPos3f mBaseMtx;
    TPos3f mBaseMtxInv;
    TPos3f mRotateMtx;
    TVec3f mUp;
    TVec3f mAngularVelocity;
    TVec3f mPreviousAxis;
    f32 mSpeed;
    const char* mMoveSound;
    f32 mMoveSoundSpeed;
    f32 mMaxSpeed;
};
