#include "Game/System/WorldMapResultLocal.hpp"

extern "C" {
    s32 fn_804E8A10(WorldMapResultCommand*);
    s32 fn_804E8A20(WorldMapResultCommand*);
    s32 fn_804E89F0(WorldMapResultCommand*);
    s32 fn_804F4CB0(s32);
    void fn_804F6B10(WorldMapResultCommand*);
    void fn_804F7480(WorldMapResultCommand*);
    void fn_804F7490(WorldMapResultCommand*);
    void fn_804F72A0(WorldMapResultCommand*);
    bool fn_804F72D0(WorldMapResultCommand*);
    s32 fn_804F74C0(WorldMapResultCommand*);
    void fn_804F7840(WorldMapResultCommand*);
    bool fn_804F7920(WorldMapResultCommand*, s32);
}

namespace MR {
    bool testCorePadTriggerA(s32);
    bool testCorePadTriggerB(s32);
}

/// Result-map step that moves the cursor ("State[移動]").
class WorldMapResultStateMove : public NerveExecutor {
public:
    WorldMapResultStateMove(WorldMapResultCommand*);
    virtual ~WorldMapResultStateMove();

    void init();
    void appear();
    void exeMove();

    // vtable slots 0x10 and 0x14 (the second is called from exeMove); both defined by the owning TU.
    virtual void onStart();
    virtual void onMoveEnd();

    /* 0x08 */ bool mIsDead;
    /* 0x0C */ WorldMapResultCommand* mHost;
    /* 0x10 */ u8 mFlagA;
    /* 0x11 */ bool mFlagB;
    /* 0x14 */ s32 mTarget;
};

namespace NrvWorldMapResultStateMove {
    WMR_NERVE_DECL(WorldMapResultStateMoveMove);
};  // namespace NrvWorldMapResultStateMove

WorldMapResultStateMove::WorldMapResultStateMove(WorldMapResultCommand* pHost) : NerveExecutor("State[移動]") {
    mIsDead = true;
    mHost = pHost;
    mFlagA = false;
    mFlagB = false;
    mTarget = 0;
}

void WorldMapResultStateMove::init() {
    initNerve(GET_NERVE(WorldMapResultStateMove, WorldMapResultStateMoveMove), 0);
    mIsDead = true;
}

void WorldMapResultStateMove::appear() {
    if (fn_804E8A10(mHost) != 0 && fn_804F72D0(mHost) == 0) {
        mTarget = fn_804E8A20(mHost);
    } else {
        mTarget = 0;
    }

    mFlagB = false;
    mFlagA = fn_804F74C0(mHost);
    mIsDead = false;
    fn_804F7840(mHost);
    setNerve(GET_NERVE(WorldMapResultStateMove, WorldMapResultStateMoveMove));
}

void WorldMapResultStateMove::exeMove() {
    fn_804F6B10(mHost);
    fn_804F7480(mHost);
    fn_804F72A0(mHost);

    if (mFlagA) {
        fn_804F7490(mHost);
    }

    if (mTarget != 0) {
        fn_804E89F0(mHost);
    }

    if (MR::testCorePadTriggerA(0) || MR::testCorePadTriggerB(0)) {
        mFlagB = true;
    }

    if (fn_804F7920(mHost, mFlagB)) {
        onMoveEnd();
    }
}

WorldMapResultStateMove::~WorldMapResultStateMove() {
}

namespace NrvWorldMapResultStateMove {
    WMR_NERVE_DEF(WorldMapResultStateMoveMove, WorldMapResultStateMove, Move);
};  // namespace NrvWorldMapResultStateMove
