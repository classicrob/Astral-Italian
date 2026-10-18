#pragma once

#include <JSystem/JGeometry/TMatrix.hpp>
#include <JSystem/JGeometry/TVec.hpp>

class CollisionPartsFilterBase;
class HitInfo;
class HitSensor;
class JMapInfoIter;
class LiveActor;
class Triangle;
class TriangleFilterBase;

namespace MR {
    const TVec3f* getNormal(const Triangle*);
    bool isWallPolygon(const TVec3f&, const TVec3f&);
    bool isFloorPolygon(const TVec3f&, const TVec3f&);
    bool isFloorPolygonCos(const TVec3f&, const TVec3f&, f32);
    bool isWallPolygon(f32);
    bool isFloorPolygon(f32);
    bool isCeilingPolygon(f32);
    bool isWaterPolygon(const Triangle*);
    bool isThroughPolygon(const Triangle*);
    bool getFirstPolyOnLineToMap(TVec3f*, Triangle*, const TVec3f&, const TVec3f&);
    bool getFirstPolyOnLineToMapAndMoveLimit(TVec3f*, Triangle*, const TVec3f&, const TVec3f&);
    bool getFirstPolyOnLineToWaterSurface(TVec3f*, Triangle*, const TVec3f&, const TVec3f&);
    bool getFirstPolyOnLineToMapExceptSensor(TVec3f*, Triangle*, const TVec3f&, const TVec3f&, const HitSensor*);
    bool getFirstPolyOnLineToMapExceptActor(TVec3f*, Triangle*, const TVec3f&, const TVec3f&, const LiveActor*);
    bool getFirstPolyOnLineToMap(TVec3f*, Triangle*, const TVec3f&, const TVec3f&, const CollisionPartsFilterBase*, const TriangleFilterBase*);
    bool getFirstPolyOnLineToWaterSurface(TVec3f*, Triangle*, const TVec3f&, const TVec3f&, const CollisionPartsFilterBase*,
                                          const TriangleFilterBase*);
    bool getFirstPolyNormalOnLineToMap(TVec3f*, const TVec3f&, const TVec3f&, TVec3f*, const HitSensor*);
    u32 getNearPolyOnLineSort(const TVec3f&, const TVec3f&, const TVec3f&, const HitSensor*);
    bool getSortedPoly(TVec3f*, Triangle*, u32);
    const Triangle* getSortedPoly(u32);
    bool isExistMapCollision(const TVec3f&, const TVec3f&);
    bool isExistMoveLimitCollision(const TVec3f&, const TVec3f&);
    bool isExistMapCollisionExceptActor(const TVec3f&, const TVec3f&, const LiveActor*);
    bool isExistMapCollisionExceptSensor(const TVec3f&, const TVec3f&, const LiveActor*);
    bool checkStrikePointToMap(const TVec3f&, HitInfo*);
    bool checkStrikePointToMapExceptSensor(const TVec3f&, const HitSensor*, HitInfo*);
    bool checkStrikeBallToMap(const TVec3f&, f32);
    bool calcMapGround(const TVec3f&, TVec3f*, f32);
    bool calcMapGroundUpper(TVec3f*, const LiveActor*);
    bool isFallNextMove(const LiveActor*, f32, f32, f32, const TriangleFilterBase*);
    bool isFallNextMove(const TVec3f&, const TVec3f&, const TVec3f&, f32, f32, f32, const TriangleFilterBase*);
    bool isFallOrDangerNextMove(const LiveActor*, f32, f32, f32);
    bool isFallOrDangerNextMove(const TVec3f&, const TVec3f&, const TVec3f&, f32, f32, f32);
    void calcVelocityMovingPoint(const Triangle*, const TVec3f&, TVec3f*);
    u32 createAreaPolygonList(Triangle*, u32, const TVec3f&, const TVec3f&);
    u32 createAreaPolygonListArray(Triangle*, u32, TVec3f*, u32);
    bool trySetMoveLimitCollision(LiveActor*);
    bool isBindedGroundIce(const LiveActor*);
    bool isBindedGroundSlip(const LiveActor*);
    bool isBindedGroundSand(const LiveActor*);
    bool isBindedGroundSnow(const LiveActor*);
    bool isBindedGroundDamageFire(const LiveActor*);
    bool isBindedGroundWaterBottomH(const LiveActor*);
    bool isBindedGroundWaterBottomM(const LiveActor*);
    bool isBindedGroundWaterBottomL(const LiveActor*);
    bool isBindedGroundWater(const LiveActor*);
    bool isBindedGroundSinkDeath(const LiveActor*);
    bool isBindedGroundAreaMove(const LiveActor*);
    bool isBindedGroundRailMove(const LiveActor*);
    bool isBindedGroundBrake(const LiveActor*);
    bool isBindedDamageFire(const LiveActor*);
    bool isBindedDamageElectric(const LiveActor*);
    u32 getCameraID(const Triangle*);
    const char* getFloorCodeString(const Triangle*);
    const char* getWallCodeString(const Triangle*);
    const char* getSoundCodeString(const Triangle*);
    s32 getFloorCodeIndex(const JMapInfoIter&);
    s32 getSoundCodeIndex(const JMapInfoIter&);
    s32 getFloorCodeIndex(const Triangle*);
    s32 getWallCodeIndex(const Triangle*);
    bool isGroundCodeWater(const Triangle*);
    bool isGroundCodeWet(const Triangle*);
    bool isGroundCodeWaterIter(const JMapInfoIter&);
    bool isGroundCodeIceIter(const JMapInfoIter&);
    bool isGroundCodeDeath(const Triangle*);
    bool isGroundCodeDamage(const Triangle*);
    bool isGroundCodeNoDig(const Triangle*);
    bool isInPlaneModeCube(const TVec3f&);
    bool isGroundCodeIce(const Triangle*);
    bool isGroundCodeDamageFire(const Triangle*);
    bool isGroundCodeFireDance(const Triangle*);
    bool isGroundCodeSand(const Triangle*);
    bool isGroundCodeDamageElectric(const Triangle*);
    bool isGroundCodeWaterBottomH(const Triangle*);
    bool isGroundCodeWaterBottomM(const Triangle*);
    bool isGroundCodeForceDash(const Triangle*);
    bool isGroundCodeSinkDeath(const Triangle*);
    bool isGroundCodeRailMove(const Triangle*);
    bool isGroundCodeAreaMove(const Triangle*);
    bool isGroundCodeNoStampSand(const Triangle*);
    bool isGroundCodeSinkDeathMud(const Triangle*);
    bool isGroundCodeBrake(const Triangle*);
    bool isWallCodeGhostThrough(const Triangle*);
    bool isWallCodeRebound(const Triangle*);
    bool isWallCodeNoAction(const Triangle*);
    bool isCameraCodeThrough(const Triangle*);
    bool isCodeSand(const Triangle*);
    const Triangle* getCameraPolyFast(const TVec3f&, const TVec3f&, const HitSensor*);
    bool getFirstPolyOnLineBFast(const TVec3f&, const TVec3f&, TVec3f*, Triangle*);

    // SMG2 DashYoshi-step variants of the map queries (unnamed in the target; named by address)
    bool fn_80037E80(TVec3f*, Triangle*, const TVec3f&, const TVec3f&);
    bool fn_80038060(TVec3f*, Triangle*, const TVec3f&, const TVec3f&, const CollisionPartsFilterBase*, const TriangleFilterBase*);
    // PLACEHOLDER(core): unnamed sensor-type line hit (wrapper of getFirstPolyOnLineCategoryWithSensorType); resolve when its owner names it
    bool fn_80038020(TVec3f*, Triangle*, const TVec3f&, const TVec3f&, u32);
    // PLACEHOLDER(core): unnamed ball-hit helper shared with Ride/GliBird.cpp; resolve when its owner names it
    s32 fn_80038770(TVec3f*, TVec3f*, HitSensor**, const TVec3f&, bool*, bool*, f32);
    // PLACEHOLDER(core): unnamed ray-cast helper (same decl as BellyDragonTU02.cpp); resolve when its owner names it
    s32 fn_80038870(const TVec3f&, TVec3f*, const TVec3f&, TVec3f*, HitSensor**, bool*, bool*, f32);
    // PLACEHOLDER(core): unnamed gravity-probe helper (takes LiveActor, pos out); resolve when its owner names it
    bool fn_800389C0(TVec3f*, LiveActor*);
    bool fn_800382E0(const TVec3f&, const TVec3f&);
    bool fn_800384C0(const TVec3f&, const TVec3f&, const LiveActor*);
    bool fn_800386E0(const TVec3f&, HitInfo*);
    bool fn_8003A720(const TVec3f&, const TVec3f&, TVec3f*, Triangle*);
    HitSensor* fn_80038D20(const Triangle*);
    // MapUtil (enemies lane, callees named by address; lead renames symbols)
    bool fn_80037E40(s32, Triangle**, const TVec3f*, const TVec3f*);
    bool fn_80037E40(TVec3f*, Triangle*, const TVec3f&, const TVec3f&);
    bool fn_80038570(const TVec3f*, const TVec3f*);
    bool fn_8008DDC0(MtxPtr, const TVec3f*, f32, f32);
    bool fn_8008E060(const TPos3f*, const TVec3f*);
    LiveActor* fn_8008D920(LiveActor*, LiveActor*, const char*, const char*, s32, MtxPtr, s32);
    LiveActor* fn_8008DB20(LiveActor*, LiveActor*, const char*, const char*, s32, MtxPtr, s32);
};  // namespace MR

namespace Collision {
    s32 checkStrikePointToMap(const TVec3f&, HitInfo*);
    s32 checkStrikeBallToMap(const TVec3f&, f32, const CollisionPartsFilterBase*, const TriangleFilterBase*);
    s32 checkStrikeBallToMapWithMovingReaction(const TVec3f&, f32, const CollisionPartsFilterBase*, const TriangleFilterBase*);
    s32 checkStrikeBallToMapWithThickness(const TVec3f&, f32, f32, const CollisionPartsFilterBase*, const TriangleFilterBase*);
    s32 checkStrikeLineToMap(const TVec3f&, const TVec3f&, s32, const CollisionPartsFilterBase*, const TriangleFilterBase*);
    s32 checkStrikeLineToSunshade(const TVec3f&, const TVec3f&, s32, const CollisionPartsFilterBase*, const TriangleFilterBase*);
    const HitInfo* getStrikeInfoMap(u32);
    s32 checkStrikeBallToDashYoshiStepAndMap(const TVec3f&, f32, const CollisionPartsFilterBase*, const TriangleFilterBase*);
    s32 checkStrikeBallToDashYoshiStepAndMapWithThickness(const TVec3f&, f32, f32, const CollisionPartsFilterBase*, const TriangleFilterBase*);
    const HitInfo* fn_8003B110(u32);  // SMG2 DashYoshi-step variant of getStrikeInfoMap (unnamed; by address)
    s32 checkStrikePointToDashYoshiStepAndMap(const TVec3f&, HitInfo*);
    u32 getStrikeInfoNumMap();
};  // namespace Collision

namespace MR {
    s32 calcMapHitAlongDirection(const TVec3f&, TVec3f*, const TVec3f&, TVec3f*, HitSensor**, bool*, bool*, f32);
}
