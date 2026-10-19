#pragma once

#include <JSystem/JGeometry/TBox.hpp>
#include <JSystem/JGeometry/TVec.hpp>
#include <revolution.h>

class LiveActor;
class RailRider;

namespace MR {
    void moveTransToOtherActorRailPos(LiveActor*, const LiveActor*);
    void moveTransToCurrentRailPos(LiveActor*);
    void moveTransXZToCurrentRailPos(LiveActor*);
    void moveCoord(LiveActor*, f32);
    void moveCoordAndTransToRailPoint(LiveActor*, s32);
    s32 moveCoordAndCheckPassPointNo(LiveActor*, f32 speed);
    s32 moveCoordAndFollowTransAndCheckPassPointNo(LiveActor*, f32 speed);
    s32 moveCoordAndFollowTransAndCheckPassPointNo(LiveActor*);
    void moveCoordToRailPoint(LiveActor*, s32);
    void moveCoordToNearestPos(LiveActor*, const TVec3f&);
#ifdef RAILUTIL_OUTLINE_FOLLOW_AND_END
    void moveCoordAndFollowTrans(LiveActor*, f32) NO_INLINE;
#else
    void moveCoordAndFollowTrans(LiveActor*, f32);
#endif
    void moveCoordAndFollowTrans(LiveActor*);
    void moveCoordAndFollowTransXZ(LiveActor*, f32);
    void moveCoordToStartPos(LiveActor*);
    void moveCoordAndTransToStartPos(LiveActor*);
    void moveCoordToEndPos(LiveActor*);
    void moveRailToNextPoint(LiveActor*);
    void moveCoordAndTransToNearestRailPos(LiveActor*);
    void moveCoordAndTransToNearestRailPos(LiveActor*, const TVec3f&);
    void moveCoordAndTransToNearestRailPoint(LiveActor*);
    void moveCoordAndTransToNextPoint(LiveActor*);
    void moveCoordAndTransToRailStartPoint(LiveActor*);

    void setRailDirectionToStart(LiveActor*);
    void setRailDirectionToEnd(LiveActor*);
    void setRailDirectionCloseToCoord(LiveActor*, f32);
    void setRailDirectionCloseToNearestPos(LiveActor*, const TVec3f&);
    void reverseRailDirection(LiveActor*);
    f32 calcMovingDirectionAlongRail(LiveActor*, TVec3f*, const TVec3f&, f32, bool, bool*);  // returns f32 in SMG2 (Hanachan)
    float calcMovingDirectionAlongRailH(LiveActor*, TVec3f*, const TVec3f&, f32, bool*);
    void calcRailClippingInfo(TVec3f*, f32*, const LiveActor*, f32, f32);
    void initAndSetRailClipping(TVec3f*, LiveActor*, f32, f32);
    f32 calcNearestRailCoord(const LiveActor*, const TVec3f&);
    f32 calcNearestRailPos(TVec3f*, const LiveActor*, const TVec3f&);
    float calcNearestRailDirection(TVec3f*, const LiveActor*, const TVec3f&);
    f32 calcNearestRailPosAndDirection(TVec3f*, TVec3f*, const LiveActor*, const TVec3f&);

    void calcBoundingBox(const LiveActor*, TBox3f*, f32);

    f32 calcDifferenceRailCoord(const LiveActor*, f32, f32);
    f32 calcDifferenceRailCoord(const LiveActor*, f32);
    bool isRailCoordNotBehind(const LiveActor*, f32);
    f32 calcDistanceToNearestRailEnd(const LiveActor*);  // fn_80052E70: min(coord, totalLength - coord)
    void calcRandomRailPointPos(TVec3f*, const LiveActor*);
    f32 calcRailSpeedEaseAtEnds(const LiveActor*, f32 speed, f32 minSpeed, f32 range);  // fn_80053700: speed on loop rails, else interpolate(normalize(distToEnd, 0, range), minSpeed, speed)  // fn_80052C00: calcDifferenceRailCoord(actor, currentCoord, coord) >= 0
    float calcDistanceHorizonToCurrentPos(const LiveActor*);
    void calcRailPosAtCoord(TVec3f*, const LiveActor*, f32);
    void calcRailPosFrontCoord(TVec3f*, const LiveActor*, f32);
    void calcRailDirection(TVec3f*, const LiveActor*);
    void calcRailDirectionAtCoord(TVec3f*, const LiveActor*, f32);
    void calcRailDirectionAtNextPoint(TVec3f*, const LiveActor*);
    void sub_800531B0(TVec3f*, const LiveActor*, f32);

    void calcRailPosAndDirectionAtCoord(TVec3f*, TVec3f*, const LiveActor*, f32);
    void calcRailStartPos(TVec3f*, const LiveActor*);
    void calcRailEndPos(TVec3f*, const LiveActor*);
    void calcRailPointPos(TVec3f*, const LiveActor*, int);
    void calcRailRandomPointPos(TVec3f*, const LiveActor*, int);
    void calcRailStartPointPos(TVec3f*, const LiveActor*);
    void calcRailEndPointPos(TVec3f*, const LiveActor*);
#ifdef RAILUTIL_OUTLINE_FOLLOW_AND_END
    void calcRailEndPointDirection(TVec3f*, const LiveActor*) NO_INLINE;
#else
    void calcRailEndPointDirection(TVec3f*, const LiveActor*);
#endif
    void calcRailStartPointPosAndDirection(TVec3f*, TVec3f*, const LiveActor*);
    void calcRailEndPointPosAndDirection(TVec3f*, TVec3f*, const LiveActor*);
    void calcDistanceToNextRailPoint(const LiveActor*, f32*);
    void calcDistanceToCurrentAndNextRailPoint(const LiveActor*, f32*, f32*);

    bool isExistRail(const LiveActor*);
    s32 getRailPointNum(const LiveActor*);
    s32 getCurrentRailPointNo(const LiveActor*);
    s32 getNextRailPointNo(const LiveActor*);
    f32 getRailTotalLength(const LiveActor*);
    f32 fn_80052E20(const LiveActor*);  // rail coord measured in the travel direction: mIsNotReverse ? total - coord : coord
    f32 getRailPartLength(const LiveActor*, int);
    f32 getRailPointCoord(const LiveActor*, int);
    const TVec3f& getRailPos(const LiveActor*);
    const TVec3f& getRailPointPosStart(const LiveActor*);
    const TVec3f& getRailPointPosEnd(const LiveActor*);
    const TVec3f& getRailDirection(const LiveActor*);
    f32 getRailCoord(const LiveActor*);
    f32 getRailCoordSpeed(const LiveActor*);
    void setRailCoord(LiveActor*, f32);
    void setRailCoordSpeed(LiveActor*, f32);
    void accelerateRailCoordSpeed(LiveActor*, f32);
    void adjustmentRailCoordSpeed(LiveActor*, f32, f32);
    void slowDownRailCoordSpeed(LiveActor*, f32);
    void moveRailRider(LiveActor*);
    bool isLoopRail(const LiveActor*);
    bool isRailReachedGoal(const LiveActor*);
    bool isRailReachedNearGoal(const LiveActor*, f32);
    bool isRailReachedEdge(const LiveActor*);
    bool isRailGoingToEnd(const LiveActor*);
    bool isRailReachedNearNextPoint(const LiveActor*, f32);
    bool isRailReachedHorizonCurrentPos(const LiveActor*, f32);
    bool getRailArg0WithInit(const LiveActor*, s32*);
    bool getRailArg7WithInit(const LiveActor*, s32*);
    bool getRailArg0NoInit(const LiveActor*, s32*);
    bool getRailArg1NoInit(const LiveActor*, s32*);
    bool getRailArg3NoInit(const LiveActor*, s32*);
    bool getRailArg1WithInit(const RailRider*, s32*);
    bool getRailPointArg0WithInit(const LiveActor*, s32, s32*);
    bool getRailPointArg2WithInit(const LiveActor*, s32, s32*);
    bool getRailPointArg0NoInit(const LiveActor*, s32, s32*);
    bool getRailPointArg4NoInit(const LiveActor*, s32, s32*);
    bool getRailPointArg6NoInit(const LiveActor*, s32, s32*);
    bool getRailPointArg7NoInit(const LiveActor*, s32, s32*);
    bool getRailPointArg1NoInit(const LiveActor*, s32, f32*);
    bool getCurrentRailPointArg0WithInit(const LiveActor*, s32*);
    bool getCurrentRailPointArg0WithInit(const LiveActor*, f32*);
    bool getCurrentRailPointArg1WithInit(const LiveActor*, s32*);
    bool getCurrentRailPointArg3WithInit(const LiveActor*, s32*);
    bool getCurrentRailPointArg0NoInit(const LiveActor*, s32*);
    bool getCurrentRailPointArg1NoInit(const LiveActor*, s32*);
    bool getCurrentRailPointArg5NoInit(const LiveActor*, s32*);
    bool getCurrentRailPointArg7NoInit(const LiveActor*, s32*);
    bool getCurrentRailPointArg0NoInit(const LiveActor*, f32*);
    bool getCurrentRailPointArg1NoInit(const LiveActor*, f32*);
    bool getCurrentRailPointArg2NoInit(const LiveActor*, f32*);
    bool getCurrentRailPointArg3NoInit(const LiveActor*, f32*);
    bool getCurrentRailPointArg6NoInit(const LiveActor*, f32*);
    bool getCurrentRailPointArg7NoInit(const LiveActor*, f32*);
    bool getCurrentRailPointArg1NoInit(const LiveActor*, bool*);
    bool getCurrentRailPointArg4NoInit(const LiveActor*, bool*);
    bool getNextRailPointArg0WithInit(const LiveActor*, s32*);
    bool getNextRailPointArg0NoInit(const LiveActor*, f32*);
    bool getNextRailPointArg1NoInit(const LiveActor*, f32*);
    bool getNextRailPointArg2NoInit(const LiveActor*, f32*);
    bool getNextRailPointArg3NoInit(const LiveActor*, f32*);
    bool getNextRailPointArg6NoInit(const LiveActor*, f32*);
    bool getNextRailPointArg7NoInit(const LiveActor*, f32*);
    bool getNextRailPointArg2WithInit(const LiveActor*, bool*);
    // RailUtil (enemies lane, callees named by address; lead renames symbols)
    void fn_800531B0(TVec3f*, LiveActor*, f32);
    void fn_80052450(LiveActor*, f32, f32, f32);
};  // namespace MR

// Merged from Petari (core lane): declarations missing here
#include <JSystem/JGeometry.hpp>
#include <JSystem/JGeometry/TBox.hpp>

namespace MR {
    f32 calcRailPosNearestPlayer(TVec3f*, const LiveActor*);
    void calcRailStartPointDirection(TVec3f*, const LiveActor*);
    void calcRailRateToNextPoint(f32*, const LiveActor*);
    bool getRailArg2NoInit(const LiveActor*, s32*);
    bool getRailArg4NoInit(const LiveActor*, s32*);
    bool getRailPointArg3WithInit(const LiveActor*, s32, f32*);
};  // namespace MR
