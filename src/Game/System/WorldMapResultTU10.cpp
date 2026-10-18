#include "Game/System/GameSequenceFunction.hpp"
#include "Game/System/MiniPlayerRocket.hpp"
#include "Game/System/WorldMapOwner.hpp"
#include "Game/System/WorldMapResultLocal.hpp"

// PLACEHOLDER(core): unnamed SMG2 callee (no symbol name yet); resolve when its owner names it
extern "C" {
    void fn_8001C030(s32);
}

namespace MR {
    void startSystemWipeCircleWithCaptureScreen(s32);
}

/// Result-map step that leaves for Koopa's castle (name "State[クッパ城へ]").
class WorldMapResultStateKoopaCastle : public NerveExecutor {
public:
    WorldMapResultStateKoopaCastle(WorldMapOwner*);
    virtual ~WorldMapResultStateKoopaCastle();

    void init();
    void appear();
    void exeWait();

    /* 0x08 */ bool mIsDead;
    /* 0x0C */ WorldMapOwner* mHost;
};

namespace NrvWorldMapResultStateKoopaCastle {
    WMR_NERVE_DECL(WorldMapResultStateKoopaCastleWait);
};  // namespace NrvWorldMapResultStateKoopaCastle

WorldMapResultStateKoopaCastle::WorldMapResultStateKoopaCastle(WorldMapOwner* pHost) : NerveExecutor("State[クッパ城へ]"), mIsDead(true), mHost(pHost) {
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
        GameSequenceFunction::setWorldMapCurrentNode(mHost->getCurrentNode());
        MR::startSystemWipeCircleWithCaptureScreen(0x5A);
        GameSequenceFunction::requestChangeStageMarioFaceShipBeforeKoopaVs3();
    }

    TVec3f pos(1.0f, 0.0f, 0.0f);
    mHost->faceRocket(&pos);
}

WorldMapResultStateKoopaCastle::~WorldMapResultStateKoopaCastle() {
}

namespace NrvWorldMapResultStateKoopaCastle {
    WMR_NERVE_DEF(WorldMapResultStateKoopaCastleWait, WorldMapResultStateKoopaCastle, Wait);
};  // namespace NrvWorldMapResultStateKoopaCastle
