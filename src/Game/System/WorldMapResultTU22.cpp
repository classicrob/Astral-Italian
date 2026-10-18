#include "Game/System/GameSequenceFunction.hpp"
#include "Game/System/WorldMapResultLocal.hpp"

// PLACEHOLDER(core): unnamed SMG2 callees (no symbol name yet); resolve when their owners name them
extern "C" {
    void fn_804F82C0(WorldMapResultCommand*, s32, s32);
    void fn_804F73F0(WorldMapResultCommand*, s32);
    s32 fn_804F7810(WorldMapResultCommand*);
    bool fn_804F72D0(WorldMapResultCommand*);
    bool fn_804E8E00(bool);
    void fn_804F7250(WorldMapResultCommand*);
    void fn_804F72A0(WorldMapResultCommand*);
    void fn_804F6D30(WorldMapResultCommand*, s32);
    s32 fn_804E8A10(WorldMapResultCommand*);
    bool fn_804F7550(WorldMapResultCommand*);
    bool fn_804F7360(WorldMapResultCommand*);
    bool fn_804F7520(WorldMapResultCommand*);
    WorldMapResultWorld* fn_804F60C0(WorldMapResultCommand*);
    void fn_804F6B10(WorldMapResultCommand*);
    void fn_804F74F0(WorldMapResultCommand*);
    void fn_804F7480(WorldMapResultCommand*);
    void fn_804F7490(WorldMapResultCommand*);
}

/// Wait step of the result map ("State[ウェイト]").
WMR_STATE_DECL(WorldMapResultStateWait, virtual void control(); void exeMain(); bool isXXX() const; bool isYYY() const; bool isZZZ() const; void setTarget(s32); void clearTarget();
               /* 0x10 */ u8 mFlagA; /* 0x11 */ u8 _11; /* 0x12 */ u8 mFlagB; /* 0x14 */ s32 mTarget; /* 0x18 */ s32 mTargetSet; /* 0x1C */ s32 mTimer;);

namespace NrvWorldMapResultStateWait {
    WMR_NERVE_DECL(WorldMapResultStateWaitMain);
};  // namespace NrvWorldMapResultStateWait

WorldMapResultStateWait::WorldMapResultStateWait(WorldMapResultCommand* pHost) : ActorStateBase< WorldMapResultCommand >("State[ウェイト]", pHost) {
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
    fn_804F82C0(mHost, -1, -1);
    setNerve(GET_NERVE(WorldMapResultStateWait, WorldMapResultStateWaitMain));
    MR::deactivateDefaultGameLayout();
    fn_804F73F0(mHost, -1);
    GameSequenceFunction::setWorldMapCurrentNode(fn_804F7810(mHost));
    mTimer = 10;
    bool flag = fn_804F72D0(mHost);
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
        fn_804F74F0(mHost);
    }

    if (mTimer > 10) {
        fn_804F7480(mHost);
        if (!mFlagB) {
            fn_804F7490(mHost);
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
    fn_804F7250(mHost);
    fn_804F72A0(mHost);
    fn_804F6D30(mHost, -1);

    if (MR::isStep(this, 30) && !mFlagA) {
        mFlagA = fn_804E8A10(mHost) == 0;
    }

    if (MR::isStep(this, 180)) {
        mFlagA = false;
    }

    if (fn_804F7550(mHost)) {
        mFlagA = false;
    }

    if (mFlagA) {
        mFlagA = fn_804F7360(mHost);
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
        fn_804F60C0(mHost)->startRoutePointOpenSound(pTarget);
    }

    mTarget = pTarget;
    mTargetSet = 2;
}

void WorldMapResultStateWait::clearTarget() {
    if (mTargetSet != 0) {
        fn_804F60C0(mHost)->startRoutePointCloseSound(mTarget);
    }

    mTarget = -1;
}

WorldMapResultStateWait::~WorldMapResultStateWait() {
}

namespace NrvWorldMapResultStateWait {
    WMR_NERVE_DEF(WorldMapResultStateWaitMain, WorldMapResultStateWait, Main);
};  // namespace NrvWorldMapResultStateWait
