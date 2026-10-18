#pragma once

// The object the world-map sequence (WorldMapSequence) and its states operate on (SMG2-only).
// Its members are spread over the WorldMapModel unit (0x804E8300..) and WorldMapResultTU11 (0x804F5F90..);
// the states call them with `mOwner`/`mHost`.

#include "Game/LiveActor/LiveActor.hpp"
#include "Game/NameObj/NameObj.hpp"
#include "Game/System/GalaxyWorldMapInfo.hpp"
#include "Game/System/WorldMapCameraHolder.hpp"
#include "Game/System/WorldMapPointMarks.hpp"
#include "Game/System/WorldMapPointParts.hpp"
#include "Game/System/WorldMapModel.hpp"

class MapGalaxyCounters;
class MapGalaxyZoomButton;
class MiniMeister;
class WorldMapModel;
class MiniPlayerRocket;
class WorldMapResultMapData;
class WorldMapResultRoute;
class WorldMapResultStatePointEvent;
class TalkMessageCtrl;
class MiniTicoMaster;
class GalaxyWorldMapInfo;

/// What WorldMapOwner::mContext points to (the per-world parts the owner hands out).
struct WorldMapOwnerContext {
    /* 0x00 */ s32 mWorldId;
    /* 0x04 */ WorldMapResultWorld* mWorld;
    /* 0x08 */ WorldMapCameraHolder* mCamera;
    /* 0x0C */ MiniMeister* mMeister;
    /* 0x10 */ u8 _10[4];
    /* 0x14 */ WorldMapModel* mModel;
    /* 0x18 */ WorldMapLayouts* mLayouts;
};

/// State that lets the player create a Tico route (WorldMapSequence::mStateTicoRoute); only its creator slot is used here.
// PLACEHOLDER(core): partial layout of the Tico-route state; resolve when that state class (fn_804FC9B0 ctor, 0x24 bytes) is written
struct WorldMapStateTicoRoute {
    /* 0x00 */ u8 _0[0x20];
    /* 0x20 */ MiniTicoMaster* mTicoCreator;
};

// PLACEHOLDER(core): `WorldMapOwner` is a descriptive name (WorldMapSequence::mOwner); resolve when the real class of the world-map owner is known
class WorldMapOwner {
public:
    // ---- WorldMapModel unit (0x804E8300..) ----
    WorldMapCameraHolder* getCamera() const;
    MiniPlayerRocket* getRocket() const;
    static void getNodePosition(TVec3f* pOut, WorldMapOwner* pOwner, s32 node);
    void warpRocketToNode(s32 node);
    bool isGalaxyInfoAppeared() const;
    const char* getGalaxyInfoName() const;
    void showGalaxyInfo(const char* pName);

    // ---- WorldMapResultTU11 (0x804F5F90..) ----
    WorldMapResultMapData* getMapData() const NO_INLINE;
    GalaxyWorldMapInfoHolder* getInfoHolder() const NO_INLINE;
    WorldMapPointMarks* getPointMarks() const NO_INLINE;
    PointPartsHolder* getPointParts() const NO_INLINE;
    MapGalaxyZoomButton* getZoomButton() const NO_INLINE;
    MapGalaxyCounters* getCounters() const NO_INLINE;
    static bool isDecideTrigger() NO_INLINE;
    bool tryDecide(s32 node) NO_INLINE;
    WorldMapResultWorld* getWorld() const NO_INLINE;
    // PLACEHOLDER(core): opaque layout object of the unsplit WorldMap* code (0x804DF2B0..0x804E31D0); resolve when that class is written
    void* getMainLayout() const NO_INLINE;
    static bool isPressedA() NO_INLINE;
    static bool isStickTilted() NO_INLINE;
    bool tryStartStarCheckEvent(WorldMapResultStatePointEvent* pState, s32 node) NO_INLINE;
    bool tryStartPointEvent2(WorldMapResultStatePointEvent* pState) NO_INLINE;
    bool isHomeNotOpened() NO_INLINE;
    bool tryEnterHome(s32 node) NO_INLINE;
    bool tryEnterGalaxy(s32 node) NO_INLINE;
    bool tryOpenKoopaDemoFlag() NO_INLINE;
    bool tryWarpToNode(s32 node) NO_INLINE;
    s32 findStickTarget(f32 threshold) NO_INLINE;
    bool tryStickMove() NO_INLINE;
    bool tryWarpByA(s32 node) NO_INLINE;
    bool tryStickWarp() NO_INLINE;
    bool tryApplyNextNode() NO_INLINE;
    bool tryStartTicoRoute(WorldMapStateTicoRoute* pState, s32 node) NO_INLINE;
    bool tryWorldWarpFromStick(PartsWarpInfo* pInfo) NO_INLINE;
    bool tryWorldWarpFromNext(PartsWarpInfo* pInfo) NO_INLINE;
    bool isAllNeededForGrandGalaxy() NO_INLINE;
    bool updateParts() NO_INLINE;
    s32 updatePointing() NO_INLINE;
    void updateCurrentPaths() NO_INLINE;
    void tryTalkPoint(s32 node) NO_INLINE;
    void prepareWorld() NO_INLINE;
    void finishWorld() NO_INLINE;
    const char* getCurrentGalaxyName() NO_INLINE;
    bool hasWorldDataAtCurrent() NO_INLINE;
    bool isWorldDataType3AtCurrent() NO_INLINE;
    bool isWorldDataType4AtCurrent() NO_INLINE;
    bool isWorldDataType5AtCurrent() NO_INLINE;
    void enterCurrentGalaxy();
    GalaxyWorldMapInfo* findGalaxyByName(const char* pName) NO_INLINE;
    bool tryOpenCurrentGalaxy() NO_INLINE;
    void showCurrentNamePlate() NO_INLINE;
    void startAllPointingAnim() NO_INLINE;
    bool isCurrentGalaxyInfoShown() NO_INLINE;
    bool tryShowCurrentGalaxyInfo() NO_INLINE;
    void setupWorldName(s32 world) NO_INLINE;
    void requestAppearWorldName() NO_INLINE;
    void requestAppearZoomButton() NO_INLINE;
    bool isZoomButtonAppeared() NO_INLINE;
    void requestAppearCoinsAndLives() NO_INLINE;
    void tryDecideZoomButton() NO_INLINE;
    bool isZoomButtonPointing() NO_INLINE;
    void changeRocketAction(const char* pName) NO_INLINE;
    void changeRocketActionBck(const char* pName) NO_INLINE;
    const TVec3f* getRocketPos() NO_INLINE;
    void faceRocket(const TVec3f* pDir) NO_INLINE;
    void faceRocketInstant(const TVec3f* pDir) NO_INLINE;
    void setRocketPos(const TVec3f& rPos, const TVec3f& rDir) NO_INLINE;
    void setRocketPosFromPrev(const TVec3f& rPos) NO_INLINE;
    void setRocketSpeed(f32 speed) NO_INLINE;
    void startRocketBounce() NO_INLINE;
    s32 getCurrentNode() NO_INLINE;
    void resetCameraMove() NO_INLINE;
    void startRocketMoveSound() NO_INLINE;
    bool updateCameraMove(s32 flag);
    void hideRocket() NO_INLINE;
    void showRocket() NO_INLINE;
    LiveActor* getMeister() const NO_INLINE;
    void appearMeister() NO_INLINE;
    void appearMeisterAtScreenPos(const TVec2f& rPos) NO_INLINE;
    void setMeisterTalk() NO_INLINE;
    void setMeisterTalkEndWait() NO_INLINE;
    s32 isMeisterTalkStarting() NO_INLINE;
    s32 isMeisterTalkEnding() NO_INLINE;
    TalkMessageCtrl* createMeisterTalk(const char* pName) NO_INLINE;
    void initCameraSetup(const char* pSuffix) NO_INLINE;
    void initCameraNodeNames() NO_INLINE;
    void startCameraTargetOther(const char* pSuffix, s32 frames) NO_INLINE;
    void startNodeCamera(s32 node, s32 step) NO_INLINE;
    void declareCamera(const char* pName);
    void startCamera(const char* pName, s32 frames) NO_INLINE;
    void startCameraAtCurrent(const char* pName, s32 frames) NO_INLINE;
    void endCamera(const char* pName, s32 a, s32 b) NO_INLINE;
    void disappearWorld() NO_INLINE;
    void setupRouteOpen(WorldMapResultRoute* pRoute) NO_INLINE;
    void startRouteCamera(WorldMapResultRoute* pRoute, const char* pCameraName) NO_INLINE;
    bool updateRoute(WorldMapResultRoute* pRoute, s32 step, f32 rate) NO_INLINE;
    bool isRouteNotHeld(WorldMapResultRoute* pRoute) NO_INLINE;
    void collectInfos(GalaxyWorldMapInfo** pOut) NO_INLINE;

    /* 0x00 */ u8 _0[0x14];
    /* 0x14 */ WorldMapOwnerContext* mContext;
    /* 0x18 */ u8 _18[4];
    /* 0x1C */ void* mMainLayout;
};
