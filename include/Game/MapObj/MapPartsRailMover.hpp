#pragma once

#include "Game/MapObj/MapPartsFunction.hpp"
#include <JSystem/JGeometry.hpp>

class MapPartsRailPointPassChecker;

class MapPartsRailMover : public MapPartsFunction {
public:
    MapPartsRailMover(LiveActor*);

    virtual ~MapPartsRailMover();
    virtual void init(const JMapInfoIter&);
    virtual void movement();
    virtual bool isWorking() const;
    virtual void start();
    virtual void end();
    virtual bool receiveMsg(u32);

    virtual void doStart();

    void moveToInitPos();
    void moveToEndPos();
    NO_INLINE void updatePosition();
    void startWithSignalMotion();
    void cancelSignalMotion();
    bool tryResetPositionRepeat();
    void resetToInitPos();
    void passPoint();
    void reachedEnd();
    void reachedEndPlayerOn();
    bool isReachedEnd() const;
    bool isDone() const;
    void setStateStopAtEndBeforeRotate();
    void calcTimeToNextRailPoint(f32*) const;
    void endRotateAtPoint();
    void calcMoveSpeed(f32*) const;
    void calcMoveSpeedDirect(f32*) const;
    void calcMoveSpeedTime(f32*) const;
    void updateAccel();
    bool tryPassPoint();
    bool tryRestartAtEnd();
    void restartAtEnd();
    void exeMove();
    void exeMoveStart();
    void exeStopAtPoint();
    void exeStopAtEnd();
    void exeWait();
    void exeVanish();
    void exeRotateAtPoint();
    void exeWaitForRestartByPlayerOn();
    void exeStopAtEndWithPlayerOn();
    void exeRotateAtEndPoint();
    void exeWaitForRestart();

    void setStopTypeStopAtEnd();
    void setSignMotionTypeShake();
    bool isStarting() const;

    /* 0x28 */ MapPartsRailPointPassChecker* mRailPointPassChecker;
    /* 0x2C */ s32 mMoveConditionType;
    /* 0x30 */ s32 mMoveStopType;
    /* 0x34 */ s32 mSignMotionType;
    /* 0x38 */ TVec3f mPosition;  // Current rail position, including any remaining snap offset.
    /* 0x44 */ TVec3f mSnapOffset;  // Placement position minus the initial rail position.
    /* 0x50 */ f32 mInitialRailCoord;
    /* 0x54 */ s32 mStopTime;
    /* 0x58 */ f32 mSpeed;
    /* 0x5C */ s32 mAccelTime;
    /* 0x60 */ f32 mAcceleration;
    /* 0x64 */ f32 mSignMotionRailCoord;
    /* 0x68 */ s32 mSnapDuration;  // MoveSnap duration in movement frames.
    /* 0x6C */ s32 mSnapFramesRemaining;
    /* 0x70 */ bool mHasRepeated;  // Set on repeat restart; no reader is currently known.
    /* 0x74 */ s32 mSignMotionTimeOverride;  // -1 uses MapParts::getMoveStartSignalTimeForRail().
    /* 0x78 */ bool mDisableSignMotionAnimation;  // Use rail-coordinate shake instead of RailMoveSign animation.
};

