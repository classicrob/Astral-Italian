#include "Game/Screen/MapGalaxyLayouts.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"

namespace NrvMapGalaxyWorld {
    class MapGalaxyWorldNrvAppear : public Nerve { public: virtual void execute(Spine*) const; static MapGalaxyWorldNrvAppear sInstance; };
    MapGalaxyWorldNrvAppear MapGalaxyWorldNrvAppear::sInstance;
    class MapGalaxyWorldNrvWait : public Nerve { public: virtual void execute(Spine*) const; static MapGalaxyWorldNrvWait sInstance; };
    MapGalaxyWorldNrvWait MapGalaxyWorldNrvWait::sInstance;
    class MapGalaxyWorldNrvDisappear : public Nerve { public: virtual void execute(Spine*) const; static MapGalaxyWorldNrvDisappear sInstance; };
    MapGalaxyWorldNrvDisappear MapGalaxyWorldNrvDisappear::sInstance;
}
using namespace NrvMapGalaxyWorld;

MapGalaxyWorld::MapGalaxyWorld() : SimpleLayout("ワールドマップ基本情報", "MapGalaxyWorld", 1, 0x4E), mKeepFrames(0) {
    MR::registerDemoSimpleCastAll(this);
}

void MapGalaxyWorld::requestAppear() {
    if (MR::isDead(this)) {
        MR::startAnim(this, "Appear", 0);
        setNerve(&MapGalaxyWorldNrvAppear::sInstance);
        appear();
    }
    mKeepFrames = 1;
}

void MapGalaxyWorld::init(const JMapInfoIter&) {
    initNerve(&MapGalaxyWorldNrvWait::sInstance);
    LayoutActor::kill();
}

void MapGalaxyWorld::appear() { LayoutActor::appear(); }
void MapGalaxyWorld::kill() { mKeepFrames = 0; LayoutActor::kill(); }

void MapGalaxyWorld::control() {
    if (isNerve(&MapGalaxyWorldNrvWait::sInstance) && mKeepFrames == 0) {
        MR::startAnim(this, "End", 0);
        setNerve(&MapGalaxyWorldNrvDisappear::sInstance);
        return;
    }
    const u32 remaining = static_cast<u32>(mKeepFrames) - 1;
    mKeepFrames = remaining & ((remaining >> 31) - 1);
}

void MapGalaxyWorld::setWorldName(const wchar_t* name) { MR::setTextBoxMessageRecursive(this, "MapGalaxyWorld", name); }
MapGalaxyWorld::~MapGalaxyWorld() {}

void NrvMapGalaxyWorld::MapGalaxyWorldNrvAppear::execute(Spine* spine) const {
    MapGalaxyWorld* actor = static_cast<MapGalaxyWorld*>(spine->mExecutor);
    if (MR::isAnimStopped(actor, 0)) actor->setNerve(&MapGalaxyWorldNrvWait::sInstance);
}
void NrvMapGalaxyWorld::MapGalaxyWorldNrvWait::execute(Spine*) const {}
void NrvMapGalaxyWorld::MapGalaxyWorldNrvDisappear::execute(Spine* spine) const {
    MapGalaxyWorld* actor = static_cast<MapGalaxyWorld*>(spine->mExecutor);
    if (MR::isAnimStopped(actor, 0)) actor->kill();
}
