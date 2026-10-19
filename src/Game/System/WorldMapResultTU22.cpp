#include "Game/System/GameSequenceFunction.hpp"
#include "Game/System/WorldMapFunction.hpp"
#include "Game/System/WorldMapResultLocal.hpp"
#include "Game/System/WorldMapOwner.hpp"

namespace NrvWorldMapResultStateWait {
    WMR_NERVE_DECL(WorldMapResultStateWaitMain);
};  // namespace NrvWorldMapResultStateWait

WorldMapResultStateWait::WorldMapResultStateWait(WorldMapOwner* pHost) : ActorStateBase< WorldMapOwner >("State[ウェイト]", pHost) {
    mFlagA = false;
    mFlagB = true;
    mTarget = -1;
    mTargetSet = 0;
    mTimer = 0;
}

void WorldMapResultStateWait::init() {
    initNerve(GET_NERVE(WorldMapResultStateWait, WorldMapResultStateWaitMain), 0);
    mIsDead = true;
}

void WorldMapResultStateWait::appear() {
    mIsDead = false;
    mHost->startNodeCamera(-1, -1);
    setNerve(GET_NERVE(WorldMapResultStateWait, WorldMapResultStateWaitMain));
    MR::deactivateDefaultGameLayout();
    mHost->setupWorldName(-1);
    GameSequenceFunction::setWorldMapCurrentNode(mHost->getCurrentNode());
    mTimer = 10;
    bool flag = mHost->isCurrentGalaxyInfoShown();
    mFlagA = flag;
    mFlagB = WorldMapFunction::isMapEventFlagOff();
}

void WorldMapResultStateWait::kill() {
    mIsDead = true;
}

// NONMATCHING: timer/count update scheduled in a different order (10 insns differ) (FP order/grouping checked: no FP instructions).
void WorldMapResultStateWait::control() {
    mHost->updateParts();

    s32 count = mTargetSet - 1;
    s32 sign = static_cast< u32 >(count) >> 31;
    mTargetSet = count & (sign - 1);

    s32 timer = mTimer + 1;
    s32 next = 1000;
    if (timer < 1000) {
        next = timer;
    }
    mTimer = next;

    if (mTimer > 40) {
        mHost->requestAppearCoinsAndLives();
    }

    if (mTimer > 10) {
        mHost->requestAppearWorldName();
        if (!mFlagB) {
            mHost->requestAppearZoomButton();
        }
    }
}

bool WorldMapResultStateWait::isXXX() const {
    if (mFlagB) {
        return false;
    }

    if (MR::isGreaterStep(this, 10)) {
        if (MR::testCorePadTriggerDown(0) || isYYY()) {
            return true;
        }
    }

    return false;
}

bool WorldMapResultStateWait::isZZZ() const {
    if (isNerve(GET_NERVE(WorldMapResultStateWait, WorldMapResultStateWaitMain)) && MR::isLessEqualStep(this, 5)) {
        return false;
    }

    return true;
}

// NONMATCHING: mFlagA = (fn_804E8A10() == 0) keeps a different cntlzw/srwi temp shape
void WorldMapResultStateWait::exeMain() {
    mHost->showCurrentNamePlate();
    mHost->startAllPointingAnim();
    mHost->tryTalkPoint(-1);

    if (MR::isStep(this, 30) && !mFlagA) {
        mFlagA = mHost->isGalaxyInfoAppeared() == 0;
    }

    if (MR::isStep(this, 180)) {
        mFlagA = false;
    }

    if (mHost->isZoomButtonPointing()) {
        mFlagA = false;
    }

    if (mFlagA) {
        mFlagA = mHost->tryShowCurrentGalaxyInfo();
    }
}

bool WorldMapResultStateWait::isYYY() const {
    if (MR::testCorePadTriggerA(0) && mHost->tryDecideZoomButton()) {
        return true;
    }

    return false;
}

void WorldMapResultStateWait::setTarget(s32 pTarget) {
    if (mTargetSet == 0) {
        mHost->getWorld()->startRoutePointOpenSound(pTarget);
    }

    mTarget = pTarget;
    mTargetSet = 2;
}

void WorldMapResultStateWait::clearTarget() {
    if (mTargetSet != 0) {
        mHost->getWorld()->startRoutePointCloseSound(mTarget);
    }

    mTarget = -1;
}

WorldMapResultStateWait::~WorldMapResultStateWait() {
}

namespace NrvWorldMapResultStateWait {
    WMR_NERVE_DEF(WorldMapResultStateWaitMain, WorldMapResultStateWait, Main);
};  // namespace NrvWorldMapResultStateWait
