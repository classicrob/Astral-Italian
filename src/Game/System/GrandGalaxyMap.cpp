#include "Game/System/GrandGalaxyMap.hpp"
#include "Game/Screen/GrandGalaxyMapInfo.hpp"
#include "Game/System/WorldMapOwner.hpp"
#include "Game/System/WorldMapResultLocal.hpp"
#include "Game/Util/LayoutUtil.hpp"
#include "Game/Util/CameraUtil.hpp"

GrandGalaxyMap::GrandGalaxyMap(WorldMapOwner* pOwner) : NerveExecutor("グランドギャラクシーマップ"), mParts(nullptr), mSequence(nullptr), mOwner(pOwner), mIsEnd(false) {}

void GrandGalaxyMap::init(bool isDirect) {
    mParts = new GrandGalaxyMapParts();
    mParts->init(mOwner->getWorldId());
    mSequence = new GrandGalaxyMapSequence(this);
    mSequence->init(isDirect);
    end();
}

void GrandGalaxyMap::start() {
    if (mIsEnd) {
        mIsEnd = false;
        mSequence->start();
        mParts->prepare();
    }
}

void GrandGalaxyMap::end() {
    if (!mIsEnd) {
        mIsEnd = true;
        mSequence->end();
        mParts->finish();
    }
}

void GrandGalaxyMap::update() {
    mSequence->updateNerve();
}

GrandGalaxyMap::~GrandGalaxyMap() {}

WorldMapResultWorld* GrandGalaxyMap::getWorld() const {
    return mParts->mWorld;
}

WorldMapCameraHolder* GrandGalaxyMap::getCamera() const {
    return mOwner->getCamera();
}

s32 GrandGalaxyMap::findWorldNode(WorldMapResultWorld* pWorld, s32 worldId) {
    WorldMapResultMapData* pMapData = pWorld->mMapData;
    PointPartsHolder* pParts = pWorld->mUnit14;
    for (s32 i = 0; i < pMapData->mNum; i++) {
        if (worldId == pParts->getWarpTarget(i)) {
            return i;
        }
    }
    return -1;
}

void MR::setActorPositionFromLayoutPane(LiveActor* pActor, const LayoutActor* pLayout, const char* pPaneName, const TVec2f& rOffset, f32 depth) {
    TVec2f trans;
    MR::copyPaneTrans(&trans, pLayout, pPaneName);
    TVec3f pos;
    MR::calcWorldPositionFromScreen(&pos, trans + rOffset, depth);
    pActor->mPosition.set(pos);
}

bool GrandGalaxyMap::isWorldWarp() const {
    return mSequence->isWorldWarp();
}

bool GrandGalaxyMap::isWorldWarpEnd() const {
    return mSequence->isWorldWarpEnd();
}

void GrandGalaxyMap::startDemo() {
    mOwner->startDemo();
}

void GrandGalaxyMap::endDemo() {
    mOwner->endDemo();
}
