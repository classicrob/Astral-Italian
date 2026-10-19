#include "Game/Util/SceneUtil.hpp"
#include "Game/Util.hpp"
#include "Game/NameObj/NameObjFinder.hpp"
#include "Game/Scene/PlacementStateChecker.hpp"
#include "Game/Scene/SceneNameObjMovementController.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/Scene/ScenePlayingResult.hpp"
#include "Game/Scene/StageDataHolder.hpp"
#include "Game/Scene/StageFileLoader.hpp"
#include "Game/System/GalaxyStatusAccessor.hpp"
#include "Game/System/GameDataFunction.hpp"
#include "Game/System/GameSystem.hpp"
#include "Game/System/GameSequenceFunction.hpp"
#include "Game/System/GameSystemSceneController.hpp"
#include "Game/System/PlacedHiddenStarScenarioTable.hpp"
#include "Game/Util/FileUtil.hpp"
#include "Game/Util/JMapIdInfo.hpp"
#include "Game/Util/JMapLinkInfo.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/SequenceUtil.hpp"
#include "Game/Util/SingletonHolder.hpp"
#include "Game/Util/StringUtil.hpp"
#include <cstdio>
#include <JSystem/JKernel/JKRArchive.hpp>
#include <cstring>

namespace {
    const JMapIdInfo cInitializeStartIdInfo(0, 0);

    void getRailInfoFromRailId(JMapInfoIter* pIter, const JMapInfo** ppInfo, const StageDataHolder* pHolder, int railId) {
        *pIter = pHolder->getCommonPathPointInfo(ppInfo, railId);
    }


    bool isInitializeState(SceneInitializeState state) {
        return SingletonHolder< GameSystem >::get()->mSceneController->isSceneInitializeState(state);
    }
};  // namespace

namespace MR {
    s32 getCurrentScenarioNo() {
        return SingletonHolder< GameSystem >::get()->mSceneController->getCurrentScenarioNo();
    }

    // NONMATCHING: equivalent selected-scenario guard branch arrangement
    s32 getCurrentOrSelectedScenarioNo() {
        s32 selectedScenarioNo = SingletonHolder< GameSystem >::get()->mSceneController->getCurrentSelectedScenarioNo();
        if (selectedScenarioNo == -1) {
            return SingletonHolder< GameSystem >::get()->mSceneController->getCurrentScenarioNo();
        }

        return selectedScenarioNo;
    }

    void setCurrentScenarioNo(s32 scenarioNo, s32 selectedScenarioNo) {
        return SingletonHolder< GameSystem >::get()->mSceneController->setCurrentScenarioNo(scenarioNo, selectedScenarioNo);
    }

    bool isScenarioDecided() {
        return SingletonHolder< GameSystem >::get()->mSceneController->isScenarioDecided();
    }

    GameSystemSceneController* getCurrentGameSystemSceneController() {
        return SingletonHolder< GameSystem >::get()->mSceneController;
    }

    const char* getCurrentStageName() {
        return SingletonHolder< GameSystem >::get()->mSceneController->mCurrSceneControlInfo.mStage;
    }

    bool isEqualSceneName(const char* pSceneName) {
        return isEqualStringCase(SingletonHolder< GameSystem >::get()->mSceneController->mCurrSceneControlInfo.mScene, pSceneName);
    }

    bool isEqualStageName(const char* pStageName) {
        const char* pStage = SingletonHolder< GameSystem >::get()->mSceneController->mCurrSceneControlInfo.mStage;

        if (pStage == nullptr) {
            return false;
        }

        return isEqualStringCase(pStage, pStageName);
    }

    bool isValidZoneName(const char* pName) {
        const char* pMatch = strstr(pName, "Zone");
        bool result = false;
        if (pMatch != nullptr && *(pMatch + strlen("Zone")) == '\0') {
            result = true;
        }

        return result;
    }

    bool isStageNoPauseMenuStars() {
        if (isEqualStageName("PeachCastleGalaxy") && getCurrentScenarioNo() == 1) {
            return true;
        }

        if (isEqualStageName("PeachCastleGalaxy") && getCurrentScenarioNo() == 3) {
            return true;
        }

        return false;
    }

    bool isStageMarioFaceShipAfterFinalKoopa() {
        if (isEqualStageName("MarioFaceShipGalaxy") && getCurrentScenarioNo() == 3) {
            return true;
        }

        return false;
    }

    bool isStageMarioFaceShipAfterGrandStar() {
        if (isEqualStageName("MarioFaceShipGalaxy") && getCurrentScenarioNo() == 4) {
            return true;
        }

        return false;
    }

    bool isStagePeachCastleScenario2() {
        if (isEqualStageName("PeachCastleGalaxy") && getCurrentScenarioNo() == 2) {
            return true;
        }

        return false;
    }

    bool isStageEpilogueDemo() {
        if (isStagePeachCastleScenario2()) {
            return true;
        }

        return isEqualStageName("ChildRoomGalaxy");
    }

    bool isStageStaffRoll() {
        return isEqualStageName("StaffRollGalaxy");
    }

    bool isStageStarPieceFollowGroupLimit() {
        return isEqualStageName("EggStarGalaxy") && getCurrentScenarioNo() == 2;
    }

    bool isStageFileSelect() {
        return isEqualStageName("FileSelect");
    }

    bool isBeginScenarioStarter() {
        if (GameSequenceFunction::hasRetryGalaxySequence()) {
            return false;
        }

        return NameObjFinder::find("シナリオスターター");
    }

    bool isStageSuddenDeathDodoryu() {
        return isEqualStageName("CosmosGardenGalaxy") && getCurrentScenarioNo() == 4;
    }

    // NONMATCHING: register allocation of the flag locals only (structure matches the target)
    bool isStageScoreAttack() {
        bool match = false;
        bool result = true;
        bool isHoneyOrMokumoku = true;
        bool isTwister = true;
        if (isEqualStageName("HoneyBeeVillageGalaxy") && getCurrentScenarioNo() == 2) {
            match = true;
        }

        if (!match) {
            match = false;
            if (isEqualStageName("MokumokuValleyGalaxy") && getCurrentScenarioNo() == 2) {
                match = true;
            }

            if (!match) {
                isHoneyOrMokumoku = false;
            }
        }

        if (!isHoneyOrMokumoku) {
            match = false;
            if (isEqualStageName("TwisterTowerGalaxy") && getCurrentScenarioNo() == 3) {
                match = true;
            }

            if (!match) {
                isTwister = false;
            }
        }

        if (!isTwister) {
            match = false;
            if (isEqualStageName("UnderGroundDangeonGalaxy") && getCurrentScenarioNo() == 2) {
                match = true;
            }

            if (!match) {
                result = false;
            }
        }

        return result;
    }

    bool isStageMarioFaceShip() {
        return isEqualStageName("MarioFaceShipGalaxy");
    }

    // NONMATCHING: string pool offsets differ (target pool has an unused OceanRingGalaxy literal at +0x50)
    bool isStageWorldMap() {
        if (isEqualStageName("WorldMap01Galaxy") ||
               isEqualStageName("WorldMap02Galaxy") ||
               isEqualStageName("WorldMap03Galaxy") ||
               isEqualStageName("WorldMap04Galaxy") ||
               isEqualStageName("WorldMap05Galaxy") ||
               isEqualStageName("WorldMap06Galaxy") ||
               isEqualStageName("WorldMap07Galaxy") ||
               isEqualStageName("WorldMap08Galaxy")) return true;
        return false;
    }

    bool isStageMarioFaceShipOrWorldMap() {
        return isEqualStageName("MarioFaceShipGalaxy") ||
               isStageWorldMap();
    }

    // NONMATCHING: same literal-pool sharing difference as isStageStoryBook
    bool isStageDisablePauseMenu() {
        if (isEqualStageName("FileSelect")) {
            return true;
        }

        if (isStageEpilogueDemo()) {
            return true;
        }

        if (isEqualStageName("MarioFaceShipGalaxy") && getCurrentScenarioNo() == 3) {
            return true;
        }

        if (isStageStaffRoll()) return true;
        return false;
    }

    // NONMATCHING: target reloads each literal address (lis/addi per use); ours shares one pool base
    bool isStageStoryBook() {
        if (isEqualStageName("PeachCastleGalaxy") && getCurrentScenarioNo() == 1) {
            return true;
        }

        if (isEqualStageName("PeachCastleGalaxy") && getCurrentScenarioNo() == 2) {
            return true;
        }

        if (isEqualStageName("PeachCastleGalaxy") && getCurrentScenarioNo() == 3) {
            return true;
        }

        if (isEqualStageName("MarioFaceShipGalaxy") && getCurrentScenarioNo() == 3) {
            return true;
        }

        if (isStageStaffRoll()) return true;
        return false;
    }

    bool isStageMarioFaceShipNormal() {
        return isEqualStageName("MarioFaceShipGalaxy") && getCurrentScenarioNo() == 2;
    }

    bool isStageBossParade() {
        return isEqualStringCase(getCurrentStageName(), "KingOfKingsGalaxy");
    }

    bool isStageDimensionBigCastle(const char* pName) {
        return isEqualStringCase(pName, "DimensionBigCastleGalaxy");
    }

    bool isStagePurpleCoinsMoreThan100(const char* pName) {
        if (pName == nullptr) {
            return false;
        }

        return isEqualString("BigTree2Galaxy", pName) ||
               isEqualString("TamakoroSliderGalaxy", pName);
    }

    bool isStagePlayStarChance(s32 scenarioNo) {
        if (isPowerStarWithDeclarer("パマタリアンハンター", scenarioNo)) {
            return false;
        }

        if (isPowerStarWithDeclarer("花さか監視[パワースター]", scenarioNo)) {
            return false;
        }

        if (isEqualStageName("MemoryRoadGalaxy")) {
            return false;
        }

        if (isEqualStageName("OmoteuLandGalaxy")) {
            if (scenarioNo == 2) {
                return false;
            }

            if (scenarioNo == 3) {
                return false;
            }
        }

        return true;
    }

    // NONMATCHING: equivalent guard branch arrangement
    bool isTimerSwitchingPlatformGalaxyPowerStarAppeared() {
        if (!isStageStatePowerStarAppeared()) return false;
        return isEqualStageName("TimerSwitchingPlatformGalaxy");
    }

    bool isBigTreeCloudGardenPairedStarUncollected(const char* pStageName, s32 scenarioNo) {
        if (isEqualString("BigTree2Galaxy", pStageName) && scenarioNo == 1) {
            return !isOnGameEventFlagPowerStarSuccess("CloudGardenGalaxy", 1) && !isOnGameEventFlagPowerStarSuccess("CloudGardenGalaxy", 3);
        }
        if (isEqualString("CloudGardenGalaxy", pStageName) && scenarioNo == 1) {
            return !isOnGameEventFlagPowerStarSuccess("BigTree2Galaxy", 1) && !isOnGameEventFlagPowerStarSuccess("CloudGardenGalaxy", 3);
        }
        if (isEqualString("CloudGardenGalaxy", pStageName) && scenarioNo == 3) {
            return !isOnGameEventFlagPowerStarSuccess("BigTree2Galaxy", 1) && !isOnGameEventFlagPowerStarSuccess("CloudGardenGalaxy", 1);
        }
        return false;
    }

    bool isBigTreeOrCloudGardenPowerStarSuccess() {
        return isOnGameEventFlagPowerStarSuccess("BigTree2Galaxy", 1) ||
               isOnGameEventFlagPowerStarSuccess("CloudGardenGalaxy", 1) ||
               isOnGameEventFlagPowerStarSuccess("CloudGardenGalaxy", 3);
    }

    bool isStageGlider() {
        return isEqualStageName("JungleGliderGalaxy") ||
               isEqualStageName("ChallengeGliderGalaxy");
    }

    bool isStageJungleGliderGalaxyExplainDemo() {
        return isEqualStageName("JungleGliderGalaxy") && getCurrentScenarioNo() == 1;
    }

    bool isStageSpecialSceneGalaxy() {
        if (isStageWorldMap()) {
            return true;
        }

        if (isEqualStageName("RedBlueExGalaxy")) {
            return true;
        }

        if (isEqualStageName("JungleGliderGalaxy")) {
            return true;
        }

        if (isEqualStageName("VsKoopaJrLv1Galaxy")) {
            return true;
        }

        if (isEqualStageName("GoroRockGalaxy")) {
            return true;
        }

        if (isEqualStageName("StarCreekGalaxy")) {
            return true;
        }

        if (isEqualStageName("GhostConveyorGalaxy")) {
            return true;
        }

        if (isEqualStageName("MysteryCandyGalaxy")) {
            return true;
        }

        if (isEqualStageName("KoopaJrLv2Galaxy")) {
            return true;
        }

        if (isEqualStageName("WanwanFactoryGalaxy")) {
            return true;
        }

        if (isEqualStageName("ThunderFleetGalaxy")) {
            return true;
        }

        if (isEqualStageName("VsKoopaJrLv3Galaxy")) {
            return true;
        }

        if (isEqualStageName("KachikochiLavaGalaxy")) {
            return true;
        }

        if (isEqualStageName("ChallengeGliderGalaxy")) {
            return true;
        }

        if (isEqualStageName("MemoryRoadGalaxy")) {
            return true;
        }

        if (isEqualStageName("TwisterTowerGalaxy")) {
            return true;
        }

        if (isEqualStageName("MagicGalaxy")) {
            return true;
        }

        if (isEqualStageName("MarioOnMarioGalaxy")) {
            return true;
        }

        if (isEqualStageName("SwitchMoveBlockGalaxy")) {
            return true;
        }

        if (isEqualStageName("DigMineGalaxy")) {
            return true;
        }

        return false;
    }

    bool isStageKoopaVs3() {
        return isEqualStageName("VsKoopaLv3Galaxy") && getCurrentScenarioNo() == 1;
    }

    bool isStageKoopaVs() {
        if (isEqualStageName("VsKoopaLv1Galaxy") ||
               isEqualStageName("VsKoopaGravityGalaxy") ||
               isEqualStageName("VsKoopaLv3Galaxy")) return true;
        return false;
    }

    bool isUseStageAudioEffect() {
        if (isStageWorldMap()) return true;
        if (isEqualStageName("RedBlueExGalaxy") ||
            isEqualStageName("JungleGliderGalaxy") ||
            isEqualStageName("VsKoopaJrLv1Galaxy") ||
            isEqualStageName("GoroRockGalaxy") ||
            isEqualStageName("StarCreekGalaxy") ||
            isEqualStageName("GhostConveyorGalaxy") ||
            isEqualStageName("MysteryCandyGalaxy") ||
            isEqualStageName("KoopaJrLv2Galaxy") ||
            isEqualStageName("WanwanFactoryGalaxy") ||
            isEqualStageName("ThunderFleetGalaxy") ||
            isEqualStageName("VsKoopaJrLv3Galaxy") ||
            isEqualStageName("KachikochiLavaGalaxy") ||
            isEqualStageName("ChallengeGliderGalaxy") ||
            isEqualStageName("MemoryRoadGalaxy") ||
            isEqualStageName("TwisterTowerGalaxy") ||
            isEqualStageName("MagicGalaxy") ||
            isEqualStageName("MarioOnMarioGalaxy") ||
            isEqualStageName("SwitchMoveBlockGalaxy") ||
            isEqualStageName("DigMineGalaxy")) return true;
        return false;
    }

    bool isNextStageFileSelect() {
        return isEqualString(SingletonHolder< GameSystem >::get()->mSceneController->getNextStageName(), "FileSelect");
    }

    bool isPlayerArchiveChangePending() {
        return SingletonHolder<GameSystem>::get()->mSceneController->mIsFileCacheHeapMissing;
    }

    void setInitializeStatePlacementPlayer() {
        SingletonHolder< GameSystem >::get()->mSceneController->setSceneInitializeState(SceneInitializeState_PlacementPlayer);
    }

    void setInitializeStatePlacementHighPriority() {
        SingletonHolder< GameSystem >::get()->mSceneController->setSceneInitializeState(SceneInitializeState_PlacementHighPriority);
    }

    void setInitializeStatePlacement() {
        SingletonHolder< GameSystem >::get()->mSceneController->setSceneInitializeState(SceneInitializeState_Placement);
    }

    void setInitializeStateAfterPlacement() {
        SingletonHolder< GameSystem >::get()->mSceneController->setSceneInitializeState(SceneInitializeState_AfterPlacement);
    }

    bool isInitializeStateEnd() {
        return SingletonHolder< GameSystem >::get()->mSceneController->isSceneInitializeState(SceneInitializeState_End);
    }

    bool isInitializeStatePlacementSomething() {
        return (::isInitializeState(SceneInitializeState_PlacementPlayer) || ::isInitializeState(SceneInitializeState_PlacementHighPriority)) ||
               ::isInitializeState(SceneInitializeState_Placement);
    }

    void stopSceneForScenarioOpeningCamera() {
        getSceneNameObjMovementController()->requestStopSceneFor(MovementControlType_4, nullptr);
    }

    void playSceneForScenarioOpeningCamera() {
        getSceneNameObjMovementController()->requestPlaySceneFor(MovementControlType_4, nullptr);
    }

    const JMapIdInfo& getCurrentMarioStartIdInfo() {
        return *SingletonHolder< GameSystem >::get()->mSceneController->mCurrSceneControlInfo.mStartIdInfo;
    }

    s32 getCurrentMarioStartId() {
        return getCurrentMarioStartIdInfo()._0;
    }

    s32 getStartPosNum() {
        return getStageDataHolder()->getStartPosNum();
    }

    s32 getCurrentStartZoneId() {
        return getStageDataHolder()->getCurrentStartZoneId();
    }

    const JMapIdInfo& getInitializeStartIdInfo() {
        return ::cInitializeStartIdInfo;
    }

    s32 getGeneralPosNum() {
        return getStageDataHolder()->getGeneralPosNum();
    }

    void getGeneralPosData(const char** ppName, TVec3f* pPosition, TVec3f* pRotation, JMapLinkInfo** ppLinkInfo, int index) {
        JMapInfoIter iter = getStageDataHolder()->getGeneralPosInfoFromDataIndex(index);
        iter.getValue("PosName", ppName);
        getJMapInfoTrans(iter, pPosition);
        getJMapInfoRotate(iter, pRotation);
        *ppLinkInfo = new JMapLinkInfo(iter, false);
    }

    bool getRestartPosData(TVec3f* pTrans, TVec3f* pRot, const JMapInfoIter& rIter) {
        s32 childObjNum = getStageDataHolder()->findPlacedStageDataHolder(rIter)->getChildObjNum(rIter);
        for (s32 i = 0; i < childObjNum; i++) {
            JMapInfoIter childIter = getStageDataHolder()->findPlacedStageDataHolder(rIter)->getChildObjInfoFromDataIndex(rIter, i);
            if (isObjectName(childIter, "GeneralRestartPos")) {
                s32 arg0;
                getJMapInfoArg0WithInit(childIter, &arg0);
                JMapIdInfo restartInfo(arg0, childIter);
                if (*getRestartMarioNo() == restartInfo) {
                    if (pTrans != nullptr) {
                        getJMapInfoTrans(childIter, pTrans);
                    }

                    if (pRot != nullptr) {
                        getJMapInfoRotate(childIter, pRot);
                    }

                    return true;
                }
            }
        }

        return false;
    }

    s32 getChildObjNum(const JMapInfoIter& rIter) {
        return getStageDataHolder()->findPlacedStageDataHolder(rIter)->getChildObjNum(rIter);
    }

    void getChildObjName(const char** ppName, const JMapInfoIter& rIter, int index) {
        JMapInfoIter childIter = getStageDataHolder()->findPlacedStageDataHolder(rIter)->getChildObjInfoFromDataIndex(rIter, index);
        getObjectName(ppName, childIter);
    }

    const char* getChildObjJapaneseName(const JMapInfoIter& rIter, int index) {
        const char* pName = nullptr;
        getChildObjName(&pName, rIter, index);
        const char* fallback = pName;
        const char* pJapaneseName = getStageDataHolder()->getJapaneseObjectName(pName);
        if (pJapaneseName != nullptr) {
            return pJapaneseName;
        }

        return fallback;
    }

    bool tryGetUseResourceArchiveResource(void** out, s32*, const char* name) {
        *out = getSceneObj<StageDataHolder>(SceneObj_StageDataHolder)->getUseResourceArchiveResource(name);
        return *out != nullptr;
    }

    void getZoneStageArchiveResource(void** ppData, s32* pSize, const char* pName, s32 zoneId) {
        StageDataHolder* pHolder;
        if (zoneId == 0) {
            pHolder = getSceneObj< StageDataHolder >(SceneObj_StageDataHolder);
        } else {
            pHolder = getStageDataHolder()->getStageDataHolderFromZoneId(zoneId);
        }

        *ppData = pHolder->getStageArchiveResource(pName);
        if (pSize != nullptr) {
            if (*ppData != nullptr) {
                *pSize = pHolder->getStageArchiveResourceSize(*ppData);
            } else {
                *pSize = 0;
            }
        }
    }

    void getGhostData(const void** out, u32* size, const char* name, int zone) {
        StageDataHolder* holder;
        if (zone == 0) holder = getSceneObj<StageDataHolder>(SceneObj_StageDataHolder);
        else holder = getStageDataHolder()->getStageDataHolderFromZoneId(zone);
        JKRArchive* archive = holder->getGhostArchive();
        if (archive == nullptr) *out = nullptr;
        else *out = archive->getResource(QUESTIONMARK_MAGIC, name);
        if (size) {
            if (*out) *size = archive->getResSize(*out);
            else *size = 0;
        }
    }

    void initChildObj(NameObj* pObj, const JMapInfoIter& rIter, int index) {
        JMapInfoIter childIter = getStageDataHolder()->findPlacedStageDataHolder(rIter)->getChildObjInfoFromDataIndex(rIter, index);
        initialize(pObj, childIter);
    }

    const char* getAppearPowerStarObjName(s32 scenarioNo) {
        return makeCurrentGalaxyStatusAccessor().getAppearPowerStarObjName(scenarioNo);
    }

    s32 getCurrentStagePowerStarNum() {
        return makeCurrentGalaxyStatusAccessor().getPowerStarNum();
    }

    s32 getZoneNum() {
        return makeCurrentGalaxyStatusAccessor().getZoneNum();
    }

    const char* getZoneNameFromZoneId(s32 zoneId) {
        return makeCurrentGalaxyStatusAccessor().getZoneName(zoneId);
    }

    s32 getZoneIdFromZoneName(const char* pName) {
        return makeCurrentGalaxyStatusAccessor().getZoneId(pName);
    }

    s32 getPlacedHiddenStarScenarioNo(const char* pStageName, s32 starId) {
        GalaxyStatusAccessor accessor = makeGalaxyStatusAccessor(pStageName);
        s32 scenarioCount = accessor.getNormalPowerStarNumWithoutComet();
        if (accessor.isGreenStar(starId) && accessor.hasCometScenario()) {
            scenarioCount++;
        }

        for (s32 i = 1; i <= scenarioCount; i++) {
            if (accessor.isExistPowerStarAtScenario(i, starId)) {
                return i;
            }
        }

        return 1;
    }

    void getRailInfo(JMapInfoIter* pRailIter, const JMapInfo** ppPointInfo, const JMapInfoIter& rIter) {
        s32 railId = -1;
        getJMapInfoRailId(rIter, &railId);
        ::getRailInfoFromRailId(pRailIter, ppPointInfo, getStageDataHolder()->findPlacedStageDataHolder(rIter), railId);
    }

    bool getNextLinkRailInfo(JMapInfoIter* pRailIter, const JMapInfo** ppPointInfo, const JMapInfoIter& rIter) {
        s32 railId;
        getJMapInfoNextLinkRailID(rIter, &railId);
        if (railId == -1) {
            return false;
        }

        const StageDataHolder* pHolder = getStageDataHolder()->findPlacedStageDataHolder(rIter);
        ::getRailInfoFromRailId(pRailIter, ppPointInfo, pHolder, railId);
        return true;
    }

    s32 getCurrentStartCameraId() {
        return getStageDataHolder()->getCurrentStartCameraId();
    }

    void getStartCameraIdInfoFromStartDataIndex(JMapIdInfo* pInfo, int index) {
        getStageDataHolder()->getStartCameraIdInfoFromStartDataIndex(pInfo, index);
    }

    s32 getPlacedRailNum(s32 zoneId) {
        if (getStageDataHolder()->isPlacedZone(zoneId)) {
            return getStageDataHolder()->getStageDataHolderFromZoneId(zoneId)->getCommonPathInfoElementNum();
        }

        return 0;
    }

    void getCameraRailInfo(JMapInfoIter* pRailIter, const JMapInfo** ppPointInfo, s32 railId, s32 zoneId) {
        ::getRailInfoFromRailId(pRailIter, ppPointInfo, getStageDataHolder()->getStageDataHolderFromZoneId(zoneId), railId);
    }

    bool getCameraRailInfoFromRailDataIndex(JMapInfoIter* pRailIter, const JMapInfo** ppPointInfo, int index, s32 zoneId) {
        *pRailIter = getStageDataHolder()->getStageDataHolderFromZoneId(zoneId)->getCommonPathPointInfoFromRailDataIndex(ppPointInfo, index);
        return isEqualRailUsage(*pRailIter, "Camera");
    }

    void getStageCameraData(void** ppData, s32* pSize, s32 zoneId) {
        if (!getStageDataHolder()->isPlacedZone(zoneId)) {
            *ppData = nullptr;
            *pSize = 0;
        } else {
            getZoneStageArchiveResource(ppData, pSize, "CameraParam.bcam", zoneId);
        }
    }

    void getCurrentScenarioStartAnimCameraData(void** ppData, s32* pSize) {
        char fileName[64];
        snprintf(fileName, sizeof(fileName), "StartScenario%d.canm", getCurrentScenarioNo());
        getZoneStageArchiveResource(ppData, pSize, fileName, 0);
    }

    void getDreamerPadData(void** out, s32* size) {
        char name[64];
        snprintf(name, sizeof(name), "Dreamer%d.pad", getCurrentMarioStartIdInfo()._0);
        getGhostData(reinterpret_cast<const void**>(out), reinterpret_cast<u32*>(size), name, 0);
    }

    bool isPlacementLocalStage() {
        return getStageDataHolder()->mStageDataHolderCount > 0;
    }

    s32 getPlacedZoneId(const JMapInfoIter& rIter) {
        return getStageDataHolder()->findPlacedStageDataHolder(rIter)->mZoneID;
    }

    const TPos3f& getZonePlacementMtx(const JMapInfoIter& rIter) {
        return getStageDataHolder()->findPlacedStageDataHolder(rIter)->mPlacementMtx;
    }

    const TPos3f* getZonePlacementMtx(s32 zoneId) {
        return &getStageDataHolder()->getStageDataHolderFromZoneId(zoneId)->mPlacementMtx;
    }

    const char* getJapaneseObjectNameOrNull(const char* pName) {
        return getStageDataHolder()->getJapaneseObjectName(pName);
    }

    const char* getJapaneseObjectName(const char* pName) {
        const char* pJapaneseName = getStageDataHolder()->getJapaneseObjectName(pName);
        if (pJapaneseName != nullptr) {
            return pJapaneseName;
        }

        return pName;
    }

    void setCurrentPlacementZoneId(s32 zoneId) {
        getPlacementStateChecker()->setCurrentPlacementZoneId(zoneId);
    }

    void clearCurrentPlacementZoneId() {
        getPlacementStateChecker()->clearCurrentPlacementZoneId();
    }

    s32 getCurrentPlacementZoneId() {
        return getPlacementStateChecker()->getCurrentPlacementZoneId();
    }

    bool isValidStageName(const char* pName) {
        return true;
    }

    bool isProductGameScene() {
        return true;
    }
};  // namespace MR
