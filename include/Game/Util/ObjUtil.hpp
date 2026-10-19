#pragma once

#include "Game/Util/Functor.hpp"
#include <va_list.h>
#include "Game/Util/JMapInfo.hpp"
#include <JSystem/JGeometry/TVec.hpp>

class NameObj;
class NameObjGroup;
class LiveActor;
class LayoutActor;
class ModelObj;
class ResourceHolder;
class ResTIMG;
class NameObjAdaptor;
class BenefitItemOneUp;
class BenefitItemObj;
class BenefitItemLifeUp;
class StageSwitchCtrl;
class Color8;
typedef struct _GXColor GXColor;

namespace MR {
    void connectToSceneLayoutTemporarily(NameObj*);
    JMapInfo* createCsvParserFromFile(const char*, const char*, ...);
    bool isExistElement(const JMapInfo*, const char*, const char*);

    bool isJudgedToClipFrustum(const TVec3f& rPosition, f32 radius);
    bool isJudgedToClipFrustum50m(const TVec3f& rPosition, f32 radius);
    bool isJudgedToClipFrustum300m(const TVec3f& rPosition, f32 radius);
    bool isJudgedToNearClip(const TVec3f& rPosition, f32 radius);

    void initialize(NameObj* pObj, const JMapInfoIter& rIterrIter);

    bool isInWater(const TVec3f& rPosition);
    bool isInDeath(const TVec3f& rPosition);
    bool isInDarkMatter(const TVec3f& rPosition);

    void connectToScene(LiveActor* pActor, int movementType, int calcAnimType, int drawBufferType, int drawType);
    void connectToScene(NameObj* pObj, int movementType, int calcAnimType, int drawBufferType, int drawType);
    void connectToSceneCollisionMapObj(LiveActor* pActor);
    void connectToSceneCollisionMapObjWeakLight(LiveActor* pActor);
    void connectToSceneCollisionMapObjStrongLight(LiveActor* pActor);
    void connectToSceneCollisionEnemy(LiveActor* pActor);
    void connectToSceneCollisionEnemyStrongLight(LiveActor* pActor);
    void connectToSceneCollisionEnemyNoShadowedMapObjStrongLight(LiveActor* pActor);
    void connectToSceneNpc(LiveActor* pActor);
    void connectToSceneNpcMovement(LiveActor* pActor);
    void connectToSceneRide(LiveActor* pActor);
    void connectToSceneEnemy(LiveActor* pActor);
    void connectToSceneEnemyMovement(NameObj* pObj);
    void connectToSceneMapObj(LiveActor* pActor);
    void connectToSceneMapObjMovement(NameObj* pObj);
    void connectToSceneMapObjMovementCalcAnim(NameObj* pObj);
    void connectToSceneMapObjNoMovement(LiveActor* pActor);
    void connectToSceneMapObjNoCalcAnim(LiveActor* pActor);
    void connectToSceneMapObjNoCalcAnimStrongLight(LiveActor* pActor);
    void connectToSceneMapObjDecoration(LiveActor* pActor);
    void connectToSceneMapObjDecorationStrongLight(LiveActor* pActor);
    void connectToSceneMapObjDecorationMovement(NameObj* pObj);
    void connectToSceneMapObjStrongLight(LiveActor* pActor);
    void connectToScenePlanet(LiveActor* pActor);
    void connectToSceneEnvironment(LiveActor* pActor);
    void connectToSceneEnvironmentStrongLight(LiveActor* pActor);
    void connectToClippedMapParts(LiveActor* pActor);
    void connectToSceneEnemyDecoration(LiveActor* pActor);
    void connectToSceneEnemyDecorationMovement(NameObj* pObj);
    void connectToSceneEnemyDecorationMovementCalcAnim(NameObj* pObj);
    void connectToSceneItem(LiveActor* pActor);
    void connectToSceneItemStrongLight(LiveActor* pActor);
    void connectToSceneIndirectEnemy(LiveActor* pActor);
    void connectToSceneIndirectNpc(LiveActor* pActor);
    void connectToSceneIndirectMapObj(LiveActor* pActor);
    void connectToSceneIndirectMapObjStrongLight(LiveActor* pActor);
    void connectToSceneScreenEffectMovement(NameObj* pObj);
    void connectToSceneAreaObj(NameObj* pObj);
    void connectToScene3DModelFor2D(LiveActor* pActor);
    void connectToSceneLayout(NameObj* pObj);
    // one unknown function
    void connectToSceneLayoutMovementCalcAnim(NameObj* pObj);
    void connectToSceneLayoutDecoration(NameObj* pObj);
    void connectToSceneLayoutFileSelect(NameObj* pObj);
    void connectToSceneTalkLayout(NameObj* pObj);
    void connectToSceneWipeLayout(NameObj* pObj);
    void connectToSceneLayoutOnPause(NameObj* pObj);
    void connectToSceneLayoutMovement(NameObj* pObj);
    void connectToSceneMirrorMapObj(LiveActor* pActor);
    void connectToSceneMirrorMapObjDecoration(LiveActor* pActor);
    void connectToSceneCamera(NameObj* pObj);
    void connectToSceneNoShadowedMapObj(LiveActor* pActor);
    void connectToSceneNoShadowedMapObjStrongLight(LiveActor* pActor);
    void connectToSceneNoSilhouettedMapObj(LiveActor* pActor);
    void connectToSceneNoSilhouettedMapObjStrongLight(LiveActor* pActor);
    void connectToSceneNoSilhouettedMapObjStrongLightCalcAnim(LiveActor* pActor);
    void connectToSceneNoSilhouettedMapObjWeakLightCalcAnim(LiveActor* pActor);
    void connectToSceneSwitchingScreenEffect(LiveActor* pActor);
    void connectToSceneSky(LiveActor* pActor);
    void connectToSceneAir(LiveActor* pActor);
    void connectToSceneSun(LiveActor* pActor);
    void connectToSceneWorldMapMiniObj(LiveActor* pActor);
    void connectToSceneWorldMapIcon(LiveActor* pActor);
    void connectToSceneSuperDreamer(LiveActor* pActor);

    void requestMovementOn(NameObj* pObj);
    void requestMovementOn(LiveActor* pActor);
    void requestMovementOn(LayoutActor* pLayout);
    void requestMovementOff(NameObj* pObj);
    void requestMovementOff(LiveActor* pActor);
    void requestMovementOff(LayoutActor* pLayout);

    NameObjGroup* joinToNameObjGroup(NameObj* pObj, const char* pGroupName);
    void registerPreDrawFunction(const MR::FunctorBase& rFunctor, int drawType);
    void registerPreDrawFunctionIfNotExist(const MR::FunctorBase& rFunctor, int drawType);

    NameObjAdaptor* createDrawAdaptor(const char* pName, int drawType, const MR::FunctorBase& rFunctor);
    NameObjAdaptor* createAdaptorAndConnectToDrawBloomModel(const char* pName, const MR::FunctorBase& rFunctor);

    void listenNameObjStageSwitchOnAppear(const NameObj* pObj, const StageSwitchCtrl* pStageSwitchCtrl, const MR::FunctorBase& rFunctor);
    void listenNameObjStageSwitchOnOffAppear(const NameObj* pObj, const StageSwitchCtrl* pStageSwitchCtrl, const MR::FunctorBase& rFunctorOn,
                                             const MR::FunctorBase& rFunctorOff);
    void listenNameObjStageSwitchOnA(const NameObj* pObj, const StageSwitchCtrl* pStageSwitchCtrl, const MR::FunctorBase& rFunctor);
    void listenNameObjStageSwitchOnOffA(const NameObj* pObj, const StageSwitchCtrl* pStageSwitchCtrl, const MR::FunctorBase& rFunctorOn,
                                        const MR::FunctorBase& rFunctorOff);
    void listenNameObjStageSwitchOnB(const NameObj* pObj, const StageSwitchCtrl* pStageSwitchCtrl, const MR::FunctorBase& rFunctor);
    void listenNameObjStageSwitchOffB(const NameObj* pObj, const StageSwitchCtrl* pStageSwitchCtrl, const MR::FunctorBase& rFunctor);
    void listenNameObjStageSwitchOnOffB(const NameObj* pObj, const StageSwitchCtrl* pStageSwitchCtrl, const MR::FunctorBase& rFunctorOn,
                                        const MR::FunctorBase& rFunctorOff);

    ResourceHolder* createAndAddResourceHolder(const char* pObjName);
    ResourceHolder* createAndAddResourceHolderStationed(const char* pResource);
    ResTIMG* loadTexFromArcHelper(const char* pArchive, const char* pFile, void* pHolderOut);
    const ResTIMG* getTextureResource(const ResourceHolder*, const char*);
    // more ResourceHolder stuff...
    bool isExistFileInArc(const ResourceHolder* pResArc, const char* pPathFormat, ...);

    JMapInfo* createCsvParser(const ResourceHolder* pResArc, const char* pPathFormat, ...);
    JMapInfo* createCsvParser(const char* pArcName, const char* pPathFormat, ...);
    JMapInfo* tryCreateCsvParser(const ResourceHolder* pResArc, const char* pPathFormat, ...);
    JMapInfo* tryCreateCsvParser(const LiveActor* pActor, const char* pPathFormat, ...);
    JMapInfo* tryCreateCsvParser(const char* pArcName, const char* pPathFormat, ...);

    const JMapInfo* tryCreateStageCsvParser(const char* pStageName, const char* pArcName, const char* pPathFormat, va_list list) NO_INLINE;
    const JMapInfo* tryCreateStageZoneInfoCsvParser(const char* pPathFormat, ...);
    const JMapInfo* tryCreateStageUseResourceCsvParser(const char* pStageName, const char* pPathFormat, ...);

    const JMapInfo* tryCreateWorldMapCsvParser(u32* pWorldId,
                                               const char* pCsvName);  // first argument isn't actually a u32 pointer (it's most likely a pointer to
                                                                       // something world map related), but treating it as one is totally valid

    s32 getCsvDataElementNum(const JMapInfo* pCsvData);
    void getCsvDataStr(const char** pDest, const JMapInfo* pCsvData, const char* pItemName, s32 dataRow);
    void getCsvDataStrOrNULL(const char** pDest, const JMapInfo* pCsvData, const char* pItemName, s32 dataRow);
    void getCsvDataS32(s32* pDest, const JMapInfo* pCsvData, const char* pItemName, s32 dataRow);
    void getCsvDataS16(s16* pDest, const JMapInfo* pCsvData, const char* pItemName, s32 dataRow);
    void getCsvDataU8(u8* pDest, const JMapInfo* pCsvData, const char* pItemName, s32 dataRow);
    void getCsvDataF32(f32* pDest, const JMapInfo* pCsvData, const char* pItemName, s32 dataRow);
    void getCsvDataBool(bool* pDest, const JMapInfo* pCsvData, const char* pItemName, s32 dataRow);
    void getCsvDataVec(Vec* pDest, const JMapInfo* pCsvData, const char* pItemName, s32 dataRow);
    void getCsvDataColor(GXColor* pDest, const JMapInfo* pCsvData, const char* pItemName, s32 dataRow);

    bool hasCsvItem(const JMapInfo*, const char* pItemName);
    bool hasCsvDataItem(const JMapInfo*, const char* pItemName, const char* pDataName);
    /// <summary>
    ///
    /// </summary>
    /// <param name="pDest">The output</param>
    /// <param name="pCsvData">The BCSV to look through</param>
    /// <param name="pItemName">The BCSV Column to find pDataName in</param>
    /// <param name="pDataName">The value to find the first row of inside the pItemName Column</param>
    /// <param name="pElementName">The BCSV Column to get the data from using the index of the row found for pDataName</param>
    JMapInfoIter getCsvDataStrByElement(const char** pDest, const JMapInfo* pCsvData, const char* pItemName, const char* pDataName,
                                        const char* pElementName);
    JMapInfoIter getCsvDataStrByElement(const char** pDest, const JMapInfo* pCsvData, const char* pItemName, s32 data, const char* pElementName);
    JMapInfoIter getCsvDataS32ByElement(s32* pDest, const JMapInfo* pCsvData, const char* pItemName, const char* pDataName,
                                        const char* pElementName);
    // more csv functions...

    JMapInfoIter getCsvDataStrByElement(const char**, const JMapInfo*, const char*, u32, const char*);
    JMapInfoIter getCsvDataU32ByElement(u32*, const JMapInfo*, const char*, const char*, const char*);
    void declarePowerStar(const NameObj* pHost);
    void declarePowerStar(const NameObj* pHost, s32 powerStarId);
    void declarePowerStarCoin100();
    void appearPowerStarContinueCurrentDemo(const NameObj* pHost, const TVec3f& rPosition);
    void appearPowerStarWithoutDemo(const NameObj* pHost);
    void requestAppearPowerStar(const NameObj* pHost);
    void requestAppearPowerStar(const NameObj* pHost, const TVec3f& rPosition);
    void requestAppearPowerStar(const NameObj* pHost, s32 powerStarId, const TVec3f& rPosition);
    void requestAppearPowerStar(const NameObj* pHost, const LiveActor* pFromActor, f32);
    void requestAppearPowerStarFromBelow(const NameObj* pHost);
    void requestAppearPowerStarCoin100();
    bool isEndPowerStarAppearDemo(const NameObj* pHost);

    ModelObj* createPowerStarDemoModel(const NameObj* pHost, const char* pName, MtxPtr pMtx);

    void declareCoin(const NameObj* pHost, s32 numItems);
    s32 getDeclareRemnantCoinCount(const NameObj* pHost);
    s32 getUncollectedCoinCount(const NameObj* pHost);
    void hopCoin(const NameObj* pHost, const TVec3f& rPosition, const TVec3f& rDirection);
    void appearCoinPop(const NameObj* pHost, const TVec3f&, s32 numItems);
    void appearCoinPopToDirection(const NameObj* pHost, const TVec3f& rPosition, const TVec3f& rDirection, s32 numItems);
    bool appearCoinAgainstGravity(const NameObj* pHost, const TVec3f& rPosition, s32 numItems);
    void appearCoinToVelocity(const NameObj* pHost, const TVec3f& rPosition, const TVec3f& rVelocity, s32 numItems);
    void appearCoinCircle(const NameObj* pHost, const TVec3f& rPosition, s32 numItems);

    void declareStarPiece(const NameObj* pHost, s32 numItems);
    s32 getDeclareRemnantStarPieceCount(const NameObj* pHost);
    s32 getDeclareRemnantStarPieceCountNotExist(const NameObj* pHost);
    void declareStarPieceReceiver(const NameObj* pHost, s32);
    void clearGotCountStarPieceReceiver(const NameObj* pHost);
    void hopStarPiece(const NameObj* pHost, const TVec3f&, const TVec3f&);
    bool appearStarPiece(const NameObj* pHost, const TVec3f& rPosition, s32 numItems, f32 radius, f32 height, bool);
    bool appearStarPieceToDirection(const NameObj* pHost, const TVec3f& rPosition, const TVec3f& rDirection, s32 numItems, f32 radius, f32 height,
                                    bool);
    // one unknown StarPiece function
    void initStarPieceGetCSSound();

    BenefitItemOneUp* createKinokoOneUp();
    void appearKinokoOneUp(BenefitItemObj* pKinoko, MtxPtr pMtx);
    void appearKinokoOneUpPop(BenefitItemObj* pKinoko, MtxPtr pMtx);
    void appearKinokoOneUpHop(BenefitItemObj* pKinoko, MtxPtr pMtx);

    BenefitItemLifeUp* createKinokoSuper();
    void appearKinokoSuperPop(BenefitItemLifeUp* pKinoko, MtxPtr pMtx);

    void stopScene(s32 duration);
    void stopSceneForDefaultHit(s32 duration);

    bool tryRumblePad(const void* pHost, const char* pPatternName, s32 wiimote);
    bool tryRumblePadVeryStrong(const void* pHost, s32 wiimote);
    bool tryRumblePadStrong(const void* pHost, s32 wiimote);
    // Provisional names for unnamed SMG2 functions fn_8004DB60 and fn_8004F2F0 (used by ActorItemGenerator).
    bool appearStarPieceToVelocity(const NameObj* pHost, const TVec3f& rPosition, const TVec3f& rVelocity, s32 numItems, bool, bool, bool, f32);
    void getPlayerVelocity(TVec3f* pDest);
    bool tryRumblePadMiddle(const void* pHost, s32 wiimote);
    bool tryRumblePadWeak(const void* pHost, s32 wiimote);
    bool tryRumblePadVeryWeak(const void* pHost, s32 wiimote);
    bool tryRumblePadWeakAndVeryWeak(const void* pHost, s32 wiimote);
    bool tryRumbleDefaultHit(const void* pHost, s32 wiimote);

    void shakeCameraVeryStrong();
    void shakeCameraStrong();
    void shakeCameraNormalStrong();
    void shakeCameraNormal();
    void shakeCameraNormalWeak();
    void shakeCameraWeak();
    void shakeCameraVeryWeak();
    void shakeCameraInfinity(NameObj* pHost, f32, f32);
    void stopShakingCamera(NameObj* pHost);

    bool isName(const NameObj* pObj, const char* pName);
    bool isSame(const NameObj* pObjA, const NameObj* pObjB);

    void suspendAllSceneNameObj();
    void resumeAllSceneNameObj();

    bool registerNamePosLinkObj(const NameObj* pLinkedObj, const JMapInfoIter& rIter, const char* pPosName);
    bool tryRegisterNamePosLinkObj(const NameObj* pLinkedObj, const JMapInfoIter& rIter, const char* pPosName);
    bool findNamePos(const char* pPosName, MtxPtr pMtx);
    bool findNamePos(const char* pPosName, TVec3f* pPosition, TVec3f* pRotation);
    void findNamePosOnGround(const char* pPosName, MtxPtr pMtx);
    bool tryFindNamePos(const char* pPosName, MtxPtr pMtx);
    bool tryFindNamePos(const char* pPosName, TVec3f* pPosition, TVec3f* pRotation);
    void findLinkNamePos(const NameObj* pLinkedObj, const char* pPosName, MtxPtr pMtx);
    bool tryFindLinkNamePos(const NameObj* pLinkedObj, const char* pPosName, TVec3f* pPosition, TVec3f* pRotation);
    bool tryFindLinkNamePos(const NameObj* pLinkedObj, const char* pPosName, MtxPtr pMtx);
};  // namespace MR

// Merged from Petari (core lane): declarations missing here
#include <revolution/gx.h>
#include <revolution/wpad.h>
class BenefitItemObj;
class JMapInfo;
class JMapInfoIter;
class NameObjGroup;
class JUTTexture;
class ResTIMG;

namespace MR {
    bool isJudgedToClipFrustum100m(const TVec3f&, f32);
    void connectToSceneCollisionMapObjMovementCalcAnim(LiveActor*);
    void connectToSceneCollisionEnemyMovement(NameObj*);
    void connectToSceneMapParts(LiveActor*);
    void connectToSceneTalkLayoutNoMovement(NameObj*);
    void connectToSceneLayoutOnPauseNoMovement(NameObj*);
    void connectToSceneLayoutOnPauseMovementCalcAnim(NameObj*);
    void connectToSceneMovie(NameObj*);
    void connectToSceneMirrorMapObjNoMovement(LiveActor*);
    void connectToSceneNoSilhouettedMapObjWeakLightNoMovement(LiveActor*);
    void connectToSceneCrystal(LiveActor*);
    void connectToSceneNormalMapObj(LiveActor*);
    NameObjGroup* joinToMovementOnOffGroup(const char*, NameObj*, u32);
    void onMovementOnOffGroup(const char*);
    void* loadResourceFromArc(const char*, const char*);
    bool isExistResourceInArc(const char*, const char*);
    const ResTIMG* loadTexFromArc(const char*, const ResourceHolder*);
    const ResTIMG* loadTexFromArc(const char*, const char*);
    const ResTIMG* loadTexFromArc(const char*, const char*, ResTIMG*);
    JUTTexture* loadTexFromArc(LiveActor*, const char*, const char*);
    JUTTexture* loadTexFromArc(LiveActor*, const char*);
    bool isStageStateScenarioOpeningCamera();
    bool isStageStatePowerStarAppeared();
    void appearCoinFix(const NameObj*, const TVec3f&, s32);
    void appearKinokoSuper(BenefitItemLifeUp*, MtxPtr, f32);
    bool tryRumblePadVeryStrongLong(const void*, s32);
};  // namespace MR

// Merged from Petari (core lane): overloads missing here


namespace MR {
    void listenNameObjStageSwitchOnOffAppear(const NameObj*, const StageSwitchCtrl*, const MR::FunctorBase&, const MR::FunctorBase&);
    void listenNameObjStageSwitchOnOffA(const NameObj*, const StageSwitchCtrl*, const MR::FunctorBase&, const MR::FunctorBase&);
    void listenNameObjStageSwitchOnOffB(const NameObj*, const StageSwitchCtrl*, const MR::FunctorBase&, const MR::FunctorBase&);
    bool appearStarPieceToDirection(const NameObj*, const TVec3f&, const TVec3f&, s32, f32, f32, bool);
};  // namespace MR
