#pragma once

#include "Game/MapObj/MapPartsRotator.hpp"

class MapPartsSeesaw2AxisRotator : public MapPartsRotatorBase {
public:
    MapPartsSeesaw2AxisRotator(LiveActor*, const char*, f32);

    virtual ~MapPartsSeesaw2AxisRotator();
    virtual void init(const JMapInfoIter&) override;
    virtual bool isWorking() const override;
    virtual void start() override;
    virtual void end() override;
    virtual bool receiveMsg(u32) override;
    virtual const TMtx34f& getRotateMtx() const override {
        return mRotateMtx;
    }

    virtual bool isMoving() const override {
        return isWorking();
    }

    void exeWait() {
    }

    void exeMove();
    void exeStay();
    void exeHipDrop();
    void rotate();
    void restoreMove();
    f32 getInertiaConst() const;

    /* 0x28 */ f32 mRotateAngle;
    /* 0x2C */ f32 mInertiaConst;
    /* 0x30 */ f32 mRestoreForce;
    /* 0x34 */ TPos3f mBaseMtx;
    /* 0x64 */ TPos3f mBaseMtxInv;
    /* 0x94 */ TPos3f mRotateMtx;
    /* 0xC4 */ TVec3f mUp;
    /* 0xD0 */ bool mHipDrop;
    /* 0xD4 */ f32 mRotateSpeed;
    /* 0xD8 */ const char* mMoveSound;
    /* 0xDC */ f32 mMoveSoundSpeed;
    /* 0xE0 */ f32 mSoundIntensity;
};
