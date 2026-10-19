#define JGEOMETRY_VEC3_TEMPLATE_CTOR_NOINLINE
#include "Game/Enemy/AnimStampController.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/System/NerveExecutor.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/MtxUtil.hpp"
#include "Game/Util/NerveUtil.hpp"
#include <JSystem/JGeometry/TVec.hpp>
#include <revolution/types.h>

namespace NrvAnimStampController {
    NEW_NERVE(HostNrvAnim, AnimStampController, Anim);
    NEW_NERVE(HostNrvAnimEnd, AnimStampController, AnimEnd);
};  // namespace NrvAnimStampController

AnimStampController::AnimStampController() : NerveExecutor("つぶれアニメコントローラ"), mDirection(0, 0, 1) {
    init(8, 0.2f, 1.5f, 0.125f, 4.0f, 0.12, 4.0f);
}

void AnimStampController::init(s32 a1, f32 a2, f32 a3, f32 a4, f32 a5, f32 a6, f32 a7) {
    mDuration = a1;
    mScaleMtx[3][1] = a2;
    mScaleMtx[3][2] = a3;
    mScaleMtx[3][3] = a4;
    mScaleMtx[4][0] = a5;
    mScaleMtx[4][1] = a6;
    mScaleMtx[4][2] = a7;
    mScaleMtx.identity();
    initNerve(GET_NERVE(AnimStampController, HostNrvAnimEnd));
}

void AnimStampController::startAnim(const TVec3f& rVec) {
    mDirection.set(rVec);
    setNerve(GET_NERVE(AnimStampController, HostNrvAnim));
    mScaleMtx.identity();
}

void AnimStampController::exeAnim() {
    TVec3f vib = TVec3f(mScaleMtx[3][2], mScaleMtx[3][1], mScaleMtx[3][2]);
    if (MR::isLessStep(this, mDuration)) {
        vib.x = MR::getReduceVibrationValue(getNerveStep(), mDuration, mScaleMtx[3][2], mScaleMtx[4][1], mScaleMtx[4][2]);
        vib.y = MR::getReduceVibrationValue(getNerveStep(), mDuration, mScaleMtx[3][1], mScaleMtx[3][3], mScaleMtx[4][0]);
        vib.z = MR::getReduceVibrationValue(getNerveStep(), mDuration, mScaleMtx[3][2], mScaleMtx[4][1], mScaleMtx[4][2]);
    } else {
        setNerve(GET_NERVE(AnimStampController, HostNrvAnimEnd));
    }

    MR::scaleMtxToDir(&mScaleMtx, mDirection, vib);
}

void AnimStampController::exeAnimEnd() {
}

bool AnimStampController::isAnimEnd() const {
    return isNerve(GET_NERVE(AnimStampController, HostNrvAnimEnd));
}
