#include "Game/Boss/DodoryuStateWait.hpp"
#include "Game/Util.hpp"
#include "Game/Boss/Dodoryu.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util/ActorAnimUtil.hpp"
#include "Game/Util/ActionSoundUtil.hpp"
#include "Game/Util/ActorAnimUtil.hpp"
#include "Game/Util/ActorShadowUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/SceneUtil.hpp"
#include "Game/Util/ScreenUtil.hpp"
#include "Game/Util/SequenceUtil.hpp"
#include "Game/Util/SoundUtil.hpp"

namespace {
    NEW_NERVE(DodoryuStateWaitNrvWait, DodoryuStateWait, Wait);
};  // namespace

DodoryuStateWait::DodoryuStateWait(Dodoryu* pHost, const char* pName) : DodoryuStateBase(pHost, pName), mNextStateCounter() {
    initWithoutIter();
}

void DodoryuStateWait::init(const JMapInfoIter& rIter) {
    MR::invalidateClipping(this);
    initNerve(GET_NERVE_ANON(DodoryuStateWaitNrvWait));
    makeActorAppeared();
}

void DodoryuStateWait::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::invalidateShadowAll(mHost);
        MR::startAction(mHost, "UnderGroundWalk");
        MR::startBtp(mHost, "Normal");
        mHost->setHillAppearNumHalf();
        mHost->startHill();
        mHost->shiftMoveStateRail(8.0f);

        if (MR::isStageSuddenDeathDodoryu() && MR::hasRetryGalaxySequence()) {
            MR::forceCloseWipeCircle();
        }
    }

    MR::startActionSound(mHost, "SE_BM_LV_DODORYU_MOVE_U_SHALLOW", -1, -1, -1);

    if (mHost->mBattleStarted && !MR::isPlayerInRush()) {
        mNextStateCounter++;
    } else {
        mNextStateCounter = 0;
    }

    if (mNextStateCounter > 30) {
        if (MR::isStageSuddenDeathDodoryu() && MR::hasRetryGalaxySequence()) {
            MR::openWipeCircle();
        }

        mHost->nextState();
    }
}
