#include "Game/System/WorldMapResultLocal.hpp"

namespace MR {
    f32 getEaseOutValue(f32, f32, f32, f32);
    f32 getFovy();
    void setFovy(f32);
}

namespace NrvWorldMapResultStateGrandGalaxyEnd {
    WMR_NERVE_DECL(WorldMapResultStateGrandGalaxyEndZoomOut);
    WMR_NERVE_DECL(WorldMapResultStateGrandGalaxyEndWait);
};  // namespace NrvWorldMapResultStateGrandGalaxyEnd

/// Result-map step that zooms the map back out at the end of the Grand Galaxy map ("State[グランドギャラクシーマップ終了]").
class WorldMapResultStateGrandGalaxyEnd : public NerveExecutor {
public:
    WorldMapResultStateGrandGalaxyEnd(s32);
    virtual ~WorldMapResultStateGrandGalaxyEnd();

    void init();
    void appear();
    void exeZoomOut();

    void exeWait() {
        setNerve(GET_NERVE(WorldMapResultStateGrandGalaxyEnd, WorldMapResultStateGrandGalaxyEndWait));
    }

    // vtable slots 0x10 and 0x14 (the second is called from exeZoomOut); both defined by the owning TU.
    virtual void onStepUpdate();
    virtual void onFovyEnd();

    /* 0x08 */ bool mIsDead;
    /* 0x09 */ u8 _9[7];
    /* 0x10 */ f32 mFovy;
};


WorldMapResultStateGrandGalaxyEnd::WorldMapResultStateGrandGalaxyEnd(s32) : NerveExecutor("State[グランドギャラクシーマップ終了]") {
}

void WorldMapResultStateGrandGalaxyEnd::init() {
    initNerve(GET_NERVE(WorldMapResultStateGrandGalaxyEnd, WorldMapResultStateGrandGalaxyEndWait), 0);
    mIsDead = true;
}

void WorldMapResultStateGrandGalaxyEnd::appear() {
    setNerve(GET_NERVE(WorldMapResultStateGrandGalaxyEnd, WorldMapResultStateGrandGalaxyEndZoomOut));
}

void WorldMapResultStateGrandGalaxyEnd::exeZoomOut() {
    if (MR::isFirstStep(this)) {
        mFovy = MR::getFovy();
    }

    s32 steps = 0x14;
    f32 r = MR::getEaseOutValue((f32)getNerveStep() / (f32)steps, 90.0f, mFovy, 1.0f);
    MR::setFovy(r);

    if (MR::isStep(this, 0x14)) {
        onFovyEnd();
    }
}

WorldMapResultStateGrandGalaxyEnd::~WorldMapResultStateGrandGalaxyEnd() {
}

namespace NrvWorldMapResultStateGrandGalaxyEnd {
    WMR_NERVE_DEF(WorldMapResultStateGrandGalaxyEndZoomOut, WorldMapResultStateGrandGalaxyEnd, ZoomOut);
    WMR_NERVE_DEF(WorldMapResultStateGrandGalaxyEndWait, WorldMapResultStateGrandGalaxyEnd, Wait);
};  // namespace NrvWorldMapResultStateGrandGalaxyEnd
