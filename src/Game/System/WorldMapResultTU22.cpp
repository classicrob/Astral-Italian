#include "Game/System/GameSequenceFunction.hpp"
#include "Game/System/WorldMapResultLocal.hpp"
#include "Game/System/WorldMapOwner.hpp"

// PLACEHOLDER(core): unnamed SMG2 callees (no symbol name yet); resolve when their owners name them
extern "C" {
    bool fn_804E8E00(bool);
    s32 fn_804E8A10(WorldMapOwner*);
    bool fn_804F7520(WorldMapOwner*);
    void fn_804F6B10(WorldMapOwner*);
}

/// Wait step of the result map ("State[ウェイト]").
WMR_OWNER_STATE_DECL(WorldMapResultStateWait, virtual void control(); void exeMain(); bool isXXX() const; bool isYYY() const; bool isZZZ() const; void setTarget(s32); void clearTarget();
               /* 0x10 */ u8 mFlagA; /* 0x11 */ u8 _11; /* 0x12 */ u8 mFlagB; /* 0x14 */ s32 mTarget; /* 0x18 */ s32 mTargetSet; /* 0x1C */ s32 mTimer;);

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
    mFlagB = fn_804E8E00(flag);
}

void WorldMapResultStateWait::kill() {
    mIsDead = true;
}

// NONMATCHING: timer/count update scheduled in a different order (10 insns differ)
void WorldMapResultStateWait::control() {
    fn_804F6B10(mHost);

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
    if (MR::testCorePadTriggerA(0) && fn_804F7520(mHost)) {
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
