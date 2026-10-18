#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include "Game/Map/WaterInfo.hpp"

class AudSoundObject;

// A rail-driven current (SMG2 only). Water areas are tested against a clipping box and a rail.
class WaterStream : public LiveActor {
public:
    WaterStream(const char*);

    virtual ~WaterStream();
    virtual void init(const JMapInfoIter&);
    virtual void initAfterPlacement();
    virtual void movement();

    bool isInWater(const TVec3f&) const;
    bool calcWaterInfo(const TVec3f&, const TVec3f&, WaterInfo*) const;
    f32 calcNearestPos(const TVec3f&, TVec3f*, TVec3f*, TVec3f*) const;
    f32 calcWaveHeight(const TVec3f&, f32, TVec3f*) const;
    void calcStreamVec(const TVec3f&, const TVec3f&, f32, const TVec3f&, f32, TVec3f*) const;
    f32 calcCurrentWidthRate(f32) const;
    f32 calcCurrentFlowSpeedRate(f32) const;
    void calcClippingBox();

    /* 0x90 */ bool mIsGravityBased;
    /* 0x91 */ bool mIsReversed;
    /* 0x94 */ f32 mWidth;
    /* 0x98 */ f32 mUnk98;
    /* 0x9C */ f32 mUnk9C;
    /* 0xA0 */ f32 mFlowSpeed;
    /* 0xA4 */ f32 mUnkA4;
    /* 0xA8 */ TVec3f mClipMin;
    /* 0xB4 */ TVec3f mClipMax;
    /* 0xC0 */ TVec3f mBoxMin;
    /* 0xCC */ TVec3f mBoxMax;
    /* 0xD8 */ u8 mPadD8[0xC];
    /* 0xE4 */ TVec3f mSoundPos1;
    /* 0xF0 */ AudSoundObject* mSound1;
    /* 0xF4 */ TVec3f mSoundPos2;
    /* 0x100 */ AudSoundObject* mSound2;
};
