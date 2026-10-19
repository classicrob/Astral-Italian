#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/NerveUtil.hpp"

namespace NrvAnimScaleController {
    NEW_NERVE(AnimScaleControllerNrvAnim, AnimScaleController, Anim);
    NEW_NERVE(AnimScaleControllerNrvStop, AnimScaleController, Stop);
    NEW_NERVE(AnimScaleControllerNrvDpdVibration, AnimScaleController, DpdVibration);
    NEW_NERVE(AnimScaleControllerNrvHitReaction, AnimScaleController, HitReaction);
    NEW_NERVE(AnimScaleControllerNrvCrush, AnimScaleController, Crush);
};  // namespace NrvAnimScaleController

namespace {

    class AnimScaleDefaultParam : public AnimScaleParam {
    public:
        AnimScaleDefaultParam();

        /* 0x34 */ AnimScaleParam tightParam;
    };

    AnimScaleDefaultParam::AnimScaleDefaultParam() {
        tightParam.mSpringStrength = 0.5f;
        tightParam.mVelocityDamping = 0.85f;
        tightParam.mDpdVibrationTime = 8.0f;
        tightParam.mDpdVibrationBase = 0.8f;
        tightParam.mDpdVibrationAmplitude = 0.08f;
        tightParam.mDpdVibrationFrequency = 2.2f;
    };

    AnimScaleDefaultParam sStaticAnimParam;
};  // namespace

AnimScaleParam::AnimScaleParam() {
    mSpringStrength = 0.2f;
    mVelocityDamping = 0.91f;
    mMinScaleY = 0.2f;
    mMaxScaleY = 1.8f;
    mDpdVibrationTime = 12.0f;
    mDpdVibrationBase = 0.55f;
    mDpdVibrationAmplitude = 0.12f;
    mDpdVibrationFrequency = 2.0f;
    mHitReactionVelocityY = 0.06f;
    mHitReactionSpringStrength = 0.12f;
    mHitReactionDamping = 0.91f;
    mCrushDuration = 0x14;
    mCrushScaleY = 0.25f;
}

AnimScaleController::AnimScaleController(AnimScaleParam* pScaleParam) : NerveExecutor("スケールアニメコントロール\00") {
    mParam = pScaleParam;
    mScale.set(1.0f);
    mScaleVelocityY = 0.0f;

    if (pScaleParam == nullptr) {
        mParam = &::sStaticAnimParam;
    }
    initNerve(GET_NERVE(AnimScaleController, AnimScaleControllerNrvStop));
}

void AnimScaleController::setParamTight() {
    mParam = &::sStaticAnimParam.tightParam;
}

void AnimScaleController::startAnim() {
    setNerve(GET_NERVE(AnimScaleController, AnimScaleControllerNrvAnim));
}

void AnimScaleController::startDpdHitVibration() {
    setNerve(GET_NERVE(AnimScaleController, AnimScaleControllerNrvDpdVibration));
}

void AnimScaleController::startHitReaction() {
    setNerve(GET_NERVE(AnimScaleController, AnimScaleControllerNrvHitReaction));
}

void AnimScaleController::startScaleVelocityY(f32 f1) {
    mScaleVelocityY = f1;
    setNerve(GET_NERVE(AnimScaleController, AnimScaleControllerNrvAnim));
}

void AnimScaleController::startAndAddScaleVelocityY(f32 f1) {
    mScaleVelocityY += f1;
    setNerve(GET_NERVE(AnimScaleController, AnimScaleControllerNrvAnim));
}

void AnimScaleController::startCrush() {
    setNerve(GET_NERVE(AnimScaleController, AnimScaleControllerNrvCrush));
}

void AnimScaleController::stopAndReset() {
    resetScale();
    // resetScale already sets mScaleVelocityY to 0, so this is completely unnecessary
    mScaleVelocityY = 0.0f;
    setNerve(GET_NERVE(AnimScaleController, AnimScaleControllerNrvStop));
}

void AnimScaleController::stop(const TVec3f& rScale) {
    mScaleVelocityY = 0.0f;
    setNerve(GET_NERVE(AnimScaleController, AnimScaleControllerNrvStop));
    mScale.x = rScale.x;
    mScale.y = rScale.y;
    mScale.z = rScale.z;
}

void AnimScaleController::resetScale() {
    mScaleVelocityY = 0.0f;
    mScale.set(1.0f);
}

void AnimScaleController::exeAnim() {
    updateScale(mParam->mSpringStrength, mParam->mVelocityDamping);
    if (tryStop()) {
        return;
    }
}

void AnimScaleController::exeStop() {
}

void AnimScaleController::exeHitReaction() {
    if (MR::isFirstStep(this)) {
        resetScale();
        mScaleVelocityY = mParam->mHitReactionVelocityY;
    }

    updateScale(mParam->mHitReactionSpringStrength, mParam->mHitReactionDamping);
    if (tryStop()) {
        return;
    }
}

void AnimScaleController::exeDpdVibration() {
    f32 nerveStep = getNerveStep();
    f32 dpdVibrationValue = MR::getReduceVibrationValue(nerveStep, mParam->mDpdVibrationTime, mParam->mDpdVibrationBase, mParam->mDpdVibrationAmplitude, mParam->mDpdVibrationFrequency);

    mScale.y = dpdVibrationValue;
    mScale.x = MR::sqrt(1.0f / mScale.y);
    mScale.z = mScale.x;
}

void AnimScaleController::exeCrush() {
    mScaleVelocityY = 0.0f;

    f32 nerveRate = MR::calcNerveRate(this, mParam->mCrushDuration);
    f32 crushValue = MR::getConvergeVibrationValue(nerveRate, 1.0f, mParam->mCrushScaleY, 0.3f, 4.0f);

    mScale.y = crushValue;
    mScale.x = MR::sqrt(1.0f / mScale.y);
    mScale.z = mScale.x;

    if (MR::isGreaterStep(this, mParam->mCrushDuration)) {
        setNerve(GET_NERVE(AnimScaleController, AnimScaleControllerNrvStop));
    }
}

// NONMATCHING: same instructions as target (ps_mul of mScale.xy * mScale.xy, fmuls for z), but CW schedules the mScale.z load before the mScale paired load and uses f1/f2 swapped; instruction order and FPR allocation only
void MR::updateBaseScale(LiveActor* pActor, AnimScaleController* pController) {
    TVec3f scale = pController->mScale * pActor->mScale;
    MR::setBaseScale(pActor, scale);
}

bool AnimScaleController::isHitReaction(s32 a1) const {
    if (!isNerve(GET_NERVE(AnimScaleController, AnimScaleControllerNrvHitReaction))) {
        return false;
    }

    if (a1 > 0) {
        return MR::isLessStep(this, a1);
    }
    return true;
}

bool AnimScaleController::tryStop() {
    if (MR::abs(1.0f - mScale.y) < 0.001f && MR::abs(mScaleVelocityY) < 0.001f) {
        resetScale();
        setNerve(GET_NERVE(AnimScaleController, AnimScaleControllerNrvStop));
        return true;
    }
    return false;
}

void AnimScaleController::update() {
    updateNerve();
}

void AnimScaleController::updateScale(f32 f1, f32 f2) {
    f32 updateValue = ((1.0f - mScale.y) * f1 + mScaleVelocityY);
    updateValue = updateValue * f2;
    mScaleVelocityY = updateValue;
    mScale.y += updateValue;

    f32 updateValue2 = mScale.y;

    f32 clampMin = mParam->mMinScaleY;
    f32 clampMax = mParam->mMaxScaleY;
    if (mScale.y < clampMin) {
        updateValue2 = clampMin;
    } else if (mScale.y > clampMax) {
        updateValue2 = clampMax;
    } else {
        updateValue2 = mScale.y;
    }

    mScale.y = updateValue2;
    mScale.x = MR::sqrt(1.0f / mScale.y);
    mScale.z = mScale.x;
}

AnimScaleController::~AnimScaleController() {};
