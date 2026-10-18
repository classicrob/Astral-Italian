#include "Game/System/WorldMapOwner.hpp"
#include "Game/System/WorldMapResultLocal.hpp"
#include "Game/System/WorldMapResultStateGrandGalaxyOpen.hpp"

// PLACEHOLDER(core): unnamed SMG2 callees (no symbol name yet); resolve when their owners name them
extern "C" {
    void fn_80059270();
    void* fn_80059330();
}

namespace MR {
    f32 getEaseInValue(f32, f32, f32, f32);
    f32 getFovy();
    void setFovy(f32);
    void setStageBGMStateBit(u32);
}

WorldMapResultStateGrandGalaxyOpen::WorldMapResultStateGrandGalaxyOpen(WorldMapOwner* pHost)
    : ActorStateBase< WorldMapOwner >("State[グランドギャラクシーマップ開始]", pHost), mFovy(0.0f) {
}

void WorldMapResultStateGrandGalaxyOpen::init() {
    initNerve(GET_NERVE(WorldMapResultStateGrandGalaxyOpen, WorldMapResultStateGrandGalaxyOpenWait), 0);
    mIsDead = true;
}

void WorldMapResultStateGrandGalaxyOpen::appear() {
    mIsDead = false;
    mFovy = MR::getFovy();
    fn_80059270();
    setNerve(GET_NERVE(WorldMapResultStateGrandGalaxyOpen, WorldMapResultStateGrandGalaxyOpenWait));
}

void WorldMapResultStateGrandGalaxyOpen::exeWait() {
    MR::isFirstStep(this);
    MR::setStageBGMStateBit(0x100);

    s32 steps = 0x1E;
    f32 v = mFovy;
    f32 r = MR::getEaseInValue((f32)getNerveStep() / (f32)steps, mFovy, 179.0f, 1.0f);
    if (r < v) {
    } else {
        v = 179.0f;
        if (r > v) {
        } else {
            v = r;
        }
    }
    MR::setFovy(v);

    if (MR::isGreaterEqualStep(this, 0x1E)) {
        if (fn_80059330() != NULL) {
            MR::setFovy(mFovy);
            kill();
        }
    }
}

WorldMapResultStateGrandGalaxyOpen::~WorldMapResultStateGrandGalaxyOpen() {
}

namespace NrvWorldMapResultStateGrandGalaxyOpen {
    WMR_NERVE_DEF(WorldMapResultStateGrandGalaxyOpenWait, WorldMapResultStateGrandGalaxyOpen, Wait);
};  // namespace NrvWorldMapResultStateGrandGalaxyOpen
