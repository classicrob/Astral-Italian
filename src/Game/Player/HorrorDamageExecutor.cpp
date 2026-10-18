#include "Game/Player/HorrorDamageExecutor.hpp"
#include "Game/Player/MarioDamage.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util/NerveUtil.hpp"

namespace NrvHorrorDamageExecutor {
    NEW_NERVE(HorrorDamageExecutorNrvBoneBreakStart, HorrorDamageExecutor, BoneBreakStart);
    NEW_NERVE(HorrorDamageExecutorNrvBoneBreakWait, HorrorDamageExecutor, BoneBreakWait);
    NEW_NERVE(HorrorDamageExecutorNrvBoneBreakEnd, HorrorDamageExecutor, BoneBreakEnd);
}

HorrorDamageExecutor::HorrorDamageExecutor(MarioDamage* damage) : NerveExecutor("ホラーダメージ内部状態管理"), mDamage(damage), mBreakDuration(0), mBreakEnded(false) {
    initNerve(&NrvHorrorDamageExecutor::HorrorDamageExecutorNrvBoneBreakStart::sInstance, 0);
}

HorrorDamageExecutor::~HorrorDamageExecutor() {}

void HorrorDamageExecutor::startBreak() {
    mBreakEnded = false;
    setNerve(&NrvHorrorDamageExecutor::HorrorDamageExecutorNrvBoneBreakStart::sInstance);
}

bool HorrorDamageExecutor::isBreakEnd() const { return mBreakEnded; }

void HorrorDamageExecutor::exeBoneBreakStart() {
    if (MR::isFirstStep(this)) mDamage->changeAnimation("骨バラバラ開始", static_cast<const char*>(nullptr));
    if (!mDamage->isAnimationTerminate("骨バラバラ開始")) {
        mDamage->changeAnimation("骨バラバラ継続", static_cast<const char*>(nullptr));
        setNerve(&NrvHorrorDamageExecutor::HorrorDamageExecutorNrvBoneBreakWait::sInstance);
    }
}

void HorrorDamageExecutor::exeBoneBreakWait() {
    if (MR::isFirstStep(this)) mBreakDuration = 180;
    if (mDamage->checkTrgA()) mBreakDuration -= 10;
    if (MR::isGreaterEqualStep(this, mBreakDuration)) {
        mDamage->changeAnimation("骨バラバラ終了", static_cast<const char*>(nullptr));
        setNerve(&NrvHorrorDamageExecutor::HorrorDamageExecutorNrvBoneBreakEnd::sInstance);
    }
}

void HorrorDamageExecutor::exeBoneBreakEnd() {
    if (mDamage->isAnimationTerminate("骨バラバラ終了")) mBreakEnded = true;
}
