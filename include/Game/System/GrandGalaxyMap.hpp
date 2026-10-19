#pragma once

#include "Game/System/NerveExecutor.hpp"
#include <JSystem/JGeometry/TVec.hpp>

class GrandGalaxyMapGalaxy;
class LiveActor;
class MiniPlayerRocket;
struct PartsWarpInfo;
class GrandGalaxyMapInfo;
class WorldMapCameraHolder;
class WorldMapOwner;
class WorldMapResultWorld;

/// The galaxy icon actors of all worlds, read from the "Galaxy" table of each world (8 bytes).
class GrandGalaxyMapGalaxyHolder {
public:
    GrandGalaxyMapGalaxyHolder();

    void init();
    GrandGalaxyMapGalaxy* find(const char* pStageName) const;

    /* 0x00 */ s32 mNum;
    /* 0x04 */ GrandGalaxyMapGalaxy** mGalaxies;
};

/// The world the grand galaxy map shows, its report layout and its galaxy icons (12 bytes).
class GrandGalaxyMapParts {
public:
    GrandGalaxyMapParts();

    void init(s32 worldId);
    void prepare();
    void finish();

    /* 0x00 */ WorldMapResultWorld* mWorld;
    /* 0x04 */ GrandGalaxyMapInfo* mInfo;
    /* 0x08 */ GrandGalaxyMapGalaxyHolder* mGalaxies;
};

class GrandGalaxyMap;

class GrandGalaxyMapStateEnterDemo;
class GrandGalaxyMapStateStart;
class GrandGalaxyMapStateWait;
class GrandGalaxyMapStateNews;
class GrandGalaxyMapStateWorldWarp;

/// NerveExecutor "シーケンス管理" of the grand galaxy map: runs its states (0x28 bytes, 0x804E0440..).
class GrandGalaxyMapSequence : public NerveExecutor {
public:
    GrandGalaxyMapSequence(GrandGalaxyMap* pHost);

    virtual ~GrandGalaxyMapSequence();

    void init(bool isDirect);
    void start();
    void end();
    void exeEnter();
    void exeNews();
    void exeWait();
    inline void exePointMove();
    inline void exeWorldWarp();
    bool isWorldWarp() const;
    bool isWorldWarpEnd() const;
    bool tryPointMove();

    /* 0x08 */ GrandGalaxyMap* mHost;
    /* 0x0C */ GrandGalaxyMapStateEnterDemo* mEnterDemo;
    /* 0x10 */ GrandGalaxyMapStateStart* mStart;
    /* 0x14 */ GrandGalaxyMapStateWait* mWait;
    /* 0x18 */ u32 _18;
    /* 0x1C */ GrandGalaxyMapStateNews* mNews;
    /* 0x20 */ GrandGalaxyMapStateWorldWarp* mWorldWarp;
    /* 0x24 */ bool mIsStickTilted;
};

/// NerveExecutor "グランドギャラクシーマップ": the zoomed-out map of all worlds, owned by WorldMapOwner (0x18 bytes).
class GrandGalaxyMap : public NerveExecutor {
public:
    GrandGalaxyMap(WorldMapOwner* pOwner);

    virtual ~GrandGalaxyMap();

    void init(bool isDirect);
    void start();
    void end();
    void update();
    WorldMapResultWorld* getWorld() const;
    WorldMapCameraHolder* getCamera() const;
    static s32 findWorldNode(WorldMapResultWorld* pWorld, s32 worldId);
    bool isWorldWarp() const;
    bool isWorldWarpEnd() const;
    void startDemo();
    void endDemo();

    // ---- GrandGalaxyMapTU02 (0x804E1130..) ----
    bool tryDecide(s32 node);
    s32 findStickTarget(f32 threshold);
    bool tryStickMove();
    bool tryWorldWarp(PartsWarpInfo* pInfo);
    void faceRocketInstant(const TVec3f& rDir);
    void setRocketSpeed(f32 speed);
    s32 tryUsePointMark();
    s32 getPathDistance(s32 node) const;
    s32 getPath(s32 node, s32 step) const;
    void changeRocketAction(const char* pName);
    void changeRocketActionBck(const char* pName);
    LiveActor* getMeister() const;
    void appearMeister();
    void setMeisterTalk();
    void setMeisterTalkEndWait();
    s32 isMeisterTalkStarting();
    s32 isMeisterTalkEnding();
    void declareAreaCameras();
    void startEventCamera(const char* pName, s32 frames);
    void startAreaCamera(s32 node, s32 frames);
    void activate();
    void deactivate();
    void killRoutes();
    MiniPlayerRocket* getRocket();
    const TVec3f* getNodeOffset(s32 node) const;
    static void getNodePosition(TVec3f* pOut, const GrandGalaxyMap* pMap, s32 node);
    void warpRocketToNode(s32 node);
    bool setupInfo(s32 worldNo);
    void startNextAppear();
    void startPreviousAppear();
    void startAppear();
    void requestPreviousEnd();
    void requestNextEnd();
    void requestEnd();
    const char* getSelectedIconState();
    s32 getCurrentWarpTarget();
    void setupWorldName(s32 worldNo);
    void showGalaxyInfo(const char* pName);
    void requestAppearWorldName();

    /* 0x08 */ GrandGalaxyMapParts* mParts;
    /* 0x0C */ GrandGalaxyMapSequence* mSequence;
    /* 0x10 */ WorldMapOwner* mOwner;
    /* 0x14 */ u8 _14;
    /* 0x15 */ bool mIsEnd;
};
