#pragma once

#include "Game/Util/EventUtil.hpp"

#include "Game/Util/JMapInfo.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>
#include <JSystem/JGeometry/TVec.hpp>

class NameObj;
class GameSystemSceneController;

namespace MR {
    bool isStageMarioFaceShipNormal();
    s32 getCurrentScenarioNo();
    s32 getCurrentSelectedScenarioNo();
    void setCurrentScenarioNo(s32, s32);
    bool isScenarioDecided();
    GameSystemSceneController* getCurrentGameSystemSceneController();

    const char* getCurrentStageName();
    void getGhostData(const void**, u32*, const char*, int);
    bool isEqualSceneName(const char*);
    bool isEqualStageName(const char*);
    bool isValidZoneName(const char*);
    bool isStageJungleGliderGalaxyExplainDemo();

    bool isPlacementLocalStage();

    const TPos3f& getZonePlacementMtx(const JMapInfoIter&);

    void getCameraRailInfo(JMapInfoIter*, const JMapInfo**, s32, s32);

    void getRailInfo(JMapInfoIter*, const JMapInfo**, const JMapInfoIter&);

    s32 getChildObjNum(const JMapInfoIter&);
    void getChildObjName(const char**, const JMapInfoIter&, int);
    void initChildObj(NameObj*, const JMapInfoIter&, int);

    s32 getCurrentStagePowerStarNum();
    bool isProductGameScene();
};  // namespace MR

// Merged from Petari (core lane): declarations missing here
#include <revolution/types.h>
class JKRMemArchive;
class JMapIdInfo;
class JMapInfo;
class JMapInfoIter;
class JMapLinkInfo;

namespace MR {
    bool isStageBeginPrologueEvent();
    bool isStageNoPauseMenuStars();
    bool isStageBeginFadeWipe();
    bool isStageBeginTitleWipe();
    bool isStageBeginWithoutWipe();
    bool isStageDisablePauseMenu();
    bool isStageAstroLocation();
    bool isStageSwimAngleLimit();
    bool isStageStarPieceFollowGroupLimit();
    bool isStageFileSelect();
    bool isStageMarioFaceShip();
    bool isStageMarioFaceShipOrWorldMap();
    bool isStageStoryBook();
    bool isStageKoopaVs();
    bool isStageKoopaVs1();
    bool isStageKoopaVs2();
    bool isStageKoopaVs3();
    bool isStageScoreAttack();
    bool isStagePlayStarChance(s32);
    bool isStageEpilogueDemo();
    bool isStageStaffRoll();
    bool isBeginScenarioStarter();
    bool isStageSuddenDeathDodoryu();
    void setInitializeStatePlacementPlayer();
    void setInitializeStatePlacementHighPriority();
    void setInitializeStatePlacement();
    void setInitializeStateAfterPlacement();
    bool isInitializeStateEnd();
    bool isInitializeStatePlacementSomething();
    void stopSceneForScenarioOpeningCamera();
    void playSceneForScenarioOpeningCamera();
    const JMapIdInfo& getCurrentMarioStartIdInfo() NO_INLINE;
    s32 getStartPosNum();
    s32 getCurrentStartZoneId();
    const JMapIdInfo& getInitializeStartIdInfo();
    JKRMemArchive* getStageArchive(const char*);
    s32 getGeneralPosNum();
    void getGeneralPosData(const char**, TVec3f*, TVec3f*, JMapLinkInfo**, int);
    const char* getAppearPowerStarObjName(s32);
    s32 getCurrentStageNormalScenarioNum();
    const char* getZoneNameFromZoneId(s32);
    s32 getPlacedHiddenStarScenarioNo(const char*, s32);
    bool getNextLinkRailInfo(JMapInfoIter*, const JMapInfo**, const JMapInfoIter&);
    s32 getCurrentStartCameraId();
    void getStartCameraIdInfoFromStartDataIndex(JMapIdInfo*, int);
    s32 getPlacedRailNum(s32);
    bool getCameraRailInfoFromRailDataIndex(JMapInfoIter*, const JMapInfo**, int, s32);
    void getStageCameraData(void**, s32*, s32);
    void getCurrentScenarioStartAnimCameraData(void**, s32*);
    s32 getPlacedZoneId(const JMapInfoIter&);
    const char* getJapaneseObjectName(const char*);
    void setCurrentPlacementZoneId(s32);
    void clearCurrentPlacementZoneId();
    s32 getCurrentPlacementZoneId();
    const char* getCurrentPlacementZoneName();
};  // namespace MR

// Merged from Petari (core lane): overloads missing here


namespace MR {
    s32 getZoneNum();
    const TPos3f* getZonePlacementMtx(s32);
    s32 getCurrentOrSelectedScenarioNo();
};  // namespace MR

// core (h1): SMG2 SceneUtil functions named from the target (fn_XXXX -> names pending namefn --write)
namespace MR {
    bool isStageMarioFaceShipAfterFinalKoopa();
    bool isStageMarioFaceShipAfterGrandStar();
    bool isStagePeachCastleScenario2();
    bool isStageWorldMap();
    bool isStageMarioFaceShipNormal();
    bool isStageBossParade();
    bool isStageDimensionBigCastle(const char*);
    bool isStagePurpleCoinsMoreThan100(const char*);
    bool isStageTimerSwitchingPlatformPowerStarAppeared();
    bool isStageScenarioEventOpen(const char*, s32);
    bool isBigTreeOrCloudGardenPowerStarSuccess();
    bool isStageGlider();
    bool isStageSpecialSceneGalaxy();
    bool isNextStageFileSelect();
    bool isValidStageName(const char*);
    s32 getCurrentMarioStartId();
    bool getRestartPosData(TVec3f*, TVec3f*, const JMapInfoIter&);
    const char* getChildObjJapaneseName(const JMapInfoIter&, int);
    void getZoneStageArchiveResource(void**, s32*, const char*, s32);
    bool tryGetUseResourceArchiveResource(void**, s32*, const char*);   // fn_80057950 (2nd arg unused)
    s32 getZoneIdFromZoneName(const char*);
    const char* getJapaneseObjectNameOrNull(const char*);
};  // namespace MR
namespace MR {
    bool isGhostLuigiOpenInCurrentStage();
}

namespace MR { bool isStageWorldMap(); }

namespace MR {
    // SMG2 fn_80056EF0 (core4 name): paired-star check of the Big Tree / Cloud Garden Galaxy events (stage, scenario).
    bool isBigTreeCloudGardenPairedStarUncollected(const char* pStageName, s32 scenarioNo);
}
