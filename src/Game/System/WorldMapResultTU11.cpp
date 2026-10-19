#include "Game/System/WorldMapOwner.hpp"
#include "Game/System/WorldMapResultStateWorldWarp.hpp"
#include "Game/System/WorldMapResultLocal.hpp"
#include "Game/System/WorldMapResultStatePointEvent.hpp"
#include "Game/System/WorldMapResultStateTicoRouteCreate.hpp"
#include "Game/System/GameSequenceFunction.hpp"
#include "Game/System/GameDataFunction.hpp"
#include "Game/System/MiniMeister.hpp"
#include "Game/System/MiniPlayerRocket.hpp"
#include "Game/System/MiniTicoMaster.hpp"
#include "Game/System/MiniPartsActors.hpp"
#include "Game/NPC/TalkMessageFunc.hpp"
#include "Game/Screen/MapGalaxyLayouts.hpp"
#include "Game/LiveActor/ActorCameraInfo.hpp"
#include "Game/Util/ActorCameraUtil.hpp"
#include "Game/Util/ActorMovementUtil.hpp"
#include "Game/Util/TalkUtil.hpp"
#include "Game/Util/EffectUtil.hpp"
#include "Game/Util/ActionSoundUtil.hpp"
#include "Game/Util/ActorAnimUtil.hpp"
#include "Game/Util/EventUtil.hpp"
#include "Game/Util/GamePadUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/StringUtil.hpp"
#include "Game/Camera/CameraTargetArg.hpp"
#include <algorithm>
#include <cstdio>
#include <cmath>

// Meister screen position and the camera offset used by the world-map events (constructed at the end of the sinit).
static TVec2f sMeisterScreenPos(120.0f, 112.0f);
static TVec3f sEventCameraOffset(0.0f, 1150.0f, -500.0f);

// Order of the galaxy markers on screen (left to right).
static bool compareMarkerX(const GalaxyWorldMapInfo* pA, const GalaxyWorldMapInfo* pB) {
    return pA->mPosition.x > pB->mPosition.x;
}

WorldMapResultMapData* WorldMapOwner::getMapData() const {
    return mContext->mWorld->mMapData;
}

GalaxyWorldMapInfoHolder* WorldMapOwner::getInfoHolder() const {
    return mContext->mWorld->mWorldData;
}

WorldMapPointMarks* WorldMapOwner::getPointMarks() const {
    return mContext->mWorld->mUnit18;
}

PointPartsHolder* WorldMapOwner::getPointParts() const {
    return mContext->mWorld->mUnit14;
}

MapGalaxyZoomButton* WorldMapOwner::getZoomButton() const {
    return mContext->mLayouts->mZoomButton;
}

MapGalaxyCounters* WorldMapOwner::getCounters() const {
    return mContext->mLayouts->mCounters;
}

bool WorldMapOwner::isDecideTrigger() {
    if (MR::testCorePadTriggerUp(0)) {
        return true;
    }

    return MR::testCorePadTriggerA(0);
}

bool WorldMapOwner::tryDecide(s32 node) {
    if (MR::testCorePadTriggerA(0)) {
        s32 current = getCamera()->mCurrent;
        if (node >= 0 && node != current) {
            return false;
        }

        if (getZoomButton()->isPointing()) {
            return getZoomButton()->tryDecide();
        }
    }

    return true;
}

WorldMapResultWorld* WorldMapOwner::getWorld() const {
    return mContext->mWorld;
}

GrandGalaxyMap* WorldMapOwner::getMainLayout() const {
    return static_cast<GrandGalaxyMap*>(mMainLayout);
}

bool WorldMapOwner::isPressedA() {
    return MR::testCorePadTriggerA(0);
}

bool WorldMapOwner::isStickTilted() {
    f32 x = MR::getPlayerStickX();
    f32 y = MR::getPlayerStickY();
    return x * x + y * y > 0.7f * 0.7f;
}

bool WorldMapOwner::tryStartStarCheckEvent(WorldMapResultStatePointEvent* pState, s32 node) {
    s32 current;
    PointPartsHolder* pParts = getPointParts();
    current = getCamera()->mCurrent;

    if (!isDecideTrigger()) {
        return false;
    }

    if (!pParts->isStarCheckPointPending(current)) {
        return false;
    }

    if (tryDecide(node)) {
        s32 field = pParts->getActorField94(current);
        if (GameDataFunction::getCurrentWorldId() > 2) {
            pState->setIndex(1, field);
        } else {
            pState->setIndex(0, field);
        }

        return true;
    }

    return false;
}

bool WorldMapOwner::tryStartPointEvent2(WorldMapResultStatePointEvent* pState) {
    s32 current;
    PointPartsHolder* pParts = getPointParts();
    current = getCamera()->mCurrent;

    if (mContext->mWorld->callU14_620(current)) {
        pState->setIndex(2, pParts->getActorField94(current));
        return true;
    }

    return false;
}

bool WorldMapOwner::isHomeNotOpened() {
    if (hasWorldDataAtCurrent()) {
        WorldMapCameraHolder* pCamera = getCamera();
        if (!getInfoHolder()->isOpened(pCamera->mCurrent)) {
            return true;
        }
    }

    return false;
}

bool WorldMapOwner::tryEnterHome(s32 node) {
    if (!isDecideTrigger()) {
        return false;
    }

    if (hasWorldDataAtCurrent()) {
        return false;
    }

    WorldMapCameraHolder* pCamera = getCamera();
    if (getPointParts()->isActorAlive(pCamera->mCurrent)) {
        return false;
    }

    return tryDecide(node);
}

bool WorldMapOwner::tryEnterGalaxy(s32 node) {
    if (!hasWorldDataAtCurrent()) {
        return false;
    }

    if (isDecideTrigger()) {
        return tryDecide(node);
    }

    return false;
}

bool WorldMapOwner::tryOpenKoopaDemoFlag() {
    if (GameDataFunction::isOnGameEventFlag("クッパ最終戦直前デモ")) {
        return false;
    }

    s32 num = mContext->mWorld->getNodeNum();
    for (s32 i = 0; i < num; i++) {
        GalaxyWorldMapInfo* pInfo = getInfoHolder()->find(i);
        if (pInfo != nullptr && pInfo->mKind == 5) {
            if (mContext->mWorld->getPathDistance(i) == 1) {
                GameDataFunction::onGameEventFlag("クッパ最終戦直前デモ");
                return true;
            }
        }
    }

    return false;
}

bool WorldMapOwner::tryWarpToNode(s32 node) {
    WorldMapCameraHolder* pCamera = getCamera();
    if (node >= 0 && MR::testCorePadTriggerA(0) && node != pCamera->mCurrent && mContext->mWorld->isPathReachable(node)) {
        getPointMarks()->emitGoEffect(node);
        pCamera->mNext = node;
        return true;
    }

    return false;
}

s32 WorldMapOwner::findStickTarget(f32 threshold) {
    if (!isStickTilted()) {
        return -1;
    }

    s32 current = getCamera()->mCurrent;
    WorldMapResultWorld* pWorld = mContext->mWorld;
    WorldMapResultNode* pCurrentNode = pWorld->getNode(current);
    f32 best = 0.0f;
    s32 result = -1;
    s32 num = pWorld->getNodeNum();
    for (s32 i = 0; i < num; i++) {
        if (pWorld->isLinkedDirect(i, current) && pWorld->isPathReachable(i)) {
            TVec3f dir = mContext->mWorld->getNode(i)->mPos - pCurrentNode->mPos;
            MR::normalizeOrZero(&dir);
            f32 value = dir.x * MR::getPlayerStickX() - dir.z * MR::getPlayerStickY();
            if (value > threshold && value > best) {
                result = i;
                best = value;
            }
        }
    }

    return result;
}

bool WorldMapOwner::tryStickMove() {
    s32 target = findStickTarget(0.7f);
    if (target >= 0) {
        getCamera()->mNext = target;
        return true;
    }

    return false;
}

bool WorldMapOwner::tryWarpByA(s32 node) {
    if (!MR::testCorePadTriggerA(0)) {
        return false;
    }

    s32 target = mContext->mWorld->callU14_2B0(getCamera()->mCurrent);
    if (target >= 0) {
        getCamera()->mNext = target;
        return tryDecide(node);
    }

    return false;
}

bool WorldMapOwner::tryStickWarp() {
    if (!getCamera()->mHasNext) {
        return false;
    }

    s32 target = mContext->mWorld->callU14_2B0(getCamera()->mCurrent);
    if (target >= 0) {
        getCamera()->mNext = target;
        return true;
    }

    return false;
}

bool WorldMapOwner::tryApplyNextNode() {
    if (getCamera()->mHasNext) {
        return getCamera()->tryApplyNext();
    }

    return false;
}

bool WorldMapOwner::tryStartTicoRoute(WorldMapResultStateTicoRouteCreate* pState, s32 node) {
    if (!isDecideTrigger()) {
        return false;
    }

    WorldMapCameraHolder* pCamera = getCamera();
    MiniTicoMaster* pCreator = static_cast< MiniTicoMaster* >(getPointParts()->getTicoCreator(pCamera->mCurrent));
    if (pCreator == nullptr) {
        return false;
    }

    if (tryDecide(node) && !pCreator->isFull() && tryDecide(-1)) {
        pState->mTicoCreator = pCreator;
        return true;
    }

    return false;
}

bool WorldMapOwner::tryWorldWarpFromStick(WorldMapResultStateWorldWarp* pInfo) {
    s32 stick = findStickTarget(0.7f);
    if (stick >= 0) {
        return getPointParts()->getWorldWarp(stick, pInfo);
    }

    return false;
}

bool WorldMapOwner::tryWorldWarpFromNext(WorldMapResultStateWorldWarp* pInfo) {
    WorldMapCameraHolder* pCamera = getCamera();
    if (pCamera->mHasNext) {
        return getPointParts()->getWorldWarp(pCamera->mNext, pInfo);
    }

    return false;
}

// NONMATCHING: MWCC folds `id < 2 || id > 6` into the range check (addi/cmplwi); the target keeps two cmpwi (same control flow otherwise)
bool WorldMapOwner::isAllNeededForGrandGalaxy() {
    s32 worldId = mContext->mWorld->mWorldId;
    if (worldId < 2) {
        return false;
    }

    if (worldId > 6) {
        return false;
    }

    if (MR::hasGrandStar(worldId)) {
        return false;
    }

    if (!getPointParts()->isAnyStarCheckPointPending()) {
        return false;
    }

    return getInfoHolder()->isAllCompleted();
}

// NONMATCHING: the target normalizes startOpenSoundAll() (addic/subfe) into a saved register before updateAll(); ours defers it until after the or (FP order/grouping checked: no FP instructions).
bool WorldMapOwner::updateParts() {
    s32 raw = getInfoHolder()->startOpenSoundAll();
    s32 opened = raw != 0;
    return (opened | getPointParts()->updateAll()) != 0;
}

s32 WorldMapOwner::updatePointing() {
    WorldMapResultWorld* pWorld = mContext->mWorld;
    PointPartsHolder* pParts = getPointParts();
    GalaxyWorldMapInfoHolder* pInfos = getInfoHolder();
    WorldMapPointMarks* pMarks = getPointMarks();
    s32 point = pInfos->getPointingPoint();
    if (point < 0) {
        point = pMarks->findReachablePoint();
    }

    if (point >= 0 && pInfos->isOpened(point)) {
        showGalaxyInfo(pInfos->getGalaxyName(point));
    }

    if (point < 0) {
        point = pParts->findActivePoint();
    }

    if (point < 0) {
        return -1;
    }

    bool usedMark = pMarks->tryUseMark(point);
    if (pParts->tryTico72C0(point)) {
        getCounters()->requestAppearStarPieces();
    } else if (pParts->tryStarCheck6000(point)) {
        getCounters()->requestAppearStars();
    } else if ((!pWorld->isPathReachable(point) || !pParts->tryPipe3420(point)) && !pInfos->updatePointing(point) && !usedMark) {
        return -1;
    }

    return point;
}

void WorldMapOwner::updateCurrentPaths() {
    mContext->mWorld->updatePaths(getCamera()->mCurrent);
}

void WorldMapOwner::tryTalkPoint(s32 node) {
    PointPartsHolder* pParts = getPointParts();
    if (pParts->findActivePoint() == -1) {
        if (node < 0) {
            node = getCamera()->mCurrent;
        }

        if (pParts->tryTalkTico(node) && !isGalaxyInfoAppeared()) {
            getCounters()->requestAppearStarPieces();
        }

        if (pParts->tryTalkStarCheckPoint(node) && !isGalaxyInfoAppeared()) {
            getCounters()->requestAppearStars();
        }
    }
}

// NONMATCHING: instruction scheduling only (the target hoists the mContext load above the stw r0 of the prologue) (FP order/grouping checked: no FP instructions).
void WorldMapOwner::prepareWorld() {
    WorldMapOwnerContext* pContext = mContext;
    pContext->mWorld->prepare();
    if (mContext->mModel != nullptr) {
        mContext->mModel->appearAll();
    }
}

// NONMATCHING: instruction scheduling only (the target hoists the mContext load above the stw r0 of the prologue) (FP order/grouping checked: no FP instructions).
void WorldMapOwner::finishWorld() {
    WorldMapOwnerContext* pContext = mContext;
    pContext->mWorld->finish();
    if (mContext->mModel != nullptr) {
        mContext->mModel->killAll();
    }
}

const char* WorldMapOwner::getCurrentGalaxyName() {
    WorldMapCameraHolder* pCamera = getCamera();
    return getInfoHolder()->getGalaxyName(pCamera->mCurrent);
}

bool WorldMapOwner::hasWorldDataAtCurrent() {
    return mContext->mWorld->hasWorldData(getCamera()->mCurrent);
}

bool WorldMapOwner::isWorldDataType3AtCurrent() {
    return mContext->mWorld->isWorldDataType3(getCamera()->mCurrent);
}

bool WorldMapOwner::isWorldDataType4AtCurrent() {
    return mContext->mWorld->isWorldDataType4(getCamera()->mCurrent);
}

bool WorldMapOwner::isWorldDataType5AtCurrent() {
    return mContext->mWorld->isWorldDataType5(getCamera()->mCurrent);
}

// NONMATCHING: register allocation only (galaxy name / node count / path table swap saved registers) (FP order/grouping checked: no FP instructions).
void WorldMapOwner::enterCurrentGalaxy() {
    mContext->mWorld->disappear();
    s32 node = getCamera()->mCurrent;
    GameSequenceFunction::setWorldMapCurrentNode(node);
    const char* pName = getInfoHolder()->getGalaxyName(node);
    mContext->mWorld->refreshRouteComet();
    if (pName == nullptr) {
        WorldMapResultWorld* pWorld = mContext->mWorld;
        WorldMapResultPathFinder* pPaths = pWorld->mPathFinder;
        PointPartsHolder* pParts = getPointParts();
        s32 num = pWorld->getNodeNum();
        for (s32 i = 0; i < num; i++) {
            if (pWorld->isPathReachable(i)) {
                MiniWorldWarpPoint* pWarp = static_cast< MiniWorldWarpPoint* >(pParts->getWorldWarpActor(i));
                if (pWarp != nullptr) {
                    if (pWarp->mIsForward) {
                        if (pPaths->getDistance(i) <= 2) {
                            GameSequenceFunction::setWorldMapFlag22C();
                        }
                    } else {
                        if (pPaths->getDistance(i) <= 1) {
                            GameSequenceFunction::setWorldMapFlag22D();
                        }
                    }
                }
            }
        }

        if (GameSequenceFunction::isTicoFatStarPieceActive()) {
            MR::setWipeCircleCenterPos(getRocket()->mPosition);
            MR::closeSystemWipeCircleWithCaptureScreen(0x5A);
        } else {
            MR::closeSystemWipeFadeWithCaptureScreen(0x3C);
        }

        GameSequenceFunction::requestChangeStageMarioFaceShip(1, -1);
        return;
    }

    for (ScenarioDataIter it = MR::makeBeginScenarioDataIter(); !it.isEnd(); it.goNext()) {
        GalaxyStatusAccessor accessor = it.makeAccessor();
        if (MR::isEqualStringCase(pName, accessor.getName())) {
            GameSequenceFunction::requestChangeScenarioSelect(pName);
            return;
        }
    }

    MR::startSystemWipeCircleWithCaptureScreen(-1);
    GameSequenceFunction::requestChangeStageInStage(pName);
}

GalaxyWorldMapInfo* WorldMapOwner::findGalaxyByName(const char* pName) {
    return getInfoHolder()->findByName(pName);
}

bool WorldMapOwner::tryOpenCurrentGalaxy() {
    WorldMapCameraHolder* pCamera = getCamera();
    if (getInfoHolder()->tryOpen(pCamera->mCurrent)) {
        mContext->mWorld->disappear();
        return true;
    }

    return false;
}

void WorldMapOwner::showCurrentNamePlate() {
    WorldMapCameraHolder* pCamera = getCamera();
    getInfoHolder()->showNamePlate(pCamera->mCurrent);
}

void WorldMapOwner::startAllPointingAnim() {
    getInfoHolder()->startAllPointingAnim();
}

bool WorldMapOwner::isCurrentGalaxyInfoShown() {
    WorldMapCameraHolder* pCamera = getCamera();
    const char* pName = getInfoHolder()->getGalaxyName(pCamera->mCurrent);
    const char* pShown = getGalaxyInfoName();
    if (pName != nullptr && pShown != nullptr && MR::isEqualString(pName, pShown)) {
        return true;
    }

    return false;
}

bool WorldMapOwner::tryShowCurrentGalaxyInfo() {
    WorldMapCameraHolder* pCamera = getCamera();
    const char* pName = getInfoHolder()->getGalaxyName(pCamera->mCurrent);
    const char* pShown = getGalaxyInfoName();
    if (pName != nullptr && (pShown == nullptr || MR::isEqualString(pName, pShown))) {
        showGalaxyInfo(pName);
        return true;
    }

    return false;
}

void WorldMapOwner::setupWorldName(s32 world) {
    char buffer[0x100];
    if (world < 0) {
        snprintf(buffer, 0x100, MR::getCurrentStageName());
    } else {
        snprintf(buffer, 0x100, "WorldMap%02dGalaxy", world);
    }

    MapGalaxyWorld* pLayout = mContext->mLayouts->mWorldLayout;
    pLayout->setWorldName(MR::getGalaxyNameOnCurrentLanguage(buffer));
}

void WorldMapOwner::requestAppearWorldName() {
    mContext->mLayouts->mWorldLayout->requestAppear();
}

void WorldMapOwner::requestAppearZoomButton() {
    getZoomButton()->requestAppear();
}

bool WorldMapOwner::isZoomButtonAppeared() {
    return getZoomButton()->isAppeared();
}

void WorldMapOwner::requestAppearCoinsAndLives() {
    getCounters()->requestAppearCoinsAndLives();
}

bool WorldMapOwner::tryDecideZoomButton() {
    return getZoomButton()->tryDecide();
}

bool WorldMapOwner::isZoomButtonPointing() {
    return getZoomButton()->isPointing();
}

void WorldMapOwner::changeRocketAction(const char* pName) {
    getCamera()->mRocket->changeAction(pName);
}

void WorldMapOwner::changeRocketActionBck(const char* pName) {
    getCamera()->mRocket->changeAction(pName);
    MR::startBckNoInterpole(getRocket(), pName);
}

const TVec3f* WorldMapOwner::getRocketPos() {
    return &getRocket()->mPosition;
}

void WorldMapOwner::faceRocket(const TVec3f* pDir) {
    getCamera()->mRocket->setMoveDirection(*pDir);
}

void WorldMapOwner::faceRocketInstant(const TVec3f* pDir) {
    getCamera()->mRocket->setMoveDirectionInstant(*pDir);
}

void WorldMapOwner::setRocketPos(const TVec3f& rPos, const TVec3f& rDir) {
    MiniPlayerRocket* pRocket = getCamera()->mRocket;
    pRocket->setMoveDirection(rDir);
    pRocket->mPosition.set(rPos);
    pRocket->resetAction();
}

void WorldMapOwner::setRocketPosFromPrev(const TVec3f& rPos) {
    MiniPlayerRocket* pRocket = getCamera()->mRocket;
    pRocket->setMoveDirection(rPos - pRocket->mPosition);
    pRocket->mPosition.set(rPos);
    pRocket->resetAction();
}

void WorldMapOwner::setRocketSpeed(f32 speed) {
    getCamera()->mRocket->mMoveSpeed = speed;
}

void WorldMapOwner::startRocketBounce() {
    getCamera()->mRocket->startBounce();
}

s32 WorldMapOwner::getCurrentNode() {
    return getCamera()->mCurrent;
}

void WorldMapOwner::resetCameraMove() {
    WorldMapCameraHolder* pCamera = getCamera();
    MiniPlayerRocket* pRocket = pCamera->mRocket;
    pCamera->mPathFrom = pCamera->mCurrent;
    pCamera->mPathStep = 0;
    pCamera->mMoveFrame = 0;
    pCamera->mTargetA.set(pRocket->mPosition);
    pCamera->mTargetB.set(pRocket->mPosition);
    pCamera->mVelocity.set(pRocket->mVelocity);
    pCamera->mAccel = 100.0f;
    pCamera->mMaxSpeed = 400.0f;
    pCamera->mFriction = 0.85f;
    MR::startAction(pRocket, "Wait");
}

void WorldMapOwner::startRocketMoveSound() {
    MR::startActionSound(getCamera()->mRocket, "Move", -1, -1, -1);
}

// NONMATCHING: register/stack or instruction/branch lowering (FP operand order/grouping checked).
bool WorldMapOwner::updateCameraMove(s32 flag) {
    WorldMapCameraHolder* pCamera = getCamera();
    WorldMapResultWorld* pWorld = mContext->mWorld;
    MiniPlayerRocket* pRocket = pCamera->mRocket;
    pRocket->resetAction();
    bool arrived = false;
    if (pCamera->mMoveFrame == 0) {
        s32 next = mContext->mWorld->getPathNode(pCamera->mNext, pCamera->mPathStep);
        if (pWorld->isLinked14(pCamera->mCurrent, next)) {
            pCamera->setNext(next);
            return true;
        }

        if (pWorld->callU14_8A0(next)) {
            pCamera->setNext(next);
            pCamera->mCurrent = pCamera->mPathFrom;
            return true;
        }

        if (next != pCamera->mNext && pWorld->callU14_860(next)) {
            pCamera->setNext(next);
        } else if (flag) {
            pCamera->mNext = next;
        }

        MR::startActionSound(getCamera()->mRocket, "Move", -1, -1, -1);
        pCamera->mCurrent = pCamera->mPathFrom;
        getNodePosition(&pCamera->mStartOffset, this, pCamera->mPathFrom);
        pCamera->mPathFrom = next;
        pCamera->mPathStep++;
        if (pCamera->mPathStep >= mContext->mWorld->getPathDistance(pCamera->mNext)) {
            arrived = true;
        }

        if (!arrived && pWorld->callU14_860(next)) {
            pCamera->setNext(next);
        }

        TVec3f from(pWorld->getNode(pCamera->mCurrent)->mPos);
        TVec3f to(pWorld->getNode(pCamera->mPathFrom)->mPos);
        f32 toX = to.x;
        f32 fromX = from.x;
        if (__fabsf(fromX - toX) > 1000.0f) {
            if (fromX < toX) {
                pCamera->mMovingLeft = false;
            } else {
                pCamera->mMovingLeft = true;
            }
        }

        startNodeCamera(pCamera->mPathFrom, 0x5A);
        if (arrived) {
            pCamera->mTargetA.set(pRocket->mPosition);
            pCamera->mTargetB.set(pRocket->mPosition);
            pCamera->mVelocity.set(pRocket->mVelocity);
            MR::zeroVelocity(pRocket);
        }
    }

    if (pCamera->mPathStep >= mContext->mWorld->getPathDistance(pCamera->mNext)) {
        arrived = true;
    }

    TVec3f goal(0.0f);
    getNodePosition(&goal, this, pCamera->mPathFrom);
    if (arrived) {
        TVec3f accel(0.0f);
        MR::calcVelocityMoveToTarget3D(&accel, pRocket, goal, pCamera->mAccel);
        pCamera->mVelocity.add(accel);
        pCamera->mVelocity.scale(pCamera->mFriction);
        pCamera->mTargetA.add(pCamera->mVelocity);
        s32 frame = 0x1A;
        f32 rate = MR::getEaseOutValue(MR::clamp(MR::normalize((f32)pCamera->mMoveFrame, 0.0f, (f32)frame), 0.0f, 1.0f), 0.0f, 1.0f, 1.0f);
        TVec3f c = goal * rate + pCamera->mTargetB * (1.0f - rate);
        if (PSVECMag(&pCamera->mVelocity) > 100.0f) {
            pRocket->setMoveDirection(pCamera->mVelocity);
        } else {
            pRocket->setMoveDirection(TVec3f(0.0f));
        }

        pRocket->mPosition = c * rate + pCamera->mTargetA * (1.0f - rate);
    } else {
        MR::addVelocityMoveToTarget3D(pRocket, goal, pCamera->mAccel);
        MR::attenuateVelocity(pRocket, pCamera->mFriction);
        MR::restrictVelocity(pRocket, pCamera->mMaxSpeed);
        pRocket->setMoveDirection(pRocket->mVelocity);
    }

    s32 frame2 = 0x14;
    f32 rate2 = MR::getEaseOutValue(MR::clamp(MR::normalize((f32)pCamera->mMoveFrame, 0.0f, (f32)frame2), 0.0f, 1.0f), 0.0f, 1.0f, 1.0f);
    TVec3f i = goal * rate2 + pCamera->mStartOffset * (1.0f - rate2);
    MR::setPosition(pCamera->mCameraTarget, i);
    pCamera->mMoveFrame++;
    if ((arrived && pCamera->mMoveFrame > 0x1A) || (MR::isNear(pRocket, goal, 10.0f) && MR::isVelocityGreaterEqual(pRocket, 1.0f) == 0) ||
        (!arrived && MR::isNear(pRocket, goal, 1000.0f))) {
        if (!arrived) {
            pCamera->mMoveFrame = 0;
            return false;
        }

        MR::startActionSound(pRocket, "MoveClick", -1, -1, -1);
        warpRocketToNode(-1);
        return true;
    }

    return false;
}

void WorldMapOwner::hideRocket() {
    MR::forceDeleteEffectAll(getCamera()->mRocket);
    MR::hideModel(getCamera()->mRocket);
}

void WorldMapOwner::showRocket() {
    MR::showModel(getCamera()->mRocket);
}

LiveActor* WorldMapOwner::getMeister() const {
    return mContext->mMeister;
}

void WorldMapOwner::appearMeister() {
    mContext->mMeister->appear();
}

void WorldMapOwner::appearMeisterAtScreenPos(const TVec2f& rPos) {
    mContext->mMeister->appearAtScreenPos(rPos);
}

void WorldMapOwner::setMeisterTalk() {
    mContext->mMeister->setNerveTalk();
}

void WorldMapOwner::setMeisterTalkEndWait() {
    mContext->mMeister->setNerveTalkEndWait();
}

s32 WorldMapOwner::isMeisterTalkStarting() {
    return mContext->mMeister->isTalkStarting();
}

s32 WorldMapOwner::isMeisterTalkEnding() {
    return mContext->mMeister->isTalkEnding();
}

TalkMessageCtrl* WorldMapOwner::createMeisterTalk(const char* pName) {
    MiniMeister* pMeister = mContext->mMeister;
    TalkMessageCtrl* pCtrl = MR::createTalkCtrlDirect(pMeister, pName, TVec3f(0.0f, 0.0f, 0.0f), nullptr);
    MR::onRootNodeAutomatic(pCtrl);
    MR::registerAnimeFunc(pCtrl, TalkMessageFuncM< MiniMeister*, bool (MiniMeister::*)(u32) >(pMeister, &MiniMeister::startTalkAction));
    return pCtrl;
}

void WorldMapOwner::initCameraSetup(const char* pSuffix) {
    char buffer[0x100];
    snprintf(buffer, 0x100, "World%02d%s", mContext->mWorldId, pSuffix);
    MR::initMultiActorCameraNoInit(this, getCamera()->mCameraInfo, buffer);
}

void WorldMapOwner::initCameraNodeNames() {
    s32 first = 0;
    s32 last = 0;
    getMapData()->getLayerRange(&first, &last);
    for (s32 i = first; i <= last; i++) {
        char name[0x100];
        snprintf(name, 0x100, "エリア(%02d)[左]", i);
        declareCamera(name);
        snprintf(name, 0x100, "エリア(%02d)[右]", i);
        declareCamera(name);
    }
}

void WorldMapOwner::startCameraTargetOther(const char* pSuffix, s32 frames) {
    char buffer[0x100];
    snprintf(buffer, 0x100, "World%02d%s", mContext->mWorldId, pSuffix);
    LiveActor* pTarget = getCamera()->mCameraTarget;
    ActorCameraInfo* pInfo = getCamera()->mCameraInfo;
    MR::startMultiActorCameraTargetOther(this, pInfo, buffer, CameraTargetArg(pTarget), frames);
}

void WorldMapOwner::startNodeCamera(s32 node, s32 step) {
    if (node < 0) {
        node = getCamera()->mCurrent;
    }

    s32 layer = getMapData()->mNodes[node].mLayer;
    char buffer[0x100];
    if (getCamera()->mMovingLeft) {
        snprintf(buffer, 0x100, "エリア(%02d)[左]", layer);
    } else {
        snprintf(buffer, 0x100, "エリア(%02d)[右]", layer);
    }

    startCamera(buffer, step);
}

void WorldMapOwner::declareCamera(const char* pName) {
    char buffer[0x100];
    snprintf(buffer, 0x100, "%s", pName);
    MR::declareEventCameraById(getCamera()->mCameraId, buffer);
}

void WorldMapOwner::startCamera(const char* pName, s32 frames) {
    char buffer[0x100];
    snprintf(buffer, 0x100, "%s", pName);
    LiveActor* pTarget = getCamera()->mCameraTarget;
    s32 id = getCamera()->mCameraId;
    MR::startEventCameraById(id, buffer, CameraTargetArg(pTarget), frames);
}

void WorldMapOwner::startCameraAtCurrent(const char* pName, s32 frames) {
    char buffer[0x100];
    snprintf(buffer, 0x100, "%s", pName);
    s32 node = getCamera()->mCurrent;
    LiveActor* pTarget = getInfoHolder()->find(node);
    if (pTarget == nullptr) {
        pTarget = getPointParts()->getActor(node);
        if (pTarget == nullptr) {
            pTarget = getCamera()->mCameraTarget;
        }
    }

    s32 id = getCamera()->mCameraId;
    MR::startEventCameraById(id, buffer, CameraTargetArg(pTarget), frames);
}

void WorldMapOwner::endCamera(const char* pName, bool resetView, s32 frames) {
    char buffer[0x100];
    snprintf(buffer, 0x100, "%s", pName);
    MR::endEventCameraById(getCamera()->mCameraId, buffer, resetView, frames);
}

void WorldMapOwner::disappearWorld() {
    mContext->mWorld->disappear();
}

void WorldMapOwner::setupRouteOpen(WorldMapResultRoute* pRoute) {
    mContext->mWorld->refreshRouteComet();
    s32 node = GameSequenceFunction::getWorldMapCurrentNode();
    getPointParts()->callStarCheckPoint(node);
    WorldMapResultWorld* pWorld = mContext->mWorld;
    MiniRouteLineList* pLinks = pWorld->mLinks;
    pLinks->openFrom(GameSequenceFunction::getWorldMapCurrentNode());
    mContext->mWorld->setupRoute(pRoute);
    mContext->mWorld->disappear();
}

void WorldMapOwner::startRouteCamera(WorldMapResultRoute* pRoute, const char* pCameraName) {
    WorldMapResultBox box;
    box.extendByRoute(mContext->mWorld, pRoute);
    TVec3f center;
    TVec3f cameraPos;
    box.getCenter(&center);
    box.getCameraPos(&cameraPos);
    MR::setProgrammableCameraParam(pCameraName, center, cameraPos, TVec3f(0.0f, 1.0f, 0.0f), true);
    MR::startGlobalEventCameraNoTarget(pCameraName, -1);
    MR::setProgrammableCameraParamFovy(pCameraName, 26.0f);
}

bool WorldMapOwner::updateRoute(WorldMapResultRoute* pRoute, s32 step, f32 rate) {
    return mContext->mWorld->updateRouteStep(pRoute, step, rate);
}

bool WorldMapOwner::isRouteNotHeld(WorldMapResultRoute* pRoute) {
    return mContext->mWorld->isRouteNotHeld(pRoute);
}

void WorldMapOwner::collectInfos(GalaxyWorldMapInfo** pOut) {
    GalaxyWorldMapInfoHolder* pInfos = getInfoHolder();
    s32 num = mContext->mWorld->getNodeNum();
    s32 count = 0;
    s32 index = 0;
    for (s32 i = 0; i < 7; i++) {
        pOut[i] = nullptr;
    }

    for (s32 i = 0; i < num; i++) {
        pOut[index] = pInfos->find(i);
        if (pOut[index] != nullptr) {
            count++;
            index++;
        }
    }

    std::sort(pOut, pOut + count, compareMarkerX);
}
