#pragma once

#include "Game/System/NerveExecutor.hpp"
#include <JSystem/JGeometry.hpp>

class AnimScaleParam {
public:
    AnimScaleParam();

    /* 0x0 */ f32 mSpringStrength;
    /* 0x4 */ f32 mVelocityDamping;
    /* 0x8 */ f32 mMinScaleY;
    /* 0xC */ f32 mMaxScaleY;
    /* 0x10 */ f32 mDpdVibrationTime;
    /* 0x14 */ f32 mDpdVibrationBase;
    /* 0x18 */ f32 mDpdVibrationAmplitude;
    /* 0x1C */ f32 mDpdVibrationFrequency;
    /* 0x20 */ f32 mHitReactionVelocityY;
    /* 0x24 */ f32 mHitReactionSpringStrength;
    /* 0x28 */ f32 mHitReactionDamping;
    /* 0x2C */ u32 mCrushDuration;
    /* 0x30 */ f32 mCrushScaleY;
};

class AnimScaleController : public NerveExecutor {
public:
    AnimScaleController(AnimScaleParam*);

    virtual ~AnimScaleController();

    void update();
    void updateScale(f32, f32);
    void startCrush();
    void startAnim();
    void startAndAddScaleVelocityY(f32);

    void setParamTight();
    void startHitReaction();
    void startScaleVelocityY(f32);
    void startDpdHitVibration();
    bool isHitReaction(s32) const;

    void stop(const TVec3f& rScale);
    void stopAndReset();
    void resetScale();

    bool tryStop();

    void exeAnim();
    inline void exeStop();
    void exeDpdVibration();
    void exeHitReaction();
    void exeCrush();

    /* 0x8 */ AnimScaleParam* mParam;
    /* 0xC */ TVec3f mScale;
    /* 0x18 */ f32 mScaleVelocityY;
};

class LiveActor;
namespace MR {
    void updateBaseScale(LiveActor*, AnimScaleController*);
};
