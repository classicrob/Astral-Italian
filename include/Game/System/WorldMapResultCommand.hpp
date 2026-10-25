#pragma once

#include "Game/NameObj/NameObj.hpp"
#include <JSystem/JGeometry/TVec.hpp>

class LiveActor;
class WorldMapResultController;
class WorldMapResultDraw;
class WorldMapResultWorld;
class WorldMapResultRoute;
class MiniTicoMaster;

/// @brief SMG2-only. Drives the "result" cutscenes on the world map (route opening, comet appear, Tico creation...).
class WorldMapResultCommand : public NameObj {
public:
    WorldMapResultCommand();

    virtual ~WorldMapResultCommand();
    virtual void init(const JMapInfoIter&);

    void startResult(s32 worldId);
    bool isResultActive() const NO_INLINE;
    s32 getResultWorldId() const NO_INLINE;
    void endResult();
    void setCameraEye(const TVec3f&);
    void setCameraTarget(const TVec3f&);
    const TVec3f& getCameraEye() const NO_INLINE;
    const TVec3f& getCameraTarget() const NO_INLINE;
    void movementDraw();
    void calcAnimDraw();
    void calcViewAndEntryDraw();
    void drawDraw();
    bool isActive() const;
    bool isStateDead() const;
    bool tryStartResultMapStart();
    bool tryStartResultMapEnd();
    bool tryRouteOpen();
    void startRouteCreateTico();
    bool updateController();
    bool isAnyResultReady() NO_INLINE;
    bool isWorldEnabled(s32) const NO_INLINE;
    bool tryCometAppear();
    void endCurrent();
    MiniTicoMaster* getCurrentTicoItem() NO_INLINE;  // was fn_804F0500
    WorldMapResultWorld* getCurrentWorld() NO_INLINE;  // was fn_804F0560
    void appearRocketAt(const TVec3f&) NO_INLINE;  // was fn_804F05B0
    void appearRocketAtNode(s32, s32) NO_INLINE;  // was fn_804F0630
    void setupTicoRouteCreate();  // was fn_804F0700
    void refreshTicoStartNode();  // was fn_804F08C0
    void updateCurrentWorldUnits();  // was fn_804F0920
    void setupRouteOpen(s32, WorldMapResultRoute*);  // was fn_804F0970
    void setupRouteOpenFromSwitchMoveBlock(s32, WorldMapResultRoute*);  // was fn_804F09C0
    void setRouteCamera(s32, WorldMapResultRoute*);  // was fn_804F0A40
    bool isRouteNotHeld(s32, WorldMapResultRoute*);  // was fn_804F0B20
    TVec3f* getGalaxyPosition(const char*) NO_INLINE;  // was fn_804F0B40
    void endGalaxyPointingAnim(const char*, s32);  // was fn_804F0BA0
    void clearGalaxyComet(const char*);  // was fn_804F0C00
    void appearCometAtGalaxy(const char*, LiveActor*);  // was fn_804F0C60
    void setupCometGalaxy(const char*);  // was fn_804F0D00
    bool updateCometCamera(const char*, TVec3f*, TVec3f*, f32);  // was fn_804F0D90
    bool updateRouteOpenSound(s32) const;  // was fn_804F0EE0

    /* 0x14 */ s32 mWorldNum;
    /* 0x18 */ WorldMapResultWorld** mWorlds;
    /* 0x1C */ WorldMapResultController* mController;
    /* 0x20 */ WorldMapResultDraw* mDraw;
    /* 0x24 */ LiveActor* mRocket;
    /* 0x28 */ LiveActor* mActor;
};

bool setWorldMapResultActor(LiveActor*);
