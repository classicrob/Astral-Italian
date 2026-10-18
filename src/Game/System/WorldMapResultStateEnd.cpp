#include "Game/System/WorldMapResultLocal.hpp"

WorldMapResultStateEnd::WorldMapResultStateEnd(WorldMapResultCommand* pHost) : ActorStateBase< WorldMapResultCommand >("State[リザルトマップ終了]", pHost) {
}

void WorldMapResultStateEnd::init() {
    initNerve(GET_NERVE(WorldMapResultStateEnd, WorldMapResultStateEndWait), 0);
    mIsDead = true;
}

void WorldMapResultStateEnd::appear() {
    MR::closeSystemWipeMario(-1);
    mIsDead = false;
    setNerve(GET_NERVE(WorldMapResultStateEnd, WorldMapResultStateEndWait));
}

void WorldMapResultStateEnd::kill() {
    mHost->endResult();

    if (fn_804D8320()) {
        MR::forceOpenSystemWipeMario();
        MR::forceCloseWipeFade();
    } else {
        MR::openSystemWipeMario(-1);
    }

    mIsDead = true;
}

WorldMapResultStateEnd::~WorldMapResultStateEnd() {
}


inline void WorldMapResultStateEnd::exeWait() {
    if (fn_80059100()) {
        setNerve(GET_NERVE(WorldMapResultStateEnd, WorldMapResultStateEndEnd));
    } else {
        fn_8005AC70(12, 30);
    }
}

inline void WorldMapResultStateEnd::exeEnd() {
    if (MR::isStep(this, getResultEndStep())) {
        kill();
    } else {
        fn_8005AC70(12, 30);
    }
}

namespace NrvWorldMapResultStateEnd {
    WMR_NERVE_DEF(WorldMapResultStateEndWait, WorldMapResultStateEnd, Wait);
    WMR_NERVE_DEF(WorldMapResultStateEndEnd, WorldMapResultStateEnd, End);
};  // namespace NrvWorldMapResultStateEnd
