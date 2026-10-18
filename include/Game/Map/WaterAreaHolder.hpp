#pragma once

#include "Game/Map/WaterInfo.hpp"
#include "Game/NameObj/NameObj.hpp"
#include <JSystem/JGeometry/TVec.hpp>

class OceanSphere;
class WaterCameraFilter;
class WaterStream;

class WaterAreaHolder : public NameObj {
public:
    WaterAreaHolder();

    virtual void movement();

    void entryOceanSphere(OceanSphere*);
    void entryWaterStream(WaterStream*);

    /* 0x14 */ s32 mNumOceanSpheres;
    /* 0x18 */ OceanSphere** mOceanSpheres;
    /* 0x1C */ s32 mNumWaterStreams;
    /* 0x20 */ WaterStream** mWaterStreams;
    /* 0x24 */ bool mCamInWater;
    /* 0x28 */ WaterInfo mWaterInfo;
    /* 0x7C */ WaterCameraFilter* mCameraFilter;
    /* 0x80 */ bool mUseBloom;
};

namespace WaterAreaFunction {
    void entryOceanSphere(OceanSphere*);
    void entryWaterStream(WaterStream*);
    void createWaterAreaHolder();
    bool tryInOceanArea(const TVec3f&, WaterInfo*);
    bool isCameraInWaterForCameraUtil();
    f32 getCameraWaterDepth();
    WaterInfo* getCameraWaterInfo();
    void pauseOffWaterArea();
};  // namespace WaterAreaFunction
