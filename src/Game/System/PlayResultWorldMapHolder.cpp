#include "Game/System/PlayResultWorldMapHolder.hpp"
#include "Game/System/WorldMapResultLocal.hpp"

PlayResultWorldMapHolder::PlayResultWorldMapHolder() : NameObj("\x83v\x83\x8c\x83""C\x8c\x8b\x89\xca\x95\xdb\x8e\x9d[\x83\x8f\x81[\x83\x8b\x83h\x83}\x83""b\x83v]") {
    mRoute = nullptr;
    mRoute = new WorldMapResultRoute();
    reset();
}

PlayResultWorldMapHolder::~PlayResultWorldMapHolder() {
}

void PlayResultWorldMapHolder::reset() {
    mCurrentNode = 0;
    mTicoFatId = -1;
    mTicoFatStarPieceNum = 0;
    clearGalaxyFlags();
    mRoute->mCount = 0;
    mIsAfterGameOver = false;
    mIsAfterFileSelect = false;
    mFlag22C = false;
    mFlag22D = false;
    mEnterType = 5;
}

void PlayResultWorldMapHolder::setTicoFatStarPieceId(s32 id) {
    mTicoFatId = id;
}

void PlayResultWorldMapHolder::setTicoFatStarPieceRequirement(s32 num) {
    mTicoFatStarPieceNum = num;
}

void PlayResultWorldMapHolder::setGalaxyFlag(s32 worldNo, s32 index) {
    mGalaxyFlags[worldNo - 1][index] = true;
}

bool PlayResultWorldMapHolder::isGalaxyFlag(s32 worldNo, s32 index) const {
    return mGalaxyFlags[worldNo - 1][index];
}

void PlayResultWorldMapHolder::clearGalaxyFlags() {
    for (s32 i = 0; i < 8; i++) {
        for (s32 j = 0; j < 0x40; j++) {
            mGalaxyFlags[i][j] = false;
        }
    }
}

void PlayResultWorldMapHolder::storeRoute(const WorldMapResultRoute* pRoute) {
    *mRoute = *pRoute;
}

void PlayResultWorldMapHolder::loadRoute(WorldMapResultRoute* pRoute) const {
    *pRoute = *mRoute;
}

void PlayResultWorldMapHolder::clearRouteCount() {
    mRoute->mCount = 0;
}
