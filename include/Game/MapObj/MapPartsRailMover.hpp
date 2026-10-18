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
    /* 0x38 */ TVec3f _28;  // Petari _28 (current position)
    /* 0x44 */ TVec3f _44;  // SMG2
    /* 0x50 */ f32 _34;
    /* 0x54 */ s32 mStopTime;
    /* 0x58 */ f32 mSpeed;
    /* 0x5C */ s32 mAccelTime;
    /* 0x60 */ f32 mAcceleration;
    /* 0x64 */ f32 _48;
    /* 0x68 */ s32 _68;  // SMG2
    /* 0x6C */ s32 _6C;  // SMG2
    /* 0x70 */ bool _70;  // SMG2
    /* 0x74 */ s32 _74;  // SMG2, init -1
    /* 0x78 */ bool _78;  // SMG2
};

// SMG2 unnamed helper at 0x80332BB0 (MapPartsFunction.cpp), named by address; used by Garigari init.
void fn_80332BB0(MapPartsRailMover*);
