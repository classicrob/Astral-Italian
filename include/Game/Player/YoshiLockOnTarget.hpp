#pragma once

#include "Game/Util/BothDirList.hpp"
#include <JSystem/JGeometry/TVec.hpp>

class LiveActor;
class YoshiTargetMark;

namespace YoshiLockOnTargetType {
    enum Value {
        Eat = 0,
        Lick = 1,
        Capture = 2,
        CaptureAlternate = 3,
        Pull = 4,
        Projectile = 5
    };
}

struct YoshiLockOnTarget {
    YoshiLockOnTarget(LiveActor* actor, MtxPtr matrix, const TVec3f& localOffset) NO_INLINE;

    /* 0x00 */ LiveActor* mActor;
    /* 0x04 */ MtxPtr mMtx;
    /* 0x08 */ TVec3f mOffset;  // Transformed by the target's matrix axes when the tongue resolves its position.
    /* 0x14 */ bool mEligibleThisFrame;  // Renewed by interaction checks; cleared after the active-target update.
};

typedef YoshiLockOnTarget YoshiLockOnTargetInfo;

struct YoshiLockOnTargetDesc {
    YoshiLockOnTargetDesc();

    /* 0x00 */ LiveActor* mActor;
    /* 0x04 */ MtxPtr mMtx;
    /* 0x08 */ TVec3f mOffset;
    /* 0x14 */ u32 mType;  // YoshiLockOnTargetType values; retains the original 32-bit storage.
};

class YoshiLockOnTargetControl : public MR::BothDirPtrLink {
public:
    YoshiLockOnTargetControl() : MR::BothDirPtrLink(this) { reset(); }
    void reset() NO_INLINE;
    bool isInvalid() const;
    bool isMarkDead() const;
    void copyTargetInfo(YoshiLockOnTargetDesc* output) const;
    void appearMark() NO_INLINE;
    void disappearMark() NO_INLINE;
    void update(const TVec3f& contextPosition, f32 maxDistance);
    void calcScreenPosition();
    void updateMarkPosition();
    void invalidate();

    /* 0x10 */ YoshiLockOnTargetInfo* mTarget;
    /* 0x14 */ u32 mFramesUntilExpiry;
    /* 0x18 */ YoshiTargetMark* mMark;
    /* 0x1C */ u32 mType;  // YoshiLockOnTargetType values.
};

f32 getYoshiLockOnDistance(s32 targetType);
