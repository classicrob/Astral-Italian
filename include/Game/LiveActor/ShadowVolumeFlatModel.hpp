#pragma once

#include "Game/LiveActor/ShadowVolumeModel.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>

class J3DModelData;

class ShadowVolumeFlatModel : public ShadowVolumeModel {
public:
    ShadowVolumeFlatModel(const char*);
    ShadowVolumeFlatModel();

    inline virtual ~ShadowVolumeFlatModel();
    virtual void draw() const;
    virtual void loadModelDrawMtx() const;

    void initModel(const char*);
    void initJointDrawMtxResister();

    /* 0x28 */ s32 mRootDrawMtxIndex;
    /* 0x2C */ s32 mDropDrawMtxIndex;
};
