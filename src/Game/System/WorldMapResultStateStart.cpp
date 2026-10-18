#include "Game/System/WorldMapResultLocal.hpp"
#include "Game/Util/SoundUtil.hpp"

WorldMapResultStateStart::WorldMapResultStateStart(WorldMapResultCommand* pHost)
    : ActorStateBase< WorldMapResultCommand >("State[リザルトマップ開始]", pHost) {
}

void WorldMapResultStateStart::init() {
    initNerve(GET_NERVE(WorldMapResultStateStart, WorldMapResultStateStartWait), 0);
    mIsDead = true;
}

void WorldMapResultStateStart::appear() {
    MR::closeSystemWipeMario(-1);
    mIsDead = false;
    setNerve(GET_NERVE(WorldMapResultStateStart, WorldMapResultStateStartWait));
    MR::stopSeCategories(0x32);
}

void WorldMapResultStateStart::kill() {
    MR::openSystemWipeMario(-1);
    mIsDead = true;
}

WorldMapResultStateStart::~WorldMapResultStateStart() {
}

inline void WorldMapResultStateStart::exeWait() {
    if (fn_80059100()) {
        setNerve(GET_NERVE(WorldMapResultStateStart, WorldMapResultStateStartEnd));
    } else {
        MR::setSeVolumeSetLevel(12, 30);
    }
}

inline void WorldMapResultStateStart::exeEnd() {
    if (MR::isStep(this, getResultEndStep())) {
        kill();
    } else {
        MR::setSeVolumeSetLevel(12, 30);
    }
}

namespace NrvWorldMapResultStateStart {
    WMR_NERVE_DEF(WorldMapResultStateStartWait, WorldMapResultStateStart, Wait);
    WMR_NERVE_DEF(WorldMapResultStateStartEnd, WorldMapResultStateStart, End);
};  // namespace NrvWorldMapResultStateStart
