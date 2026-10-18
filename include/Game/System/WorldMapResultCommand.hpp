#pragma once

#include "Game/NameObj/NameObj.hpp"
#include <JSystem/JGeometry/TVec.hpp>

class LiveActor;
class WorldMapResultController;
class WorldMapResultDraw;
class WorldMapResultWorld;

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

    /* 0x14 */ s32 mWorldNum;
    /* 0x18 */ WorldMapResultWorld** mWorlds;
    /* 0x1C */ WorldMapResultController* mController;
    /* 0x20 */ WorldMapResultDraw* mDraw;
    /* 0x24 */ LiveActor* mRocket;
    /* 0x28 */ LiveActor* mActor;
};
