#include "Game/System/WorldMapResultLocal.hpp"

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

/// Result-map step that zooms the map in for the Grand Galaxy start ("State[グランドギャラクシーマップ開始]").
class WorldMapResultStateGrandGalaxyOpen : public NerveExecutor {
public:
    WorldMapResultStateGrandGalaxyOpen(s32);
    virtual ~WorldMapResultStateGrandGalaxyOpen();

    void init();
    void appear();
    void exeWait();

    // vtable slots 0x10 and 0x14 (the second is called from exeWait); both defined by the owning TU.
    virtual void onStepUpdate();
    virtual void onFovyEnd();

    /* 0x08 */ bool mIsDead;
    /* 0x0C */ s32 mWorldId;
    /* 0x10 */ f32 mFovy;
};

namespace NrvWorldMapResultStateGrandGalaxyOpen {
    WMR_NERVE_DECL(WorldMapResultStateGrandGalaxyOpenWait);
};  // namespace NrvWorldMapResultStateGrandGalaxyOpen

WorldMapResultStateGrandGalaxyOpen::WorldMapResultStateGrandGalaxyOpen(s32 pWorldId)
    : NerveExecutor("State[グランドギャラクシーマップ開始]"), mIsDead(true), mWorldId(pWorldId), mFovy(0.0f) {
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
            onFovyEnd();
        }
    }
}

WorldMapResultStateGrandGalaxyOpen::~WorldMapResultStateGrandGalaxyOpen() {
}

namespace NrvWorldMapResultStateGrandGalaxyOpen {
    WMR_NERVE_DEF(WorldMapResultStateGrandGalaxyOpenWait, WorldMapResultStateGrandGalaxyOpen, Wait);
};  // namespace NrvWorldMapResultStateGrandGalaxyOpen
