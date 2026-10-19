#pragma once

#include "Game/LiveActor/ShadowVolumeDrawer.hpp"

class LiveActor;

class ShadowVolumeQuakeWave : public ShadowVolumeDrawer {
public:
    ShadowVolumeQuakeWave(LiveActor*);

    virtual ~ShadowVolumeQuakeWave();
    virtual void loadModelDrawMtx() const override;
    virtual void drawShape() const override;
    virtual bool isDraw() const override;

    /* 0x24 */ LiveActor* mActor;
    /* 0x28 */ MtxPtr mMtx;
    /* 0x2C */ f32 _2C;
    /* 0x30 */ f32 _30;
    /* 0x34 */ f32 _34;
};
