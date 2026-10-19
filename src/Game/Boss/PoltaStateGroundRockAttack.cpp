#include "Game/Boss/PoltaStateGroundRockAttack.hpp"
#include "Game/Util.hpp"
#include "Game/Boss/Polta.hpp"
#include "Game/Boss/PoltaFunction.hpp"
#include "Game/Boss/PoltaStateAttackGround.hpp"
#include "Game/LiveActor/ActorStateBase.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util/ActorMovementUtil.hpp"
#include "Game/Util/LayoutUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/NerveUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/SoundUtil.hpp"

namespace NrvPoltaStateGenerateBombTeresa {
    NEW_NERVE(PoltaStateGroundRockAttackNrvSign, PoltaStateGroundRockAttack, Sign);
    NEW_NERVE(PoltaStateGroundRockAttackNrvGenerate, PoltaStateGroundRockAttack, Generate);
    NEW_NERVE(PoltaStateGroundRockAttackNrvEnd, PoltaStateGroundRockAttack, End);
};  // namespace NrvPoltaStateGenerateBombTeresa

PoltaStateGroundRockAttack::PoltaStateGroundRockAttack(Polta* pPolta)
    : ActorStateBase< Polta >("[state]ポルタ地面岩攻撃", pPolta), mRockCount(4), mIsClockwise(false) {
    initNerve(GET_NERVE(PoltaStateGenerateBombTeresa, PoltaStateGroundRockAttackNrvSign));
}

void PoltaStateGroundRockAttack::appear() {
    mIsDead = false;
    setNerve(GET_NERVE(PoltaStateGenerateBombTeresa, PoltaStateGroundRockAttackNrvSign));
}

void PoltaStateGroundRockAttack::exeSign() {
    if (MR::isFirstStep(this)) {
        PoltaFunction::requestStartControllArm(getHost());
        PoltaFunction::startAction(getHost(), "GenerateGroundRockSign", true);
        MR::startActionSound(getHost(), "BvPoltaPrepGndRock");
    }
    getHost()->rotateToPlayer();
    if (MR::isActionEnd(getHost())) {
        setNerve(GET_NERVE(PoltaStateGenerateBombTeresa, PoltaStateGroundRockAttackNrvGenerate));
    }
}

void PoltaStateGroundRockAttack::exeGenerate() {
    if (MR::isFirstStep(this)) {
        PoltaFunction::startAction(getHost(), "GenerateGroundRockStart", true);
        MR::zeroVelocity(getHost());
        mIsClockwise = MR::isClockwiseToDir(getHost(), *MR::getPlayerLastMove(), *MR::getPlayerPos() - getHost()->mPosition);
    }

    if (MR::isBckOneTimeAndStopped(getHost())) {
        PoltaFunction::startAction(getHost(), "GenerateGroundRock", true);
        MR::startActionSound(getHost(), "BvPoltaGenGndRock");
        MR::startActionSound(getHost(), "BmPoltaGenGndRock");
    }
    if (MR::isLessEqualStep(this, 15 * (mRockCount - 1)) && MR::isIntervalStep(this, 15)) {
        s32 v14 = getNerveStep() / 15;
        s32 v16 = v14 % 4;

        if (v14 > 0 && !v16) {
            mIsClockwise = !mIsClockwise;
        }
        PoltaFunction::appearGroundRock(getHost(), (850.0f + (400.0f * v16)), mIsClockwise ? 20.0f : -20.0f);
    }
    if (MR::isGreaterStep(this, 15 * mRockCount + 90)) {
        setNerve(GET_NERVE(PoltaStateGenerateBombTeresa, PoltaStateGroundRockAttackNrvEnd));
    }
}

void PoltaStateGroundRockAttack::exeEnd() {
    if (MR::isFirstStep(this)) {
        PoltaFunction::startAction(getHost(), "GenerateGroundRockEnd", true);
    }
    if (MR::isActionEnd(getHost())) {
        kill();
    }
}

PoltaStateGroundRockAttack::~PoltaStateGroundRockAttack() {
}
