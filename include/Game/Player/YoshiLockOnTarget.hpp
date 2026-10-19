#pragma once
#include "Game/Util/BothDirList.hpp"
#include <JSystem/JGeometry/TVec.hpp>
class LiveActor;
class YoshiTargetMark;
struct YoshiLockOnTarget {
    YoshiLockOnTarget(LiveActor*, MtxPtr, const TVec3f&) NO_INLINE;
    LiveActor* mActor;
    MtxPtr mMtx;
    TVec3f mOffset;
    bool mValid;
};
typedef YoshiLockOnTarget YoshiLockOnTargetInfo;
struct YoshiLockOnTargetDesc {
    YoshiLockOnTargetDesc();
    LiveActor* mActor;
    MtxPtr mMtx;
    TVec3f mOffset;
    u32 mType;
};
class YoshiLockOnTargetControl : public MR::BothDirPtrLink {
public:
    YoshiLockOnTargetControl() : MR::BothDirPtrLink(this) { reset(); }
    void reset() NO_INLINE;
    bool isInvalid() const;
    bool isMarkDead() const;
    void copyTargetInfo(YoshiLockOnTargetDesc*) const;
    void appearMark() NO_INLINE;
    void disappearMark() NO_INLINE;
    void update(const TVec3f&, f32);
    void calcScreenPosition();
    void updateMarkPosition();
    void invalidate();

    YoshiLockOnTargetInfo* mTarget;
    u32 mTimer;
    YoshiTargetMark* mMark;
    u32 mType;
};
f32 getYoshiLockOnDistance(s32);
