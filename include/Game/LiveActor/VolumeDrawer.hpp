#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include "Game/Util/Color.hpp"

class VolumeDrawer : public LiveActor {
public:
    VolumeDrawer(const char* pName, const char* pArcName, f32 (*pMtx)[4], int drawType);

    void setHost(const LiveActor* pHost);
    void setColor(const Color8& rColor);
    void setDrawPasses(s32 passes);
    void setScale(const TVec3f& rScale);
    void setScaleUpRate(s32 frames);
    void setScaleDownRate(s32 frames);
    virtual void control();
    bool isShowVolume() const;
    virtual void calcAndSetBaseMtx();

    /* 0x90 */ f32 (*mBaseMtx)[4];
    /* 0x94 */ LiveActor* mHost;
    /* 0x98 */ Color8 mColor;
    /* 0x9C */ s32 mDrawPasses;
    /* 0xA0 */ TVec3f mScale;
    /* 0xAC */ f32 mScaleRate;
    /* 0xB0 */ f32 mScaleStep;
    /* 0xB4 */ bool mIsEnabled;
};

namespace MR {
    VolumeDrawer* createVolumeDrawer(const char*, const char*, MtxPtr);
};  // namespace MR
