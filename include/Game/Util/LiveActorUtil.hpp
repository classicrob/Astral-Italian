#pragma once

#include "Game/Util/ActorAnimUtil.hpp"
#include "Game/Util/ActorInitUtil.hpp"

#include <JSystem/JGeometry/TMatrix.hpp>
#include <JSystem/JGeometry/TVec.hpp>
#include <revolution.h>

class ActorLightCtrl;
class ActorPoseTQ;
class AnimScaleController;
struct BckCtrlData;
class BreakModel;
class CollisionObj;
class CollisionParts;
class CollisionPartsFilterBase;
class DemoObj;
class FurMulti;
class JMapInfoIter;
class LiveActor;
class LiveActorGroup;
class LodCtrl;
class MirrorActor;
class NameObj;
class ModelObj;
class Nerve;
class HitSensor;
class PartsModel;
class RailObj;
class ResTIMG;
class ResourceHolder;
class TriangleFilterBase;
class ProjmapEffectMtxSetter;

namespace MR {
    void tryHitReactionDead(const LiveActor*);
    void tryHitReactionBreak(const LiveActor*);
    void tryHitReactionAppear(const LiveActor*);
    void tryHitReactionBlow(const LiveActor*, const TVec3f*);
    void tryHitReactionAtPos(const LiveActor*, const char*, const TVec3f*);
    void tryHitReactionBetweenSensors(const LiveActor*, const char*, HitSensor*, HitSensor*);
    void tryHitReaction(const LiveActor*, const char*);
    enum CollisionScaleType { CollisionScaleType_AutoEqualScale, CollisionScaleType_NotUsingScale, CollisionScaleType_Default, CollisionScaleType_Unk2 = CollisionScaleType_Default };
    void initSensors(LiveActor*, const char*, const char*);
    void initSwitches(LiveActor*, const JMapInfoIter&, const char*, const char*);

    void setPosition(LiveActor* pActor, const TVec3f& rPosition);
    void setPosition(LiveActor* pActor, f32 x, f32 y, f32 z);
    void setRotation(LiveActor* pActor, const TVec3f& rRotation);
    void setRotation(LiveActor* pActor, f32 x, f32 y, f32 z);
    void setRotation(LiveActor* pActor, f32 xyz);
    void setRotation(LiveActor* pActor, const TPos3f& rMtx);
    void setRotation(LiveActor* pActor, const TQuat4f& rQuat);
    void setScale(LiveActor* pActor, const TVec3f&);
    void setScale(LiveActor* pActor, f32, f32, f32);
    void setScale(LiveActor* pActor, f32 xyz);
    void setScale(LiveActor* pActor, f32);
    void copyTransRotateScale(const LiveActor* pToActor, LiveActor* pFromActor);
    void copyTransRotate(const LiveActor* pToActor, LiveActor* pFromActor);
    void rotateAroundActor(TVec3f& rRotated, const TVec3f& rRotateOffset, const LiveActor* pActor);

    bool isValidMovement(const LiveActor* pActor);
    bool isValidCalcAnim(const LiveActor* pActor);
    bool isValidCalcViewAndEntry(const LiveActor* pActor);
    bool isValidDraw(const LiveActor* pActor);
    void calcAnimDirect(LiveActor* pActor);

    void setClippingTypeSphere(LiveActor* pActor, f32 clippingRadius);
    void setClippingTypeSphere(LiveActor* pActor, f32 clippingRadius, const TVec3f* pClippingPos);
    void setClippingTypeSphereContainsModelBoundingBox(LiveActor* pActor, f32 radiusExtension);
    void setClippingFar50m(LiveActor* pActor);
    void setClippingFar100m(LiveActor* pActor);
    void setClippingFar200m(LiveActor* pActor);
    void setClippingFar300m(LiveActor* pActor);
    void setClippingFarMax(LiveActor* pActor);
    void setClippingFar(LiveActor* pActor, f32 farDistance);

    void setGroupClipping(LiveActor* pActor, const JMapInfoIter& rIter, int size);
    void validateClipping(LiveActor* pActor);
    void invalidateClipping(LiveActor* pActor);
    bool changeShowModelFlagSyncNearClipping(LiveActor* pActor, f32 nearDistance);
    bool isClipped(const LiveActor* pActor);
    bool isInvalidClipping(const LiveActor* pActor);

    void setBaseTRMtx(LiveActor* pActor, MtxPtr pTRMtx);
    void setBaseTRMtx(LiveActor* pActor, const TPos3f& rMtx);
    void setBaseTRMtx(LiveActor* pActor, const TQuat4f& rQuat);
    void setBaseTRMtx(LiveActor* pActor, ActorPoseTQ* pPose);
    void setBaseScale(LiveActor* pActor, const TVec3f& rScale);
    void updateBaseScale(LiveActor* pActor, AnimScaleController* pScaleController);

    ResourceHolder* getResourceHolder(const LiveActor* pActor);
    ResourceHolder* getModelResourceHolder(const LiveActor* pActor);
    ResTIMG* getTexFromModel(const char* pTexName, const LiveActor* pActor);
    ResTIMG* getTexFromArc(const char* pBtiName, const LiveActor* pActor);
    const char* getModelResName(const LiveActor* pActor);
    bool isExistTexture(const LiveActor* pActor, const char* pTexName);
    bool isExistIndirectTexture(const LiveActor* pActor, const char* pTexName);

    void newDifferedDLBuffer(LiveActor* pActor);
    void initDLMakerFog(LiveActor* pActor, bool);

    // TODO: Need J3DGXColor
    // void initDLMakerMatColor0(LiveActor* pActor, const char* pMaterialName, const J3DGXColor* pMatColor);
    void initDLMakerChangeTex(LiveActor* pActor, const char* pTexName);
    void initDLMakerTexMtx(LiveActor* pActor, const char* pTexName);
    ProjmapEffectMtxSetter* initDLMakerProjmapEffectMtxSetter(LiveActor* pActor);
    // four unknown model-related functions

    void initMirrorReflection(LiveActor* pActor);
    void setMirrorReflectionInfoFromMtxYUp(const TPos3f& rMtx);
    void setMirrorReflectionInfoFromModel(LiveActor* pActor);

    // changeModelDataTexAll

    void initJointTransform(const LiveActor* pActor);
    // getJointTransform

    f32 getBckFrame(const LiveActor* pActor);
    f32 getBtkFrame(const LiveActor* pActor);
    f32 getBrkFrame(const LiveActor* pActor);
    f32 getBtpFrame(const LiveActor* pActor);
    f32 getBvaFrame(const LiveActor* pActor);
    f32 getBckRate(const LiveActor* pActor);
    f32 getBtkRate(const LiveActor* pActor);
    f32 getBrkRate(const LiveActor* pActor);
    f32 getBtpRate(const LiveActor* pActor);
    f32 getBpkRate(const LiveActor* pActor);
    f32 getBvaRate(const LiveActor* pActor);
    f32 getBckFrameMax(const LiveActor* pActor);
    f32 getBtkFrameMax(const LiveActor* pActor);
    f32 getBrkFrameMax(const LiveActor* pActor);
    f32 getBtpFrameMax(const LiveActor* pActor);
    f32 getBvaFrameMax(const LiveActor* pActor);
    const char* getPlayingBckName(const LiveActor* pActor);
    void reflectBckCtrlData(LiveActor* pActor, const BckCtrlData& rData);
    // one unknown function

    s32 getLightNumMax(const LiveActor* pActor);

    void initLightCtrl(LiveActor* pActor);
    void initLightCtrlForPlayer(LiveActor* pActor);
    void updateLightCtrl(LiveActor* pActor);
    void updateLightCtrlDirect(LiveActor* pActor);
    void loadActorLight(const LiveActor* pActor);
    // getLightAmbientColor
    ActorLightCtrl* getLightCtrl(const LiveActor* pActor);

    bool isStep(const LiveActor* pActor, s32 step) NO_INLINE;
    bool isFirstStep(const LiveActor* pActor);
    bool isLessStep(const LiveActor* pActor, s32 step);
    bool isLessEqualStep(const LiveActor* pActor, s32 step);
    bool isGreaterStep(const LiveActor* pActor, s32 step);
    bool isGreaterEqualStep(const LiveActor* pActor, s32 step);
    bool isIntervalStep(const LiveActor* pActor, s32 step);
    bool isNewNerve(const LiveActor* pActor, s32 step);
    f32 calcNerveRate(const LiveActor* pActor, s32 step);
    f32 calcNerveRate(const LiveActor* pActor, s32 minStep, s32 maxStep);
    f32 calcNerveEaseInRate(const LiveActor* pActor, s32 step);
    f32 calcNerveEaseOutRate(const LiveActor* pActor, s32 step);
    f32 calcNerveEaseOutRate(const LiveActor* pActor, s32 minStep, s32 maxStep);
    f32 calcNerveEaseInOutRate(const LiveActor* pActor, s32 step);
    f32 calcNerveEaseInOutRate(const LiveActor* pActor, s32 minStep, s32 maxStep);
    f32 calcNerveValue(const LiveActor* pActor, s32 step, f32 min, f32 max);
    f32 calcNerveValue(const LiveActor* pActor, s32 minStep, s32 maxValue, f32 min, f32 max);
    f32 calcNerveEaseInValue(const LiveActor* pActor, s32 step, f32 min, f32 max);
    f32 calcNerveEaseInValue(const LiveActor* pActor, s32 minStep, s32 maxValue, f32 min, f32 max);
    f32 calcNerveEaseOutValue(const LiveActor* pActor, s32 step, f32 min, f32 max);
    f32 calcNerveEaseOutValue(const LiveActor* pActor, s32 minStep, s32 maxValue, f32 min, f32 max);
    f32 calcNerveEaseInOutValue(const LiveActor* pActor, s32 step, f32 min, f32 max);
    f32 calcNerveEaseInOutValue(const LiveActor* pActor, s32 minStep, s32 maxValue, f32 min, f32 max);
    f32 calcNerveParabolaValue(const LiveActor* pActor, s32 step);
    void setNerveAtStep(LiveActor* pActor, const Nerve* pNerve, s32 step);
    void setNerveAtBckStopped(LiveActor* pActor, const Nerve* pNerve);
    bool trySetNerve(LiveActor* pActor, const Nerve* pNerve);

    bool isOnGround(const LiveActor* pActor);
    bool isOnGroundCos(const LiveActor* pActor, f32 cos);
    bool isBindedGround(const LiveActor* pActor);
    bool isBindedWall(const LiveActor* pActor);
    bool isBindedWallOfMap(const LiveActor* pActor);
    bool isBindedWallOfMoveLimit(const LiveActor* pActor);
    bool isBindedRoof(const LiveActor* pActor);
    bool isBinded(const LiveActor* pActor);
    bool isPressedRoofAndGround(const LiveActor* pActor);
    bool isPressedBetweenWalls(const LiveActor* pActor);

    const TVec3f& getGroundNormal(const LiveActor* pActor);
    const TVec3f& getWallNormal(const LiveActor* pActor);
    const TVec3f& getRoofNormal(const LiveActor* pActor);
    const TVec3f& getBindedNormal(const LiveActor* pActor);
    const TVec3f& getBindedHitPos(const LiveActor* pActor);
    const TVec3f& getGroundHitPos(const LiveActor* pActor);
    const TVec3f& getWallHitPos(const LiveActor* pActor);
    const TVec3f& getRoofHitPos(const LiveActor* pActor);
    f32 calcHitPowerToWall(const LiveActor* pActor);

    u32 getBindedPlaneNum(const LiveActor* pActor);
    const TVec3f* getBindedPlaneNormal(const LiveActor* pActor, int);
    HitSensor* getBindedPlaneSensor(const LiveActor* pActor, int);
    const TVec3f& getBindedFixReactionVector(const LiveActor* pActor);
    void setBinderOffsetVec(LiveActor* pActor, const TVec3f* pOffset, bool);
    void setBinderRadius(LiveActor* pActor, f32 radius);
    void scaleBinderRadius(LiveActor* pActor, f32 scale);
    f32 getBinderRadius(const LiveActor* pActor);
    void setBinderIgnoreMovingCollision(LiveActor* pActor);

    void validateExCollisionParts(LiveActor* pActor);
    void invalidateExCollisionParts(LiveActor* pActor);
    void setBinderCollisionPartsFilter(LiveActor* pActor, CollisionPartsFilterBase* pCollisionPartsFilter);
    void setBinderExceptSensorType(LiveActor* pActor, u32 exceptSensorType);
    void setBinderExceptActor(LiveActor* pActor, const LiveActor* pExceptActor);
    void setBindTriangleFilter(LiveActor* pActor, TriangleFilterBase* pTriangleFilter);
    bool isExistBinder(const LiveActor* pActor);

    void onEntryDrawBuffer(LiveActor* pActor);
    void offEntryDrawBuffer(LiveActor* pActor);

    bool isDead(const LiveActor* pActor);
    bool isHiddenModel(const LiveActor* pActor);
    void showModel(LiveActor* pActor);
    void showModelAndEffects(LiveActor* pActor);
    void hideModel(LiveActor* pActor);
    void hideModelAndEffects(LiveActor* pActor);
    void hideModelAndOnCalcAnim(LiveActor* pActor);
    void showModelIfHidden(LiveActor* pActor) NO_INLINE;
    void hideModelIfShown(LiveActor* pActor);
    void hideModelAndOnCalcAnimIfShown(LiveActor* pActor);
    bool isStopAnimFrame(const LiveActor* pActor);
    void stopAnimFrame(LiveActor* pActor);
    void releaseAnimFrame(LiveActor* pActor);
    bool isNoCalcAnim(const LiveActor* pActor);
    void onCalcAnim(LiveActor* pActor);
    void offCalcAnim(LiveActor* pActor);
    bool isNoCalcView(const LiveActor* pActor);
    bool isNoEntryDrawBuffer(const LiveActor* pActor);
    bool isNoBind(const LiveActor* pActor);
    void onBind(LiveActor* pActor);
    void offBind(LiveActor* pActor);
    bool isCalcGravity(const LiveActor* pActor);
    void onCalcGravity(LiveActor* pActor);
    void offCalcGravity(LiveActor* pActor);

    // various ActorActionKeeper functions

    LiveActorGroup* joinToGroup(LiveActor* pActor, const char* pGroupName);
    LiveActorGroup* joinToGroupArray(LiveActor* pActor, const JMapInfoIter& rIter, const char* pGroupName, s32 size);
    LiveActorGroup* getGroupFromArray(const LiveActor* pActor);
    void callMakeActorDeadAllGroupMember(const LiveActor* pActor);
    void callKillAllGroupMember(const LiveActor* pActor);
    void callMakeActorAppearedAllGroupMember(const LiveActor* pActor);
    void callAppearAllGroupMember(const LiveActor* pActor);
    void callRequestMovementOnAllGroupMember(const LiveActor* pActor);
    void callInvalidateClippingAllGroupMember(const LiveActor* pActor);
    void callValidateClippingAllGroupMember(const LiveActor* pActor);

    void addToAttributeGroupSearchTurtle(const LiveActor* pActor);
    bool isExistInAttributeGroupSearchTurtle(const LiveActor* pActor);

    void calcGravity(LiveActor* pActor);
    void calcGravity(LiveActor* pActor, const TVec3f& rPosition);
    void calcGravityOrZero(LiveActor* pActor);
    void calcGravityOrZero(LiveActor* pActor, const TVec3f& rPosition);

    FurMulti* initFur(LiveActor* pActor);
    FurMulti* initFurEnemy(LiveActor* pActor);
    FurMulti* initFurPlanet(LiveActor* pActor);
    FurMulti* initFurPlayer(LiveActor* pActor);

    void initCollisionParts(LiveActor* pActor, const char* pCollisionName, HitSensor* pBodySensor, MtxPtr pMtx);
    void initCollisionPartsAutoEqualScale(LiveActor* pActor, const char* pCollisionName, HitSensor* pBodySensor, MtxPtr pMtx);
    void initCollisionPartsAutoEqualScaleOne(LiveActor* pActor, const char* pCollisionName, HitSensor* pBodySensor, MtxPtr pMtx);
    void initCollisionPartsFromResourceHolder(LiveActor* pActor, const char* pCollisionName, HitSensor* pBodySensor, ResourceHolder* pResourceHolder,
                                              MtxPtr pMtx);
    CollisionParts* createCollisionPartsFromLiveActor(LiveActor* pActor, const char* pCollisionName, HitSensor* pBodySensor,
                                                      CollisionScaleType collisionScaleType);
    CollisionParts* createCollisionPartsFromLiveActor(LiveActor* pActor, const char* pCollisionName, HitSensor* pBodySensor, MtxPtr pMtx,
                                                      CollisionScaleType collisionScaleType);
    CollisionParts* createCollisionPartsFromResourceHolder(ResourceHolder* pResourceHolder, const char* pCollisionName, HitSensor* pBodySensor,
                                                           const TMtx34f& rMtx, CollisionScaleType collisionScaleType);
    CollisionParts* createDashYoshiStepCollisionPartsFromResourceHolder(ResourceHolder* pResourceHolder, const char* pCollisionName,
                                                                        HitSensor* pBodySensor, MtxPtr pMtx, CollisionScaleType collisionScaleType);
    CollisionParts* tryCreateCollisionMoveLimit(LiveActor* pActor, HitSensor* pBodySensor);
    CollisionParts* tryCreateCollisionWaterSurface(LiveActor* pActor, HitSensor* pBodySensor);

    f32 getCollisionBoundingSphereRange(const LiveActor* pActor);
    bool isValidCollisionParts(LiveActor* pActor);
    void validateCollisionParts(LiveActor* pActor);
    void validateCollisionParts(CollisionParts* pCollisionParts);
    void invalidateCollisionParts(LiveActor* pActor);
    void invalidateCollisionParts(CollisionParts* pCollisionParts);
    void onUpdateCollisionParts(LiveActor* pActor);
    void onUpdateCollisionPartsOneTimeImmediately(LiveActor* pActor);
    void offUpdateCollisionParts(LiveActor* pActor);
    // one collision-related function

    void resetAllCollisionMtx(LiveActor* pActor);
    void setCollisionMtx(LiveActor* pActor);
    void setCollisionMtx(LiveActor* pActor, CollisionParts* pCollisionParts);
    CollisionParts* getCollisionParts(LiveActor* pActor);
    bool isExistCollisionParts(const LiveActor* pActor);
    u32 getCollisionSensortType(const CollisionParts* pCollisionParts);

    DemoObj* createDemoObj(const char* pName, const char* pModelName, MtxPtr pMtx);
    DemoObj* createDemoObjWithAnimArc(const char* pName, const char* pModelName, const char* pAnimArcName, MtxPtr pMtx);
    DemoObj* createDemoObj(const char* pName, const char* pModelName, const char* pFuncName, MtxPtr pMtx);
    DemoObj* createDemoObjWithAnimArc(const char* pName, const char* pModelName, const char* pAnimArcName, const char* pFuncName, MtxPtr pMtx);

    ModelObj* createModelObjMapObjDecoration(const char* pName, const char* pModelName, MtxPtr pMtx);
    ModelObj* createModelObjMapObj(const char* pName, const char* pModelName, MtxPtr pMtx);
    ModelObj* createModelObjMapObjStrongLight(const char* pName, const char* pModelName, MtxPtr pMtx);
    ModelObj* createModelObjNoSilhouettedMapObj(const char* pName, const char* pModelName, MtxPtr pMtx);
    ModelObj* createModelObjNoSilhouettedMapObjStrongLight(const char* pName, const char* pModelName, MtxPtr pMtx);
    ModelObj* createModelObjIndirectMapObj(const char* pName, const char* pModelName, MtxPtr pMtx);
    ModelObj* createModelObjPlayerDecoration(const char* pName, const char* pModelName, MtxPtr pMtx);
    ModelObj* createModelObjEnemy(const char* pName, const char* pModelName, MtxPtr pMtx);
    ModelObj* createModelObjNPC(const char* pName, const char* pModelName, MtxPtr pMtx);
    ModelObj* createModelObjBloomModel(const char* pName, const char* pModelName, MtxPtr pMtx, bool);

    CollisionObj* createCollisionObj(LiveActor* pHost, const char* pCollisionName, bool noInitBodySensor, MtxPtr pMtx);

    BreakModel* createBreakModel(LiveActor* pHost, MtxPtr pMtx);
    BreakModel* tryCreateBreakModel(LiveActor* pHost, MtxPtr pMtx);
    DemoObj* tryCreateBreakDustDemoObj(LiveActor* pHost, MtxPtr pMtx);

    bool tryStartAllAnim(const LiveActor*, const char*);
    ModelObj* createBloomModel2(LiveActor* pHost, MtxPtr pMtx);  // directly jumps to function below
    PartsModel* createBloomModel(LiveActor* pHost, MtxPtr pMtx);
    PartsModel* createWaterModel(LiveActor* pHost, MtxPtr pMtx);
    PartsModel* createIceModel(LiveActor* pHost, MtxPtr pMtx, bool);
    PartsModel* createIndirectPlanetModel(LiveActor* pHost, MtxPtr pMtx);
    PartsModel* createIndirectModel(LiveActor* pHost, MtxPtr pMtx);

    PartsModel* createPartsModelMapObj2(LiveActor* pHost, const char* pName, const char* pModelName, MtxPtr pMtx);  // effectively the same as below
    PartsModel* createPartsModelMapObj(LiveActor* pHost, const char* pName, const char* pModelName, MtxPtr pMtx);
    PartsModel* createPartsModelMapObjStrongLight(LiveActor* pHost, const char* pName, const char* pModelName, MtxPtr pMtx);
    PartsModel* createPartsModelNoSilhouettedMapObj(LiveActor* pHost, const char* pName, const char* pModelName, MtxPtr pMtx);
    PartsModel* createPartsModelEnemy(LiveActor* pHost, const char* pName, const char* pModelName, MtxPtr pMtx);
    PartsModel* createPartsModelNpc(LiveActor* pHost, const char* pName, const char* pModelName, MtxPtr pMtx);
    PartsModel* createPartsModelIndirectNpc(LiveActor* pHost, const char* pName, const char* pModelName, MtxPtr pMtx);
    PartsModel* createPartsModelEnemyAndFix(LiveActor* pHost, const char* pName, const char* pModelName, MtxPtr pMtx);
    PartsModel* createPartsModelNpcAndFix(LiveActor* pHost, const char* pName, const char* pModelName, MtxPtr pMtx);

    RailObj* createRailObj(const char* pName, LiveActor* pHost, const JMapInfoIter& rIter);

    const char* createLowModelObjName(const LiveActor* pActor);
    const char* createMiddleModelObjName(const LiveActor* pActor);
    LodCtrl* createLodCtrlNPC(LiveActor* pActor, const JMapInfoIter& rIter);
    LodCtrl* createLodCtrlPlanet(LiveActor* pActor, const JMapInfoIter& rIter, f32, s32);
    MirrorActor* tryCreateMirrorActor(LiveActor* pActor, const char* pModelName);

    void stopSceneAtStep(const LiveActor* pActor, s32 step, s32 stopDuration);

    void tryRumblePadAndCameraDistanceStrong(const LiveActor* pActor, f32, f32, f32);
    void tryRumblePadAndCameraDistanceMiddle(const LiveActor* pActor, f32, f32, f32);

    bool tryKillAndForceDeleteEffectAll(LiveActor* pActor);
    void forceClearFootPrint(LiveActor* pActor);

    LiveActor* getLinkedActor(const LiveActor* pActor);

    HitSensor* getBodySensor(LiveActor* pActor);
    void setBodySensorType(LiveActor* pActor, u32 sensorType);
    // LiveActorUtil (enemies lane, callees named by address; lead renames symbols)
    void fn_80034820(LiveActor*);
    void fn_80034760(LiveActor*, HitSensor*, HitSensor*);
    // PLACEHOLDER(enemies): fn_800145D0, fn_800514B0, fn_800513C0 (unnamed core helpers used by FlowerMimic); resolve when their owner unit names them. Signatures inferred from the callers.
    void fn_800145D0(TVec3f*, const TVec3f&);
    void fn_800514B0(TQuat4f*, TVec3f*);
    void fn_800513C0(TQuat4f*, TVec3f*);
    bool isInPlaneModeCube(const TVec3f&);
    // LiveActorUtil (creation) (enemies lane, callees named by address; lead renames symbols)
    NameObj* fn_80105820(LiveActor*, s32, const char*, f32);
};  // namespace MR

// Merged from Petari (core lane): declarations missing here
#include "Game/Animation/AnmPlayer.hpp"
#include "Game/Util/JMapInfo.hpp"
#include <JSystem/J3DGraphBase/J3DStruct.hpp>
#include <revolution/mtx.h>
class BckCtrlData;
class BrkCtrl;
class BtkCtrl;
class Flag;
class MsgSharedGroup;
class TexMtxCtrl;
class XjointTransform;

namespace MR {
    CollisionParts* tryCreateCollisionSunshade(LiveActor*, HitSensor*);
    bool isBindedWallOrSlopeGround(const LiveActor*, f32, TVec3f*);
    bool isPressedMovingWall(const LiveActor*);
    void initDefaultPosNoRepeat(LiveActor*, const JMapInfoIter&);
    bool isActionStart(const LiveActor*, const char*);
    bool tryStartAction(const LiveActor*, const char*);
    void startAllAnim(const LiveActor*, const char*);
    bool isBckExist(const LiveActor*, const char*);
    bool isBtkExist(const LiveActor*, const char*);
    bool isBrkExist(const LiveActor*, const char*);
    bool isBpkExist(const LiveActor*, const char*);
    bool isBtpExist(const LiveActor*, const char*);
    bool isBvaExist(const LiveActor*, const char*);
    void changeModelDataTexAll(LiveActor*, const char*, const ResTIMG&);
    XjointTransform* getJointTransform(const LiveActor*, const char*);
    void setJointTransformLocalMtx(const LiveActor*, const char*, MtxPtr);
    void initLightCtrlNoDrawEnemy(LiveActor*);
    void initLightCtrlNoDrawMapObj(LiveActor*);
    void calcLightPos0(TVec3f*, const LiveActor*);
    void calcLightPos1(TVec3f*, const LiveActor*);
    const GXColor* getLightAmbientColor(const LiveActor*);
    void calcWallNormalHorizontal(TVec3f*, const LiveActor*);
    f32 calcHitPowerToGround(const LiveActor*);
    LiveActor* getPairedGroupMember(const LiveActor*);
    s32 countHideGroupMember(const LiveActor*);
    s32 countShowGroupMember(const LiveActor*);
    void addToAttributeGroupReflectSpinningBox(const LiveActor*);
    bool isExistInAttributeGroupReflectSpinningBox(const LiveActor*);
    CollisionParts* tryCreateCollisionAllOtherCategory(LiveActor*, HitSensor*, CollisionParts**, CollisionParts**, CollisionParts**);
    bool isExistKcl(LiveActor*, const char*);
    void onUpdateCollisionPartsOnetimeImmediately(LiveActor*);
    u32 getCollisionSensorType(const CollisionParts*);
    ModelObj* createModelObjNpc(const char*, const char*, MtxPtr);
    ModelObj* createModelObjPlanetLow(const char*, const char*, MtxPtr);
    LodCtrl* createLodCtrlMapObj(LiveActor*, const JMapInfoIter&, f32);
    Flag* createMapFlag(const char*, const char*, const TVec3f*, const TVec3f&, f32, f32, f32, s32, s32, f32);
    void tryRumblePadAndCameraDistanceVeryStrong(const LiveActor*, f32, f32, f32);
};  // namespace MR

// Merged from Petari (core lane): overloads missing here
class J3DGXColor;

namespace MR {
    bool isExistIndirectTexture(const LiveActor*);
    void initDLMakerMatColor0(LiveActor*, const char*, const J3DGXColor*);
    bool isNewNerve(const LiveActor*);
    void initCollisionPartsFromResourceHolder(LiveActor*, const char*, HitSensor*, ResourceHolder*, MtxPtr);
    CollisionParts* createCollisionPartsFromLiveActor(LiveActor*, const char*, HitSensor*, MR::CollisionScaleType);
    CollisionParts* createCollisionPartsFromLiveActor(LiveActor*, const char*, HitSensor*, MtxPtr, MR::CollisionScaleType);
    CollisionParts* createCollisionPartsFromResourceHolder(ResourceHolder*, const char*, HitSensor*, const TPos3f&, MR::CollisionScaleType);
    PartsModel* createPartsModelEnemyAndFix(LiveActor*, const char*, const char*, MtxPtr, const TVec3f&, const TVec3f&, const char*);
    PartsModel* createPartsModelNpcAndFix(LiveActor*, const char*, const char*, const char*);
    // Provisional name for the unnamed SMG2 function fn_804E4FB0 (used by MiniMeister::exeTalkEnd).
    void stopTalkEffect(LiveActor* pActor);
};  // namespace MR

namespace MR {
}
