#ifdef GALAXY_NATIVE
#include <petari/wm_route_diag.hpp>
#endif
#include "Game/System/GrandGalaxyMap.hpp"
#include "Game/Screen/GrandGalaxyMapInfo.hpp"
#include "Game/System/GalaxyStatusAccessor.hpp"
#include "Game/System/GameDataConst.hpp"
#include "Game/System/GameDataFunction.hpp"
#include "Game/System/GrandGalaxyMapGalaxy.hpp"
#include "Game/System/MiniPlayerRocket.hpp"
#include "Game/System/ScenarioDataParser.hpp"
#include "Game/System/WorldMapFunction.hpp"
#include "Game/System/WorldMapOwner.hpp"
#include "Game/System/WorldMapResultLocal.hpp"
#include "Game/Util.hpp"
#include "Game/Camera/CameraTargetArg.hpp"
#include <cstdio>

bool GrandGalaxyMap::tryDecide(s32 node) {
    WorldMapCameraHolder* pCamera = getCamera();
    if (node >= 0 && MR::testCorePadTriggerA(0) && node != pCamera->mCurrent && getWorld()->isPathReachable(node)) {
        getWorld()->mUnit18->emitGoEffect(node);
        pCamera->mNext = node;
        return true;
    }

    return false;
}

s32 GrandGalaxyMap::findStickTarget(f32 threshold) {
    s32 current = getCamera()->mCurrent;
    WorldMapResultWorld* pWorld = getWorld();
    WorldMapResultNode* pCurrentNode = pWorld->getNode(current);
    f32 best = 0.0f;
    s32 target = -1;
    s32 num = pWorld->getNodeNum();

    for (s32 i = 0; i < num; i++) {
        if (pWorld->isLinkedDirect(i, current) && getWorld()->isPathReachable(i)) {
            TVec3f dir(getWorld()->getNode(i)->mPos);
            dir -= pCurrentNode->mPos;
            MR::normalizeOrZero(&dir);
            f32 dot = dir.x * MR::getPlayerStickX() - dir.z * MR::getPlayerStickY();
            if (dot > threshold && dot > best) {
                target = i;
                best = dot;
            }
        }
    }

    return target;
}

bool GrandGalaxyMap::tryStickMove() {
    s32 target = findStickTarget(0.7f);
    if (target >= 0) {
        getCamera()->mNext = target;
        return true;
    }

    return false;
}

bool GrandGalaxyMap::tryWorldWarp(PartsWarpInfo* pInfo) {
    if (MR::testCorePadTriggerA(0)) {
        if (mParts->mInfo->isPointingBoxBase()) {
            MR::startActionSound(getCamera()->mRocket, "Invalid", -1, -1, -1);
            return false;
        }
    } else if (!MR::testCorePadTriggerUp(0)) {
        return false;
    }

    s32 current = getCamera()->mCurrent;
    return getWorld()->mUnit14->getStarRoadWarp(current, pInfo);
}

void GrandGalaxyMap::faceRocketInstant(const TVec3f& rDir) {
    getCamera()->mRocket->setMoveDirectionInstant(rDir);
}

void GrandGalaxyMap::setRocketSpeed(f32 speed) {
    getCamera()->mRocket->mMoveSpeed = speed;
}

s32 GrandGalaxyMap::tryUsePointMark() {
    if (mOwner->isZoomButtonPointing()) {
        return -1;
    }

    getWorld();
    s32 point = getWorld()->mUnit18->findReachablePoint();
    if (point < 0) {
        return -1;
    }

    getWorld()->mUnit18->tryUseMark(point);
    return point;
}

s32 GrandGalaxyMap::getPathDistance(s32 node) const {
    return mParts->mWorld->mPathFinder->getDistance(node);
}

s32 GrandGalaxyMap::getPath(s32 node, s32 step) const {
    return mParts->mWorld->mPathFinder->getPath(node, step);
}

void GrandGalaxyMap::changeRocketAction(const char* pName) {
    mOwner->changeRocketAction(pName);
}

void GrandGalaxyMap::changeRocketActionBck(const char* pName) {
    mOwner->changeRocketActionBck(pName);
}

LiveActor* GrandGalaxyMap::getMeister() const {
    return mOwner->getMeister();
}

void GrandGalaxyMap::appearMeister() {
    mOwner->appearMeister();
}

void GrandGalaxyMap::setMeisterTalk() {
    mOwner->setMeisterTalk();
}

void GrandGalaxyMap::setMeisterTalkEndWait() {
    mOwner->setMeisterTalkEndWait();
}

s32 GrandGalaxyMap::isMeisterTalkStarting() {
    return mOwner->isMeisterTalkStarting();
}

s32 GrandGalaxyMap::isMeisterTalkEnding() {
    return mOwner->isMeisterTalkEnding();
}

void GrandGalaxyMap::declareAreaCameras() {
    s32 min = 0;
    s32 max = 0;
    getWorld()->mMapData->getLayerRange(&min, &max);
    for (s32 i = min; i <= max; i++) {
        char areaName[0x100];
        snprintf(areaName, sizeof(areaName), "エリア(%02d)", i);
        char cameraName[0x100];
        snprintf(cameraName, sizeof(cameraName), "%s", areaName);
        MR::declareEventCameraById(getCamera()->mCameraId, cameraName);
    }
}

void GrandGalaxyMap::startEventCamera(const char* pName, s32 frames) {
    char cameraName[0x100];
    snprintf(cameraName, sizeof(cameraName), "%s", pName);
    LiveActor* pTarget = getCamera()->mCameraTarget;
    s32 cameraId = getCamera()->mCameraId;
#ifdef GALAXY_NATIVE
    if (PetariWmRouteDiag::enabled()) {
        const auto* data = getWorld()->mMapData; const s32 current = getCamera()->mCurrent;
        const s32 layer = data && data->mNodes && current >= 0 && current < data->mNum ? data->mNodes[current].mLayer : -1;
        PetariWmRouteDiag::request("GrandGalaxyMap::startEventCamera", cameraName, cameraId, frames, pTarget, pTarget ? pTarget->mPosition.x : 0.0f, pTarget ? pTarget->mPosition.y : 0.0f, pTarget ? pTarget->mPosition.z : 0.0f, current, layer);
    }
#endif
    MR::startEventCameraById(cameraId, cameraName, CameraTargetArg(pTarget), frames);
}

void GrandGalaxyMap::startAreaCamera(s32 node, s32 frames) {
    if (node < 0) {
        node = getCamera()->mCurrent;
    }

    char areaName[0x100];
    snprintf(areaName, sizeof(areaName), "エリア(%02d)", getWorld()->mMapData->mNodes[node].mLayer);
    startEventCamera(areaName, frames);
}

void GrandGalaxyMap::activate() {
    start();
}

void GrandGalaxyMap::deactivate() {
    end();
}

void GrandGalaxyMap::killRoutes() {
    getWorld()->mLinks->killAll();
    getWorld()->mUnit18->killAll();
}

MiniPlayerRocket* GrandGalaxyMap::getRocket() {
    return getCamera()->mRocket;
}

// Returns the address of a local copy of the offset (as the target does); callers read it right away.
const TVec3f* GrandGalaxyMap::getNodeOffset(s32 node) const {
    static Vec sNodeOffset = {0.0f, 1000.0f, 0.0f};
#ifdef GALAXY_NATIVE
    // ORIGINAL BUG: the returned local is dead once the call returns; natively keep the copy alive.
    static const TVec3f sNativeNodeOffset(sNodeOffset);
    return &sNativeNodeOffset;
#else
    TVec3f offset(sNodeOffset);
    return &offset;
#endif
}

void GrandGalaxyMap::getNodePosition(TVec3f* pOut, const GrandGalaxyMap* pMap, s32 node) {
    const WorldMapResultNode* pNode = pMap->getWorld()->getNode(node);
    pOut->x = pNode->mPos.x;
    pOut->y = pNode->mPos.y;
    pOut->z = pNode->mPos.z;
    pOut->add(*pMap->getNodeOffset(node));
}

void GrandGalaxyMap::warpRocketToNode(s32 node) {
    WorldMapCameraHolder* pCamera = getCamera();
    MiniPlayerRocket* pRocket = getCamera()->mRocket;
    if (node < 0) {
        node = pCamera->mNext;
    }

    TVec3f pos(0.0f);
    getNodePosition(&pos, this, node);
    MR::setPosition(pRocket, pos);
    MR::zeroVelocity(pRocket);
    MR::setPosition(pCamera->mCameraTarget, pos);
    pCamera->mCurrent = node;
    pCamera->mNext = -1;
    getWorld()->updatePaths(getCamera()->mCurrent);
}

bool GrandGalaxyMap::setupInfo(s32 worldNo) {
    GrandGalaxyMapInfo* pInfo = mParts->mInfo;
    GrandGalaxyMapGalaxyHolder* pGalaxies = mParts->mGalaxies;
    bool hasIcon = false;
    s32 index = 0;
    pInfo->kill();

    for (ScenarioDataIter iter = MR::makeBeginScenarioDataIter(); !iter.isEnd(); iter.goNext()) {
        GalaxyStatusAccessor accessor = iter.makeAccessor();
        if (worldNo != accessor.getWorldNo()) {
            continue;
        }

        const char* pName = accessor.getName();
        GrandGalaxyMapGalaxy* pGalaxy = pGalaxies->find(pName);
        if (accessor.isOpened()) {
            pGalaxy->emitEffect(false);
            bool hasTicoCoin = GameDataFunction::isOnGalaxyFlagTicoCoin(pName);
            s32 starNum = MR::getGalaxyStarTotalNum(pName);
            bool hasGreen = WorldMapFunction::hasIncompleteGreenStar(pName);
            bool isCompletedWithGreen = accessor.isCompletedWithGreen();
            bool isCompletedNoGreen = accessor.isCompletedNoGreen();
            pInfo->setIconCometPanes(index, hasTicoCoin, starNum, hasGreen, isCompletedWithGreen, isCompletedNoGreen);
            pInfo->setIconEventPane(index, pName);
            pInfo->setIconActor(index, pGalaxy);
            hasIcon = true;
        } else if (GameDataFunction::isGalaxyStateNew(pName)) {
            pGalaxy->emitEffect(true);
            pInfo->setIconNewPanes(index);
            pInfo->setIconActor(index, pGalaxy);
            hasIcon = true;
        } else {
            pInfo->setIconDefaultPanes(index);
            pInfo->setIconActor(index, nullptr);
        }

        pInfo->setIconState(index, pName);
        pInfo->setIconIndex(index, GameDataConst::getWorldNo(pName));
        index++;
    }

    pInfo->sortEntries();
    return hasIcon;
}

void GrandGalaxyMap::startNextAppear() {
    s32 current = getCamera()->mCurrent;
    s32 worldNo = getWorld()->mUnit14->getWarpTarget(current);
    if (worldNo > 0 && setupInfo(worldNo)) {
        mParts->mInfo->startNextAppear();
    }
}

void GrandGalaxyMap::startPreviousAppear() {
    s32 current = getCamera()->mCurrent;
    s32 worldNo = getWorld()->mUnit14->getWarpTarget(current);
    if (worldNo > 0 && setupInfo(worldNo)) {
        mParts->mInfo->startPreviousAppear();
    }
}

void GrandGalaxyMap::startAppear() {
    s32 current = getCamera()->mCurrent;
    s32 worldNo = getWorld()->mUnit14->getWarpTarget(current);
    if (worldNo > 0 && setupInfo(worldNo)) {
        mParts->mInfo->startAppear();
    }
}

void GrandGalaxyMap::requestPreviousEnd() {
    mParts->mInfo->requestPreviousEnd();
}

void GrandGalaxyMap::requestNextEnd() {
    mParts->mInfo->requestNextEnd();
}

void GrandGalaxyMap::requestEnd() {
    mParts->mInfo->requestEnd();
}

const char* GrandGalaxyMap::getSelectedIconState() {
    return mParts->mInfo->getSelectedIconState();
}

s32 GrandGalaxyMap::getCurrentWarpTarget() {
    s32 current = getCamera()->mCurrent;
    return getWorld()->mUnit14->getWarpTarget(current);
}

void GrandGalaxyMap::setupWorldName(s32 worldNo) {
    mOwner->setupWorldName(worldNo);
}

void GrandGalaxyMap::showGalaxyInfo(const char* pName) {
    mOwner->showGalaxyInfo(pName);
}

void GrandGalaxyMap::requestAppearWorldName() {
    mOwner->requestAppearWorldName();
}
