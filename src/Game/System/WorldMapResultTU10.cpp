#include "Game/System/WorldMapResultLocal.hpp"

extern "C" {
    void fn_8001C030(s32);
    void fn_804D72A0();
    void fn_804F7650(s32, TVec3f*, f32);
    void fn_804F7810(s32);
}

namespace MR {
    void startSystemWipeCircleWithCaptureScreen(s32);
}

namespace GameSequenceFunction {
    void requestChangeStageMarioFaceShipBeforeKoopaVs3();
}

/// Result-map step that leaves for Koopa's castle (name "State[クッパ城へ]").
class WorldMapResultStateKoopaCastle : public NerveExecutor {
public:
    WorldMapResultStateKoopaCastle(s32);
    virtual ~WorldMapResultStateKoopaCastle();

    void init();
    void appear();
    void exeWait();

    /* 0x08 */ bool mIsDead;
    /* 0x0C */ s32 mWorldId;
};

namespace NrvWorldMapResultStateKoopaCastle {
    WMR_NERVE_DECL(WorldMapResultStateKoopaCastleWait);
};  // namespace NrvWorldMapResultStateKoopaCastle

WorldMapResultStateKoopaCastle::WorldMapResultStateKoopaCastle(s32 pWorldId) : NerveExecutor("State[クッパ城へ]"), mIsDead(true), mWorldId(pWorldId) {
}

void WorldMapResultStateKoopaCastle::init() {
    initNerve(GET_NERVE(WorldMapResultStateKoopaCastle, WorldMapResultStateKoopaCastleWait), 0);
    mIsDead = true;
}

void WorldMapResultStateKoopaCastle::appear() {
    mIsDead = false;
    setNerve(GET_NERVE(WorldMapResultStateKoopaCastle, WorldMapResultStateKoopaCastleWait));
}

void WorldMapResultStateKoopaCastle::exeWait() {
    if (MR::isFirstStep(this)) {
        fn_8001C030(0x1E);
    }

    if (MR::isStep(this, 0x1E)) {
        fn_804F7810(mWorldId);
        fn_804D72A0();
        MR::startSystemWipeCircleWithCaptureScreen(0x5A);
        GameSequenceFunction::requestChangeStageMarioFaceShipBeforeKoopaVs3();
    }

    TVec3f pos(1.0f, 0.0f, 0.0f);
    fn_804F7650(mWorldId, &pos, 1.0f);
}

WorldMapResultStateKoopaCastle::~WorldMapResultStateKoopaCastle() {
}

namespace NrvWorldMapResultStateKoopaCastle {
    WMR_NERVE_DEF(WorldMapResultStateKoopaCastleWait, WorldMapResultStateKoopaCastle, Wait);
};  // namespace NrvWorldMapResultStateKoopaCastle
