#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include "Game/Util/Color.hpp"

class JUTTexture;

class WaterCameraFilter : public LiveActor {
public:
    WaterCameraFilter();

    virtual void init(const JMapInfoIter& rIter);
    virtual void draw() const;
    virtual void control();

    void exeAirToWater();
    void exeWaterToAir();
    void exeWater();
    void exeAir();
    void loadMaterial() const;

    inline void initScreenTex();

    /* 0x90 */ f32 mWaterAlpha;
    /* 0x94 */ f32 mWavePhase;
    /* 0x98 */ Color8 mWaterColor;
    /* 0x9C */ Color8 mHighColor;
    /* 0xA0 */ Color8 mLowColor;
    /* 0xA4 */ f32 mDepthLow;
    /* 0xA8 */ f32 mIndirectScale;
    /* 0xAC */ JUTTexture* mScreenTex;
    /* 0xB0 */ JUTTexture* mFilterTex;
};
