#pragma once
#include <JSystem/JGeometry/TVec.hpp>
class LiveActor;
class YoshiTargetMark;
struct YoshiLockOnTarget {
    LiveActor* mActor;
    MtxPtr mMtx;
    TVec3f mOffset;
    bool mValid;
};
typedef YoshiLockOnTarget YoshiLockOnTargetInfo;
struct YoshiLockOnTargetDesc {
    LiveActor* mActor;
    MtxPtr mMtx;
    TVec3f mOffset;
    u32 mType;
};
class YoshiLockOnTargetControl {
public:
    bool isInvalid() const;
    bool isMarkDead() const;
    void copyTargetInfo(YoshiLockOnTargetDesc*) const;
    void appearMark();
    void disappearMark();
    void update(const TVec3f&, f32);
    void calcScreenPosition();
    void updateMarkPosition();
    void invalidate();
    u8 _0[0x10];
    YoshiLockOnTargetInfo* mTarget;
    u32 mTimer;
    YoshiTargetMark* mMark;
    u32 mType;
};
f32 getYoshiLockOnDistance(s32);
