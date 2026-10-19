#include "Game/System/GrandGalaxyMap.hpp"
#include "Game/Screen/GrandGalaxyMapInfo.hpp"
#include "Game/System/WorldMapResultLocal.hpp"

GrandGalaxyMapParts::GrandGalaxyMapParts() : mInfo(nullptr), mGalaxies(nullptr) {}

void GrandGalaxyMapParts::init(s32 worldId) {
    mWorld = new WorldMapResultWorld(8);
    mWorld->initWorld(1);
    s32 node = 0;
    if (worldId < 8) {
        node = GrandGalaxyMap::findWorldNode(mWorld, worldId);
    }
    mInfo = new GrandGalaxyMapInfo();
    mInfo->initWithoutIter();
    mGalaxies = new GrandGalaxyMapGalaxyHolder();
    mGalaxies->init();
    mWorld->refreshNode(node);
}

void GrandGalaxyMapParts::prepare() {
    mWorld->prepare();
}

void GrandGalaxyMapParts::finish() {
    mWorld->finish();
}
