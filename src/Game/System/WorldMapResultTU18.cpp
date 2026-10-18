#include "Game/System/WorldMapResultLocal.hpp"
#include "Game/System/MiniPlayerRocket.hpp"
#include "Game/System/WorldMapOwner.hpp"

extern "C" {
    // PLACEHOLDER(core): mapped callee signature (symbols.txt name) does not match this call's arguments; resolve when the callee's real signature is confirmed
    s32 fn_804E89F0(WorldMapOwner*);
    // PLACEHOLDER(core): the symbol at 0x804E8A20 is named getGalaxyInfoName() (const char*) but this caller stores the result in an s32; resolve when the return type is confirmed
    s32 fn_804E8A20(WorldMapOwner*);
    s32 fn_804F4CB0(s32);
    void fn_804F6B10(WorldMapOwner*);
    bool fn_804F7920(WorldMapOwner*, s32);
}

namespace MR {
    bool testCorePadTriggerA(s32);
    bool testCorePadTriggerB(s32);
}

/// Result-map step that moves the cursor ("State[移動]").
class WorldMapResultStateMove : public NerveExecutor {
public:
    WorldMapResultStateMove(WorldMapOwner*);
    virtual ~WorldMapResultStateMove();

    void init();
    void appear();
    void exeMove();

    // vtable slots 0x10 and 0x14 (the second is called from exeMove); both defined by the owning TU.
    virtual void onStart();
    virtual void onMoveEnd();

    /* 0x08 */ bool mIsDead;
    /* 0x0C */ WorldMapOwner* mHost;
    /* 0x10 */ u8 mFlagA;
    /* 0x11 */ bool mFlagB;
    /* 0x14 */ s32 mTarget;
};

namespace NrvWorldMapResultStateMove {
    WMR_NERVE_DECL(WorldMapResultStateMoveMove);
};  // namespace NrvWorldMapResultStateMove

WorldMapResultStateMove::WorldMapResultStateMove(WorldMapOwner* pHost) : NerveExecutor("State[移動]") {
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
    if (mHost->isGalaxyInfoAppeared() != 0 && mHost->isCurrentGalaxyInfoShown() == 0) {
        mTarget = fn_804E8A20(mHost);
    } else {
        mTarget = 0;
    }

    mFlagB = false;
    mFlagA = mHost->isZoomButtonAppeared();
    mIsDead = false;
    mHost->resetCameraMove();
    setNerve(GET_NERVE(WorldMapResultStateMove, WorldMapResultStateMoveMove));
}

void WorldMapResultStateMove::exeMove() {
    fn_804F6B10(mHost);
    mHost->requestAppearWorldName();
    mHost->startAllPointingAnim();

    if (mFlagA) {
        mHost->requestAppearZoomButton();
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
