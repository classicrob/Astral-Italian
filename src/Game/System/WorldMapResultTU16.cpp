#include "Game/System/WorldMapResultLocal.hpp"
#include "Game/System/MiniPlayerRocket.hpp"
#include "Game/System/WorldMapOwner.hpp"
#include "Game/System/WorldMapResultStateOpenGalaxy.hpp"

namespace MR {
    void hidePlayer();
    void showPlayer();
}

extern s32 sOpenGalaxyStep;

WorldMapResultStateOpenGalaxy::WorldMapResultStateOpenGalaxy(WorldMapOwner* pHost) : ActorStateBaseInterface("State[ギャラクシーをオープンする]") {
    mIsDead = true;
    mHost = pHost;
    _10 = 0.0f;
    _14 = 0.0f;
    _18 = 0.0f;
}

void WorldMapResultStateOpenGalaxy::init() {
    initNerve(GET_NERVE(WorldMapResultStateOpenGalaxy, WorldMapResultStateOpenGalaxyOpen), 0);
    mIsDead = true;
}

void WorldMapResultStateOpenGalaxy::appear() {
    mIsDead = false;
    setNerve(GET_NERVE(WorldMapResultStateOpenGalaxy, WorldMapResultStateOpenGalaxyOpen));
    if (mHost->tryOpenCurrentGalaxy()) {
        mHost->startDemo();
        mHost->changeRocketAction("Wait");
        MR::hidePlayer();
    } else {
        mIsDead = true;
    }
}

void WorldMapResultStateOpenGalaxy::kill() {
    mIsDead = true;
}

void WorldMapResultStateOpenGalaxy::exeOpen() {
    MR::isFirstStep(this);
    if (MR::isGreaterEqualStep(this, sOpenGalaxyStep)) {
        mHost->endDemo();
        MR::showPlayer();
        kill();
    }
}

WorldMapResultStateOpenGalaxy::~WorldMapResultStateOpenGalaxy() {
}

namespace NrvWorldMapResultStateOpenGalaxy {
    WMR_NERVE_DEF(WorldMapResultStateOpenGalaxyOpen, WorldMapResultStateOpenGalaxy, Open);
    WMR_NERVE_DEF(WorldMapResultStateOpenGalaxyWait, WorldMapResultStateOpenGalaxy, Wait);
    WMR_NERVE_DEF(WorldMapResultStateOpenGalaxyClose, WorldMapResultStateOpenGalaxy, Close);
};  // namespace NrvWorldMapResultStateOpenGalaxy
