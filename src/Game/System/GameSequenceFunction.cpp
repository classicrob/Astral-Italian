#ifdef GALAXY_NATIVE
#include "launch_stage.hpp"
#include <cstdio>
#include <cstdlib>
#endif
#include "Game/System/GameSequenceFunction.hpp"
#include "Game/Util.hpp"
#include "Game/NWC24/NWC24Messenger.hpp"
#include "Game/Scene/GameSceneFunction.hpp"
#include "Game/System/GalaxyStatusAccessor.hpp"
#include "Game/System/GameDataConst.hpp"
#include "Game/System/GameDataFunction.hpp"
#include "Game/System/GameSequence.hpp"
#include "Game/System/GameSequenceDirector.hpp"
#include "Game/System/GameSequenceInGame.hpp"
#include "Game/System/GameSequenceInStage.hpp"
#include "Game/System/GameSequenceMarioFaceShip.hpp"
#include "Game/System/GameSequenceMarioFaceShipEvent.hpp"
#include "Game/System/GameSystem.hpp"
#include "Game/System/GameSystemFunction.hpp"
#include "Game/System/GameSystemSceneController.hpp"
#include "Game/System/PlayResultInStageHolder.hpp"
#include "Game/System/PlayResultMarioFaceShipHolder.hpp"
#include "Game/System/PlayResultWorldMapHolder.hpp"
#include "Game/System/SaveDataHandleSequence.hpp"
#include "Game/System/ScenarioDataParser.hpp"
#include "Game/System/UserFile.hpp"
#include "Game/Util/JMapIdInfo.hpp"
#include "Game/Util/SingletonHolder.hpp"
#include "Game/Util/StringUtil.hpp"
#include <algorithm>
#include <cstdio>

#define GAME_SYSTEM (SingletonHolder< GameSystem >::get())
#define IN_STAGE_HOLDER (GameSequenceFunction::getGameSequenceInGame()->getPlayResultInStageHolder())
#define WORLD_MAP_HOLDER (GameSequenceFunction::getGameSequenceInGame()->getPlayResultWorldMapHolder())
#define FACE_SHIP_HOLDER (GameSequenceFunction::getGameSequenceInGame()->getPlayResultMarioFaceShipHolder())

namespace {
    // One galaxy that owns a comet, collected by updateGalaxyCometAppearance and sorted by comet medal cost.
    struct CometGalaxyEntry {
        CometGalaxyEntry() {}

        CometGalaxyEntry& operator=(const CometGalaxyEntry& rOther) NO_INLINE {
            mName = rOther.mName;
            mMedalNum = rOther.mMedalNum;
            mHasStar = rOther.mHasStar;
            return *this;
        }

        /* 0x0 */ const char* mName;
        /* 0x4 */ s32 mMedalNum;
        /* 0x8 */ bool mHasStar;
    };

    struct CometGalaxyList {
        CometGalaxyList() {
            CometGalaxyEntry* pEntry = mEntries;
            CometGalaxyEntry* pEnd = mEntries + 64;
            while (pEntry < pEnd) {
                pEntry->mName = nullptr;
                pEntry->mMedalNum = -1;
                pEntry->mHasStar = false;
                pEntry++;
            }
            mCount = 0;
        }

        CometGalaxyEntry mEntries[64];
        s32 mCount;
    };

    struct CometGalaxyEntryLess {
        bool operator()(const CometGalaxyEntry& rA, const CometGalaxyEntry& rB) const {
            return rA.mMedalNum < rB.mMedalNum;
        }
    };
}  // namespace

namespace GameSequenceFunction {
    GameSequenceInGame* getGameSequenceInGame() NO_INLINE {
        return GAME_SYSTEM->mSequenceDirector->mGameSequence->mInGame;
    }

    GameSequenceEnding* getGameSequenceEnding() NO_INLINE {
        return GAME_SYSTEM->mSequenceDirector->mGameSequence->mEnding;
    }

    SaveDataHandleSequence* getSaveDataHandleSequence() NO_INLINE {
        return GAME_SYSTEM->mSequenceDirector->mSaveDataHandleSequence;
    }

    NWC24Messenger* getNWC24Messenger() NO_INLINE {
        return GAME_SYSTEM->mSequenceDirector->mNWC24Messenger;
    }

    void closeWipeCircleToPlayerHead() {
        TPos3f headMtx;
        MR::calcPlayerJointMtx(&headMtx, "Head");
        TVec3f result;
        TVec3f pos(0.0f, -20.0f, 0.0f);
        headMtx.mult(pos, result);
        MR::closeSystemWipeCircleWithCaptureScreen(-1);
        MR::setWipeCircleCenterPos(result);
    }

    void closeWipeCircleAndPrepareDreamer(const NameObj* pObj, s32 scenarioNo) {
        GameDataFunction::forceMarioPlayer();
        GameSystemSceneController* pController = GAME_SYSTEM->mSceneController;
        JMapIdInfo idInfo(scenarioNo, pObj->mLinkInfo.mZoneID);
        requestChangeStageForceWipeClose(pController->mCurrSceneControlInfo.mStage, pController->getCurrentScenarioNo(), pController->getCurrentSelectedScenarioNo(), &idInfo);
        getGameSequenceInGame()->setInStage(pController->mCurrSceneControlInfo.mStage);
        closeWipeCircleToPlayerHead();
    }

    void prepareStageResetLuigiIntrusively(bool isLuigi, const TVec3f& rPos) {
        IN_STAGE_HOLDER->clearForRetry();
        GameSystemSceneController* pController = GAME_SYSTEM->mSceneController;
        JMapIdInfo idInfo(MR::getInitializeStartIdInfo());
        requestChangeStageForceWipeClose(pController->mCurrSceneControlInfo.mStage, pController->getCurrentScenarioNo(), pController->getCurrentSelectedScenarioNo(), &idInfo);
        getGameSequenceInGame()->setInStage(pController->mCurrSceneControlInfo.mStage);
        MR::closeSystemWipeCircleWithCaptureScreen(-1);
        MR::setWipeCircleCenterPos(rPos);
        MR::stopStageBGM(0x1E);
        MR::stopSubBGM(0x1E);
        MR::onDestroySceneKeepAllSound();
        MR::startSystemSE("SE_SY_CHANGE_ROOM_TO_LUIGI", -1, -1);
        if (isLuigi) {
            GameDataFunction::forceLuigiPlayer();
        } else {
            GameDataFunction::forceMarioPlayer();
        }
    }

    void prepareStageResetChangeRoom(s32 scenarioNo, bool isLuigi) {
        requestChangeStageMarioFaceShip(scenarioNo, -1);
        closeWipeCircleToPlayerHead();
        MR::stopStageBGM(0x1E);
        MR::onDestroySceneKeepAllSound();
        MR::startSystemSE("SE_SY_CHANGE_ROOM_TO_LUIGI", -1, -1);
        if (isLuigi) {
            GameDataFunction::forceLuigiPlayer();
        } else {
            GameDataFunction::forceMarioPlayer();
        }
    }

    bool isClearedPowerStar(const char* pStageName, s32 powerStarId) {
        PlayResultInStageHolder* pHolder = IN_STAGE_HOLDER;
        bool result = false;
        if (powerStarId == pHolder->getClearedPowerStarNum() && MR::isEqualString(pHolder->getStageName(), pStageName)) {
            result = true;
        }
        return result;
    }

    bool isClearedGrandStarVsKoopaLv3GalaxyFirstTime() {
        PlayResultInStageHolder* pHolder = IN_STAGE_HOLDER;
        bool result = false;
        if (GameDataConst::isGrandStar(pHolder->getStageName(), pHolder->getClearedPowerStarNum()) && !isClearedPowerStar("VsKoopaLv3Galaxy", 1)) {
            result = true;
        }
        return result;
    }

    void startPreLoadSaveDataSequence() {
        getSaveDataHandleSequence()->startPreLoad();
    }

    void startGameDataSaveSequence(bool a, bool b, bool c) {
        getSaveDataHandleSequence()->startSaveGameData(a, b, c);
    }

    void startGameDataSaveSequenceAtGameOverEnd() {
        if (!MR::isOnGameEventFlagNormalEnding()) {
            GameDataFunction::forceMarioPlayer();
        }
        GameDataFunction::onGameEventFlag("ゲームオーバーで終了");
        getSaveDataHandleSequence()->startSaveGameData(false, false, true);
    }

    void startWaitSaveDataSequence() {
        getSaveDataHandleSequence()->startWaitSaveData();
    }

    void startLoadUserFileSequence(int index) {
        getSaveDataHandleSequence()->startLoadUserFile(index);
    }

    void startReloadUserFileSequence() {
        getSaveDataHandleSequence()->startReloadUserFile();
    }

    void startCreateUserFileSequence(int index) {
        getSaveDataHandleSequence()->startCreateUserFile(index);
    }

    void startDeleteUserFileSequence(int index) {
        getSaveDataHandleSequence()->startDeleteUserFile(index);
    }

    void storeMiiOrIconIdAndSaveAllUserFileSequence(int index, const void* pMiiId, const u32* pIconId) {
        getSaveDataHandleSequence()->storeMiiOrIconId(index, pMiiId, pIconId);
        getSaveDataHandleSequence()->startSaveAll();
    }

    void storeMiiOrIconIdUserFileSequence(int index, const void* pMiiId, const u32* pIconId) {
        getSaveDataHandleSequence()->storeMiiOrIconId(index, pMiiId, pIconId);
    }

    void startSaveAllUserFileSequence() {
        getSaveDataHandleSequence()->startSaveAll();
    }

    bool isNoSaveMode() {
        return getSaveDataHandleSequence()->_2C;
    }

    bool isActiveSaveDataHandleSequence() {
        return getSaveDataHandleSequence()->isActive();
    }

    bool isSuccessSaveDataHandleSequence() {
        return getSaveDataHandleSequence()->_24 == 2;
    }

    void restoreUserFile(UserFile* pUserFile, int index) {
        getSaveDataHandleSequence()->restoreUserFile(pUserFile, index);
    }

    u8* getClearedSaveTempBuffer(s32 size) {
        return getSaveDataHandleSequence()->clearTempBuffer(size);
    }

    void loadSaveTempBufferAsGameData() {
        getSaveDataHandleSequence()->loadTempBufferAsGameData();
    }

    bool tryNANDErrorSequence(s32 code) {
        return getSaveDataHandleSequence()->tryNANDErrorSequence(code);
    }

    void requestPrepareResetNWC24() {
        getNWC24Messenger()->prepareReset();
    }

    bool isEnableToResetNWC24() {
        return !getNWC24Messenger()->isRunning();
    }

    void resetNWC24() {
        getNWC24Messenger()->reset();
    }

    void notifyToGameSequenceProgressToEndScene() {
        GameDataFunction::copySysConfigData();
        resetPlayResultHolders();
        if (!MR::isOnGameEventFlagNormalEnding()) {
            GameDataFunction::forceMarioPlayer();
        }
        requestChangeStageNoReset("FileSelect", 1, -1, nullptr);
        GAME_SYSTEM->mSequenceDirector->mGameSequence->setTitle();
    }

    void requestChangeStageAfterFileSelect() {
        GAME_SYSTEM->mSequenceDirector->mGameSequence->setInGame();
        resetPlayResultHolders();
        if (!MR::isOnGameEventFlagNormalEnding()) {
            GameDataFunction::forceMarioPlayer();
        }
#ifdef GALAXY_NATIVE
        // An explicit startup request overrides this one destination only.
        namespace Launch = PetariNative::App::LaunchStage;
        if (Launch::onFileLoaded(true)) return;
        if (Launch::startupStatus() == Launch::StartupStatus::Failed) {
            std::fprintf(stderr, "SMG2 stage request: %s\n", Launch::startupError().c_str());
            std::_Exit(2);
        }
#endif
        if (GameDataFunction::isOnGameEventFlag("オープニング実行")) {
            s32 powerStarNum = GameDataFunction::calcCurrentPowerStarNum();
            if (powerStarNum == 0) {
                requestChangeStage("PeachCastleGalaxy", 3, -1, nullptr);
                getGameSequenceInGame()->setOpening();
            } else if (powerStarNum == 1 && !GameDataFunction::isOnGameEventFlagWorld1Entered()) {
                PlayResultMarioFaceShipHolder* pHolder = FACE_SHIP_HOLDER;
                pHolder->mIsAfterFileSelect = true;
                requestChangeStageMarioFaceShip(0, 1);
            } else {
                if (GameDataFunction::isOnGameEventFlag("ゲームオーバーで終了")) {
                    PlayResultWorldMapHolder* pHolder = WORLD_MAP_HOLDER;
                    pHolder->mIsAfterGameOver = true;
                    GameDataFunction::offGameEventFlag("ゲームオーバーで終了");
                }
                PlayResultMarioFaceShipHolder* pFaceShip = FACE_SHIP_HOLDER;
                pFaceShip->mIsAfterFileSelect = true;
                WORLD_MAP_HOLDER->mIsAfterFileSelect = true;
                requestChangeStageMarioFaceShip(0, -1);
            }
        } else {
            requestChangeStage("PeachCastleGalaxy", 1, -1, nullptr);
            getGameSequenceInGame()->setOpening();
        }
    }

    void requestChangeStageMarioFaceShip(s32 startIdx, s32 scenarioNo) {
        s32 scenario = scenarioNo;
        if (scenarioNo < 0) {
            scenario = 2;
        }
        JMapIdInfo idInfo(startIdx, 0);
        requestChangeStage("MarioFaceShipGalaxy", scenario, -1, &idInfo);
        getGameSequenceInGame()->setMarioFace();
    }

    void requestChangeStageWorldMap() {
        GAME_SYSTEM->mSequenceDirector->mGameSequence->setInGame();
        IN_STAGE_HOLDER->reset();
        if (!MR::isOnGameEventFlagNormalEnding()) {
            GameDataFunction::forceMarioPlayer();
        }
        char stageName[0x20];
        snprintf(stageName, 0x20, "WorldMap%02dGalaxy", GameDataFunction::getCurrentWorldId());
        requestChangeStage(stageName, 1, -1, nullptr);
        getGameSequenceInGame()->setWorldMap();
    }

    void requestChangeStageWorldMapEnterType0() {
        PlayResultWorldMapHolder* pHolder = WORLD_MAP_HOLDER;
        pHolder->mEnterType = 0;
        requestChangeStageWorldMap();
    }

    void requestChangeStageWorldMapEnterType1() {
        PlayResultWorldMapHolder* pHolder = WORLD_MAP_HOLDER;
        pHolder->mEnterType = 1;
        requestChangeStageWorldMap();
    }

    void requestChangeStageWorldMapMoveToWorld(s32 worldId) {
        if (worldId > GameDataFunction::getCurrentWorldId()) {
            PlayResultWorldMapHolder* pHolder = WORLD_MAP_HOLDER;
            pHolder->mEnterType = 2;
        } else {
            PlayResultWorldMapHolder* pHolder = WORLD_MAP_HOLDER;
            pHolder->mEnterType = 3;
        }
        GameDataFunction::setCurrentWorldId(worldId);
        requestChangeStageWorldMap();
    }

    void requestChangeStageWorldMapEnterType4(s32 worldId) {
        PlayResultWorldMapHolder* pHolder = WORLD_MAP_HOLDER;
        pHolder->mEnterType = 4;
        GameDataFunction::setCurrentWorldId(worldId);
        requestChangeStageWorldMap();
    }

    void requestChangeStageInStage(const char* pStageName) {
        IN_STAGE_HOLDER->reset();
        requestChangeStageNoReset(pStageName, 1, -1, nullptr);
        getGameSequenceInGame()->setInStage(pStageName);
    }

#ifdef GALAXY_NATIVE
    void requestChangeStageInStageScenario(const char* pStageName, s32 scenarioNo, s32 selectedScenarioNo) {
        IN_STAGE_HOLDER->reset();
        requestChangeStage(pStageName, scenarioNo, selectedScenarioNo, nullptr);
        getGameSequenceInGame()->setInStage(pStageName);
    }
#endif

    void requestChangeStageAfterStageClear() {
        const char* pStageName = IN_STAGE_HOLDER->getStageName();
        reflectStageResult();
        if (isClearedPowerStar("VsKoopaLv3Galaxy", 1)) {
            if (!MR::isOnGameEventFlagNormalEnding()) {
                GameDataFunction::forceMarioPlayer();
            }
            requestChangeStage("PeachCastleGalaxy", 2, -1, nullptr);
            GAME_SYSTEM->mSequenceDirector->mGameSequence->setEnding();
            getGameSequenceEnding()->startInit();
        } else {
            if (isClearedIslandFleetGalaxyFirstTimeAtResult()) {
                if (!MR::isOnGameEventFlagNormalEnding()) {
                    GameDataFunction::forceMarioPlayer();
                }
                GameDataFunction::setGalaxyStateOpened(pStageName);
                requestChangeStageMarioFaceShip(0, 1);
            } else if (isClearedGrandStarVsKoopaLv3GalaxyAtResult()) {
                if (!MR::isOnGameEventFlagNormalEnding()) {
                    GameDataFunction::forceMarioPlayer();
                }
                requestChangeStageMarioFaceShip(0, 4);
            } else {
                requestChangeStageMarioFaceShip(0, -1);
            }
            MR::forceCloseSystemWipeCircle();
        }
    }

    void requestChangeStageAfterMiss() {
        if (getGameSequenceInGame()->isMarioFace()) {
            requestChangeStageMarioFaceShip(0, -1);
            return;
        }
        IN_STAGE_HOLDER->clearAfterMiss();
        GameSequenceInStage* pInStage = getGameSequenceInGame()->mInStage;
        pInStage->mIsRetry = true;
        GameSystemSceneController* pController = GAME_SYSTEM->mSceneController;
        JMapIdInfo idInfo(*IN_STAGE_HOLDER->mRestartMarioNo);
        requestChangeStageForceWipeClose(pController->mCurrSceneControlInfo.mStage, pController->getCurrentScenarioNo(), pController->getCurrentSelectedScenarioNo(), &idInfo);
        getGameSequenceInGame()->setInStage(pController->mCurrSceneControlInfo.mStage);
    }

    void restartCurrentStageAndClearAfterMiss() {
        IN_STAGE_HOLDER->clearAfterMiss();
        GameSystemSceneController* pController = GAME_SYSTEM->mSceneController;
        JMapIdInfo idInfo(*IN_STAGE_HOLDER->mRestartMarioNo);
        requestChangeStageForceWipeClose(pController->mCurrSceneControlInfo.mStage, pController->getCurrentScenarioNo(), pController->getCurrentSelectedScenarioNo(), &idInfo);
        getGameSequenceInGame()->setInStage(pController->mCurrSceneControlInfo.mStage);
        closeWipeCircleToPlayerHead();
    }

    void requestChangeScenarioSelect(const char* pStageName) {
        IN_STAGE_HOLDER->reset();
        requestChangeStageScenarioSelect(pStageName);
        getGameSequenceInGame()->setInStage(pStageName);
    }

    void requestChangeStageMarioFaceShipBeforeKoopaVs3() {
        IN_STAGE_HOLDER->reset();
        requestChangeStage("MarioFaceShipGalaxy", 3, -1, nullptr);
        getGameSequenceInGame()->setBeforeKoopaVs3();
    }

    void requestChangeStageStaffRoll() {
        s32 scenarioNo = (isCollectedAll120PowerStars() != 0) + 1;
        requestChangeStage("StaffRollGalaxy", scenarioNo, -1, nullptr);
        getGameSequenceEnding()->startWipeClose();
    }

    void requestChangeStageChildRoom() {
        if (isCollectedAll120PowerStars()) {
            requestChangeStage("ChildRoomGalaxy", 1, -1, nullptr);
            getGameSequenceEnding()->startDemoMessage();
        } else {
            getGameSequenceEnding()->requestSaveDemo();
        }
    }

    void setInStageKillReady() {
        GameSequenceInStage* pInStage = getGameSequenceInGame()->mInStage;
        pInStage->mIsKillReady = true;
    }

    void reflectStageResult() {
        PlayResultInStageHolder* pHolder = IN_STAGE_HOLDER;
        pHolder->getStageName();
        pHolder->setupStageResult();
        updateGameDataAtStageClear();
        IN_STAGE_HOLDER->updateNewlyOpenedGalaxies();
    }

    void setClearedPowerStarNum(s32 num) {
        IN_STAGE_HOLDER->setClearedPowerStarNum(num);
    }

    bool hasStageResultSequence() {
        return IN_STAGE_HOLDER->mIsStageResult;
    }

    bool hasResultPowerStar() {
        return IN_STAGE_HOLDER->mHasPowerStar;
    }

    bool isResultStarNotGreen() {
        return IN_STAGE_HOLDER->mIsNotGreenStar;
    }

    bool isResultNotifyTicoCoin() {
        return IN_STAGE_HOLDER->mIsTicoCoinSaved;
    }

    bool isNewlyOpenedGalaxyAtResult(const char* pName) {
        return IN_STAGE_HOLDER->isNewlyOpenedGalaxy(pName);
    }

    const char* getClearedStageName() {
        return IN_STAGE_HOLDER->getStageName();
    }

    s32 getClearedPowerStarId() {
        return IN_STAGE_HOLDER->getClearedPowerStarNum();
    }

    s32 getClearedStarPieceNum() {
        return IN_STAGE_HOLDER->getClearedStarPieceNum();
    }

    s32 getClearedCoinNum() {
        return IN_STAGE_HOLDER->getClearedCoinNum();
    }

    s32 getResultBestTime() {
        s32 time = IN_STAGE_HOLDER->mPlayTime;
        s32 result = 0x34BC0;
        if (time < 0x34BC0) {
            result = time;
        }
        return result;
    }

    bool isResultNewBestTime() {
        return IN_STAGE_HOLDER->mIsNewBestTime;
    }

    bool isClearedIslandFleetGalaxyFirstTimeAtResult() {
        bool result = false;
        if (IN_STAGE_HOLDER->mHasPowerStar && isClearedPowerStar("IslandFleetGalaxy", 1)) {
            result = true;
        }
        return result;
    }

    bool isClearedGrandStarVsKoopaLv3GalaxyAtResult() {
        bool result = false;
        if (IN_STAGE_HOLDER->mHasPowerStar && isClearedGrandStarVsKoopaLv3GalaxyFirstTime()) {
            result = true;
        }
        return result;
    }

    bool isClearedAll120PowerStarAtResult() {
        bool result = false;
        if (IN_STAGE_HOLDER->mHasPowerStar && GameDataFunction::calcCurrentPowerStarNum() == 0x78) {
            result = true;
        }
        return result;
    }

    // NONMATCHING: isGot/isAll swap their callee-saved registers (r31/r29) (FP order/grouping checked: no FP instructions).
    bool isClearedAllPowerStarAtResult() {
        bool isAll;
        bool result = false;
        bool isGot = true;
        isAll = false;
        if (!IN_STAGE_HOLDER->mHasPowerStar && !IN_STAGE_HOLDER->mIsNotGreenStar) {
            isGot = false;
        }
        if (isGot && GameDataFunction::getNormalPowerStarNumOwned() == 0x78) {
            isAll = true;
        }
        if (isAll && GameDataFunction::calcCurrentPowerStarNum() == GameDataFunction::getPowerStarNumMax() - 2) {
            result = true;
        }
        return result;
    }

    bool isResultUnlockGhostLuigi() {
        return IN_STAGE_HOLDER->mIsUnlockGhostLuigi;
    }

    bool isClearedIslandFleetGalaxyFirstTime() {
        PlayResultInStageHolder* pHolder = IN_STAGE_HOLDER;
        bool result = false;
        if (!GameDataFunction::hasPowerStar(pHolder->getStageName(), pHolder->getClearedPowerStarNum()) && isClearedPowerStar("IslandFleetGalaxy", 1)) {
            result = true;
        }
        return result;
    }

    // NONMATCHING: register/stack or instruction scheduling (FP order/grouping proven by default rdclass a).
    bool isClearedVsKoopaLv3GalaxyFirstTime() {
        bool result;
        PlayResultInStageHolder* pHolder = IN_STAGE_HOLDER;
        result = false;
        if (!GameDataFunction::hasPowerStar(pHolder->getStageName(), pHolder->getClearedPowerStarNum()) && isClearedGrandStarVsKoopaLv3GalaxyFirstTime()) {
            result = true;
        }
        return result;
    }

    bool isClearedVsKoopaLv3Galaxy1() {
        return isClearedPowerStar("VsKoopaLv3Galaxy", 1);
    }

    PlayResultInStageHolder* getPlayResultInStageHolder() NO_INLINE {
        return getGameSequenceInGame()->getPlayResultInStageHolder();
    }

    GameSequenceNameEntryTable* getGameSequenceNameEntryTable() {
        return GAME_SYSTEM->mSequenceDirector->mGameSequence->mNameEntryTable;
    }

    s32 getResultMissCount() {
        return IN_STAGE_HOLDER->mMissCount;
    }

    bool isResultMiss() {
        return IN_STAGE_HOLDER->mMissCount > 0;
    }

    bool hasRetryGalaxySequence() {
        return IN_STAGE_HOLDER->mHasRetryGalaxy;
    }

    void setResultPowerStarAppeared() {
        PlayResultInStageHolder* pHolder = IN_STAGE_HOLDER;
        pHolder->mIsPowerStarAppeared = true;
    }

    bool isResultPowerStarAppeared() {
        return IN_STAGE_HOLDER->mIsPowerStarAppeared;
    }

    s32 getResultPlayerHealth() {
        if (IN_STAGE_HOLDER->mPadMode == 1) {
            return 3;
        }
        return IN_STAGE_HOLDER->getPlayerHealth();
    }

    s32 getResultCoinOneUpCount() {
        return IN_STAGE_HOLDER->mCoinOneUpNum;
    }

    void incPlayTimeInStage() {
        PlayResultInStageHolder* pHolder = IN_STAGE_HOLDER;
        pHolder->mPlayTime = MR::min(pHolder->mPlayTime + 1, 0x34BC0);
    }

    s32 getElapsedStageFrame() {
        return MR::min(IN_STAGE_HOLDER->mPlayTime, 0x34BC0);
    }

    void resetPlayTimeInStage() {
        PlayResultInStageHolder* pHolder = IN_STAGE_HOLDER;
        pHolder->mPlayTime = 0;
    }

    void resetPlayResultInStage() {
        IN_STAGE_HOLDER->reset();
    }

    void setWorldMapCurrentNode(s32 node) {
        WORLD_MAP_HOLDER->mCurrentNode = node;
    }

    s32 getWorldMapCurrentNode() {
        return WORLD_MAP_HOLDER->mCurrentNode;
    }

    void setTicoFatStarPieceId(s32 id) {
        WORLD_MAP_HOLDER->setTicoFatStarPieceId(id);
    }

    void setTicoFatStarPieceRequirement(s32 num) {
        WORLD_MAP_HOLDER->setTicoFatStarPieceRequirement(num);
    }

    s32 getTicoFatStarPieceId() {
        return WORLD_MAP_HOLDER->mTicoFatId;
    }

    s32 getTicoFatStarPieceRequirement() {
        return WORLD_MAP_HOLDER->mTicoFatStarPieceNum;
    }

    bool isTicoFatStarPieceActive() {
        return WORLD_MAP_HOLDER->mTicoFatId >= 0;
    }

    void clearTicoFatStarPiece() {
        PlayResultWorldMapHolder* pHolder = WORLD_MAP_HOLDER;
        pHolder->mTicoFatId = -1;
        pHolder->mTicoFatStarPieceNum = 0;
    }

    void setWorldMapGalaxyFlag(s32 worldNo, s32 index) {
        WORLD_MAP_HOLDER->setGalaxyFlag(worldNo, index);
    }

    bool isWorldMapGalaxyFlag(s32 worldNo, s32 index) {
        return WORLD_MAP_HOLDER->isGalaxyFlag(worldNo, index);
    }

    void storeWorldMapRoute(const WorldMapResultRoute* pRoute) {
        WORLD_MAP_HOLDER->storeRoute(pRoute);
    }

    void loadWorldMapRoute(WorldMapResultRoute* pRoute) {
        WORLD_MAP_HOLDER->loadRoute(pRoute);
    }

    void clearWorldMapRouteCount() {
        WORLD_MAP_HOLDER->clearRouteCount();
    }

    void clearWorldMapAfterGameOver() {
        PlayResultWorldMapHolder* pHolder = WORLD_MAP_HOLDER;
        pHolder->mIsAfterGameOver = false;
    }

    bool isWorldMapAfterGameOver() {
        return WORLD_MAP_HOLDER->mIsAfterGameOver;
    }

    bool isWorldMapAfterFileSelect() {
        return WORLD_MAP_HOLDER->mIsAfterFileSelect;
    }

    void clearWorldMapAfterFileSelect() {
        PlayResultWorldMapHolder* pHolder = WORLD_MAP_HOLDER;
        pHolder->mIsAfterFileSelect = false;
    }

    void setWorldMapFlag22C() {
        PlayResultWorldMapHolder* pHolder = WORLD_MAP_HOLDER;
        pHolder->mFlag22C = true;
    }

    void clearWorldMapFlag22C() {
        PlayResultWorldMapHolder* pHolder = WORLD_MAP_HOLDER;
        pHolder->mFlag22C = false;
    }

    bool isWorldMapFlag22C() {
        return WORLD_MAP_HOLDER->mFlag22C;
    }

    void setWorldMapFlag22D() {
        PlayResultWorldMapHolder* pHolder = WORLD_MAP_HOLDER;
        pHolder->mFlag22D = true;
    }

    void clearWorldMapFlag22D() {
        PlayResultWorldMapHolder* pHolder = WORLD_MAP_HOLDER;
        pHolder->mFlag22D = false;
    }

    bool isWorldMapFlag22D() {
        return WORLD_MAP_HOLDER->mFlag22D;
    }

    void setWorldMapEnterType5() {
        PlayResultWorldMapHolder* pHolder = WORLD_MAP_HOLDER;
        pHolder->mEnterType = 5;
    }

    bool isWorldMapEnterType0() {
        return WORLD_MAP_HOLDER->mEnterType == 0;
    }

    bool isWorldMapEnterType1() {
        return WORLD_MAP_HOLDER->mEnterType == 1;
    }

    bool isWorldMapEnterType2() {
        return WORLD_MAP_HOLDER->mEnterType == 2;
    }

    bool isWorldMapEnterType3() {
        return WORLD_MAP_HOLDER->mEnterType == 3;
    }

    bool isWorldMapEnterType4() {
        return WORLD_MAP_HOLDER->mEnterType == 4;
    }

    bool isWorldMapEnterType5() {
        return WORLD_MAP_HOLDER->mEnterType == 5;
    }

    bool isFaceShipAfterFileSelect() {
        return FACE_SHIP_HOLDER->mIsAfterFileSelect;
    }

    void clearFaceShipAfterFileSelect() {
        PlayResultMarioFaceShipHolder* pHolder = FACE_SHIP_HOLDER;
        pHolder->mIsAfterFileSelect = false;
    }

    bool isFaceShipRespondGameOver() {
        return FACE_SHIP_HOLDER->mIsRespondGameOver;
    }

    void setFaceShipRespondGameOver() {
        PlayResultMarioFaceShipHolder* pHolder = FACE_SHIP_HOLDER;
        pHolder->mIsRespondGameOver = true;
    }

    void clearFaceShipRespondGameOver() {
        PlayResultMarioFaceShipHolder* pHolder = FACE_SHIP_HOLDER;
        pHolder->mIsRespondGameOver = false;
    }

    bool isFaceShipSkipBankInfo() {
        return FACE_SHIP_HOLDER->mIsSkipBankInfo;
    }

    void setFaceShipSkipBankInfo() {
        PlayResultMarioFaceShipHolder* pHolder = FACE_SHIP_HOLDER;
        pHolder->mIsSkipBankInfo = true;
    }

    s32 getFaceShipClearedScenariosCount() {
        return FACE_SHIP_HOLDER->mClearedScenariosCount;
    }

    void incFaceShipClearedScenariosCount() {
        FACE_SHIP_HOLDER->mClearedScenariosCount++;
    }

    void clearFaceShipClearedScenariosCount() {
        PlayResultMarioFaceShipHolder* pHolder = FACE_SHIP_HOLDER;
        pHolder->mClearedScenariosCount = 0;
    }

    bool isFaceShipFlag1C() {
        return FACE_SHIP_HOLDER->mFlag1C;
    }

    void setFaceShipFlag1C() {
        PlayResultMarioFaceShipHolder* pHolder = FACE_SHIP_HOLDER;
        pHolder->mFlag1C = true;
    }

    void clearFaceShipFlag1C() {
        PlayResultMarioFaceShipHolder* pHolder = FACE_SHIP_HOLDER;
        pHolder->mFlag1C = false;
    }

    void resetPlayResultHolders() {
        IN_STAGE_HOLDER->reset();
        WORLD_MAP_HOLDER->reset();
        FACE_SHIP_HOLDER->reset();
        getGameSequenceNameEntryTable()->clear();
    }

    void setPadModeDreamer(const NameObj* pObj, s32 scenarioNo) {
        IN_STAGE_HOLDER->setPadModeDreamer();
        closeWipeCircleAndPrepareDreamer(pObj, scenarioNo);
    }

    void setPadModeSuperDreamer(const NameObj* pObj, s32 scenarioNo) {
        IN_STAGE_HOLDER->setPadModeSuperDreamer();
        closeWipeCircleAndPrepareDreamer(pObj, scenarioNo);
    }

    void clearPadModeDreamer() {
        IN_STAGE_HOLDER->clearPadMode();
        getSaveDataHandleSequence()->backupCurrentGameData();
        GameSystemSceneController* pController = GAME_SYSTEM->mSceneController;
        JMapIdInfo idInfo(*pController->mCurrSceneControlInfo.mStartIdInfo);
        requestChangeStageForceWipeClose(pController->mCurrSceneControlInfo.mStage, pController->getCurrentScenarioNo(), pController->getCurrentSelectedScenarioNo(), &idInfo);
        getGameSequenceInGame()->setInStage(pController->mCurrSceneControlInfo.mStage);
        closeWipeCircleToPlayerHead();
    }

    void clearPadModeSuperDreamer() {
        IN_STAGE_HOLDER->clearPadMode();
        GameSceneFunction::startStarPointerModeGame();
        MR::activateDefaultGameLayout();
    }

    bool isPadModeDreamer() {
        return IN_STAGE_HOLDER->mPadMode == 1;
    }

    bool isPadModeSuperDreamer() {
        return IN_STAGE_HOLDER->mPadMode == 2;
    }

    bool isPadMode() {
        bool result = true;
        if (IN_STAGE_HOLDER->mPadMode != 1 && IN_STAGE_HOLDER->mPadMode != 2) {
            result = false;
        }
        return result;
    }

    bool isPadMode2() {
        return isPadMode();
    }

    bool isAllowDreamerMissNum(s32 missNum) {
        if (GameDataFunction::getScenarioMissNum(MR::getCurrentStageName(), MR::getCurrentScenarioNo()) < missNum) {
            return false;
        }
        return isAllowDreamerInStage();
    }

    bool isAllowDreamerInStage() {
        if (GameDataFunction::isLuigiPlayer()) {
            return false;
        }
        return !MR::isPowerStarGreenInCurrentStage();
    }

    bool isSuperDreamer() {
        return IN_STAGE_HOLDER->mIsSuperDreamer;
    }

    // NONMATCHING: branch layout of the nested bool temporaries differs (same logic)
    bool isLuigiIntrusivelyAppear() {
        if (!GameDataFunction::isOnLuigiIntrusivelyStart()) {
            return false;
        }
        if (GameDataFunction::isLuigiPlayer()) {
            return false;
        }
        if (MR::isOnGameEventFlagNormalEnding()) {
            return false;
        }
        if (isPadMode()) {
            return false;
        }
        if (checkAppearGhostLuigi(MR::getCurrentStageName(), MR::getCurrentOrSelectedScenarioNo())) {
            return false;
        }
        bool result = true;
        bool isIntruding = true;
        if (!GameDataFunction::isOnCurrentScenarioFlagLuigiIntruded()) {
            if (!IN_STAGE_HOLDER->mIsLuigiIntruding) {
                isIntruding = false;
            }
        }
        if (!isIntruding && !GameDataFunction::isOnGameEventFlagStarPieceCounterStop()) {
            result = false;
        }
        return result;
    }

    void prepareStageResetLuigiIntrusivelyLuigi(const TVec3f& rPos) {
        prepareStageResetLuigiIntrusively(true, rPos);
    }

    void prepareStageResetLuigiIntrusivelyMario(const TVec3f& rPos) {
        prepareStageResetLuigiIntrusively(false, rPos);
    }

    void prepareStageResetChangeRoomLuigi(s32 scenarioNo) {
        prepareStageResetChangeRoom(scenarioNo, true);
    }

    void prepareStageResetChangeRoomMario(s32 scenarioNo) {
        prepareStageResetChangeRoom(scenarioNo, false);
    }

    // NONMATCHING: branch layout of the final 'return false' (same logic)
    bool checkAppearGhostLuigi(const char* pStageName, s32 scenarioNo) {
        if (isPadMode()) {
            return false;
        }
        if (GameDataFunction::isOnGameEventFlagCoinCounterStop()) {
            return true;
        }
        s32 placed = getPlacedGreenStarScenarioNo(pStageName, scenarioNo);
        return !MR::isOnGameEventFlagPowerStarSuccess(pStageName, placed) ? false : GameDataFunction::isOnGalaxyScenarioFlagGhostLuigi(pStageName, placed);
    }

    s32 getPlacedGreenStarScenarioNo(const char* pStageName, s32 scenarioNo) {
        GalaxyStatusAccessor accessor = MR::makeGalaxyStatusAccessor(pStageName);
        return !accessor.isGreenStar(scenarioNo) ? scenarioNo : MR::getPlacedHiddenStarScenarioNo(pStageName, scenarioNo);
    }

    void collectTicoCoinInStage() {
        PlayResultInStageHolder* pHolder = IN_STAGE_HOLDER;
        pHolder->mIsPurpleCoinFull = true;
    }

    bool isCollectTicoCoinInStage() {
        return IN_STAGE_HOLDER->mIsPurpleCoinFull;
    }

    // NONMATCHING: branch layout of the two-flag test (same logic)
    bool isCollectTicoCoinSaved() {
        bool result = true;
        if (IN_STAGE_HOLDER->mIsPurpleCoinFull == 0) {
            if (IN_STAGE_HOLDER->mIsCollectTicoCoinSaved == 0) {
                result = false;
            }
        }
        return result;
    }

    void collectTicoCoinIfSaved() {
        if (IN_STAGE_HOLDER->mIsPurpleCoinFull) {
            IN_STAGE_HOLDER->setCollectTicoCoinSaved();
        }
    }

    // NONMATCHING: range check folded into one unsigned compare instead of two signed compares (same logic)
    bool hasGrandStarOfCurrentWorld() {
        s32 worldId = GameDataFunction::getCurrentWorldId();
        if (!(1 <= worldId && worldId <= 6)) {
            return false;
        }
        return GameDataFunction::hasGrandStar(worldId);
    }

    bool isGreenStarEventOnly() {
        if (GameDataFunction::isOnGameEventFlag("スター120個エンディング後デモ") && !GameDataFunction::isOnGameEventFlag("グリーンスター出現デモ")) {
            return true;
        }
        return false;
    }

    void setupNameEntryTableIfRetryGalaxy() {
        if (IN_STAGE_HOLDER->mHasRetryGalaxy) {
            getGameSequenceNameEntryTable()->registerZoneNames();
        }
    }

    bool isMarioFaceShipEventEmpty() {
        return getGameSequenceInGame()->mMarioFace->mEvent->isEmpty();
    }

    bool hasWorldMapHeap() {
        return GAME_SYSTEM->mSceneController->mHasWorldMapHeap;
    }

    void setupChangeStage(const char* pStageName, s32 scenarioNo, s32 selectedScenarioNo, const JMapIdInfo* pIdInfo) {
        GAME_SYSTEM->mSequenceDirector->mSaveDataHandleSequence->backupCurrentUserFile();
        MR::setStarPointerModeBase();
        GameSystemSceneController* pController = GAME_SYSTEM->mSceneController;
        pController->mNextSceneControlInfo.setScene("Game");
        pController->mNextSceneControlInfo.setStage(pStageName);
        pController->mNextSceneControlInfo.mScenarioNo = scenarioNo;
        pController->mNextSceneControlInfo.mSelectedScenarioNo = selectedScenarioNo;
        pController->mNextSceneControlInfo.setStartIdInfo(pIdInfo != nullptr ? *pIdInfo : MR::getInitializeStartIdInfo());
        pController->requestChangeScene();
    }

    void requestChangeStage(const char* pStageName, s32 scenarioNo, s32 selectedScenarioNo, const JMapIdInfo* pIdInfo) {
        setupChangeStage(pStageName, scenarioNo, selectedScenarioNo, pIdInfo);
        GAME_SYSTEM->mSequenceDirector->mGameSequence->mIsReset = true;
        GAME_SYSTEM->mSceneController->startScenarioSelectSceneBackground();
        getGameSequenceNameEntryTable()->clear();
    }

    void requestChangeStageNoReset(const char* pStageName, s32 scenarioNo, s32 selectedScenarioNo, const JMapIdInfo* pIdInfo) {
        setupChangeStage(pStageName, scenarioNo, selectedScenarioNo, pIdInfo);
        GAME_SYSTEM->mSequenceDirector->mGameSequence->mIsReset = false;
        GAME_SYSTEM->mSceneController->startScenarioSelectSceneBackground();
        getGameSequenceNameEntryTable()->clear();
    }

    void requestChangeStageScenarioSelect(const char* pStageName) {
        setupChangeStage(pStageName, -1, -1, nullptr);
        GAME_SYSTEM->mSequenceDirector->mGameSequence->mIsReset = false;
        GAME_SYSTEM->mSceneController->startScenarioSelectScene();
        getGameSequenceNameEntryTable()->clear();
    }

    void requestChangeStageForceWipeClose(const char* pStageName, s32 scenarioNo, s32 selectedScenarioNo, const JMapIdInfo* pIdInfo) {
        setupChangeStage(pStageName, scenarioNo, selectedScenarioNo, pIdInfo);
        GAME_SYSTEM->mSequenceDirector->mGameSequence->mIsReset = false;
        GAME_SYSTEM->mSceneController->startScenarioSelectSceneBackground();
        MR::forceCloseSystemWipeCircle();
    }

    void requestChangeScene(const char* pSceneName) {
        GameSystemSceneController* pController = GAME_SYSTEM->mSceneController;
        pController->mNextSceneControlInfo.setScene(pSceneName);
        pController->mNextSceneControlInfo.setStage("");
        pController->mNextSceneControlInfo.mScenarioNo = -1;
        pController->requestChangeScene();
        MR::setStarPointerModeBase();
        GAME_SYSTEM->mSequenceDirector->mGameSequence->mIsReset = true;
    }

    void startScene() {
        GameSystemFunction::setPermissionToCheckWiiRemoteConnectAndScreenDimming(true);
        GAME_SYSTEM->mSceneController->startScene();
        GAME_SYSTEM->mSequenceDirector->mGameSequence->mIsReset = false;
    }

    bool isReadyToStartScene() {
        return GAME_SYSTEM->mSceneController->isReadyToStartScene();
    }

    bool isGameSequenceWipeStarted() {
        return GAME_SYSTEM->mSequenceDirector->mGameSequence->mIsWipeStarted;
    }

    s32 isCollectedAll120PowerStars() {
        return GameDataFunction::calcCurrentPowerStarNum() >= 0x78;
    }

    void updateGameDataAtStageClear() {
        PlayResultInStageHolder* pHolder = getPlayResultInStageHolder();
        const char* pStageName = pHolder->getStageName();
        s32 starId = pHolder->getClearedPowerStarNum();
        bool isGreen = MR::isPowerStarGreen(pStageName, starId);
        if (GameDataFunction::hasPowerStar(pStageName, starId)) {
            if (pHolder->mIsNotGreenStar) {
                GameDataFunction::setPowerStar(pStageName, starId);
            }
        } else if (!pHolder->mIsSuperDreamer || isGreen) {
            GameDataFunction::setPowerStar(pStageName, starId);
        } else {
            GameDataFunction::setBronzeStar(pStageName, starId);
        }
        collectTicoCoinIfSaved();
        if (pHolder->mIsCollectTicoCoinSaved) {
            GameDataFunction::onGalaxyFlagTicoCoin(pStageName);
        }
        GameDataFunction::onGalaxyScenarioFlagAlreadyVisited(pStageName, starId);
        if (pHolder->getClearedStarPieceNum() > 0) {
            s32 room = 9999 - GameDataFunction::getStockedStarPieceNum();
            if (room < pHolder->getClearedStarPieceNum()) {
                pHolder->addStarPieceNum(room - pHolder->getClearedStarPieceNum());
            }
            GameDataFunction::tryOnGameEventFlagStarPieceCounterStop(pHolder->getClearedStarPieceNum());
        }
        if (pHolder->getClearedCoinNum() > 0) {
            s32 room = 9999 - GameDataFunction::getStockedCoinNum();
            if (room < pHolder->getClearedCoinNum()) {
                pHolder->addCoinNum(room - pHolder->getClearedCoinNum());
            }
            pHolder->setCoinOneUpNum(GameDataFunction::tryOnGameEventFlagCoinCounterStop(pHolder->getClearedCoinNum()));
        }
        if (GameDataFunction::getScenarioClearTime(pStageName, starId) == 0 || pHolder->mIsNewBestTime) {
            GameDataFunction::setScenarioBestTime(pStageName, starId, pHolder->mPlayTime);
        }
        if (GameDataFunction::isLuigiPlayer()) {
            if (!isGreen && !checkAppearGhostLuigi(pStageName, starId)) {
                pHolder->mIsUnlockGhostLuigi = true;
            }
            GameDataFunction::onGalaxyScenarioFlagGhostLuigi(pStageName, starId);
        }
        GameDataFunction::updateLuigiIntrusionCounter();
        if (GameDataFunction::isOnGameEventFlag("銀行屋キノピオ初回")) {
            GameDataFunction::updateBankInterest();
        }
        if (isClearedAllPowerStarAtResult()) {
            GameDataFunction::onGameEventFlag("最終ギャラクシー出現");
        }
        GameDataFunction::clearFaceShipEventValues();
        if (pHolder->mHasPowerStar) {
            if (GameDataConst::isGrandStar(pStageName, starId)) {
                s32 grandStarNo = GameDataFunction::getGrandStarNo(pStageName);
                if (grandStarNo < 6) {
                    GameDataFunction::setFaceShipEventGrandStarNo(grandStarNo);
                }
            }
        }
        updateGalaxyCometAppearance(pStageName, starId);
    }

    // NONMATCHING: the 64-entry init loop is emitted as a counted (bdnz) loop instead of the target pointer loop, which shifts register allocation; std::sort<> body also differs slightly (MSL __sort132 register choice) (FP order/grouping checked: no FP instructions).
    void updateGalaxyCometAppearance(const char* pStageName, s32 starId) {
        GalaxyStatusAccessor stageAccessor = MR::makeGalaxyStatusAccessor(pStageName);
        if (stageAccessor.isValidComet(starId)) {
            GameDataFunction::offGalaxyFlagComet(pStageName);
        }
        if (GameDataFunction::isOnGameEventFlagCometExplain() ||
            (!isClearedGrandStarVsKoopaLv3GalaxyAtResult() && GameDataFunction::calcNormalPowerStarNumFromWorld(3) >= 4)) {
            CometGalaxyList list;
            s32 worldCometNum[8];
            MR::zeroMemory(worldCometNum, sizeof(worldCometNum));
            for (ScenarioDataIter iter = MR::makeBeginScenarioDataIter(); !iter.isEnd(); iter.goNext()) {
                GalaxyStatusAccessor accessor = iter.makeAccessor();
                if (accessor.hasCometScenario()) {
                    const char* pName = accessor.getName();
                    bool hasStar = accessor.hasPowerStar(accessor.getCometScenarioId());
                    CometGalaxyEntry entry;
                    entry.mMedalNum = GameDataFunction::getCometAppearMedalNum(pName);
                    entry.mName = pName;
                    entry.mHasStar = hasStar;
                    list.mEntries[list.mCount++] = entry;
                    if (GameDataFunction::isOnGalaxyFlagComet(pName)) {
                        worldCometNum[accessor.getWorldNo()]++;
                    }
                }
            }
            CometGalaxyEntryLess less;
            std::sort< CometGalaxyEntry*, CometGalaxyEntryLess& >(list.mEntries, list.mEntries + list.mCount, less);
            s32 ticoCoinNum = GameDataFunction::calcTicoCoinNum();
            bool isStageFound = false;
            for (s32 i = 0; i < list.mCount; i++) {
                CometGalaxyEntry* pEntry = &list.mEntries[i];
                const char* pName = pEntry->mName;
                GalaxyStatusAccessor accessor = MR::makeGalaxyStatusAccessor(pName);
                s32 worldNo = accessor.getWorldNo();
                if (!GameDataFunction::isOnGalaxyFlagComet(pName) && worldCometNum[worldNo] < 3) {
                    const char* pEntryName = pEntry->mName;
                    GalaxyStatusAccessor entryAccessor = MR::makeGalaxyStatusAccessor(pEntryName);
                    entryAccessor.getWorldNo();
                    bool isEnable;
                    if (pEntry->mHasStar) {
                        isEnable = false;
                    } else if (!MR::hasAllNormalPowerStarWithoutComet(pEntryName)) {
                        isEnable = false;
                    } else if (!GameDataFunction::isOnGalaxyFlagTicoCoin(pEntryName)) {
                        isEnable = false;
                    } else if (ticoCoinNum < pEntry->mMedalNum) {
                        isEnable = false;
                    } else {
                        const char* pEntryGalaxyName = entryAccessor.getName();
                        const char* pNames[2] = {"HomeCenterGalaxy", "MarioOnMarioGalaxy"};
                        bool isFirstOk;
                        if (MR::isEqualString(pEntryGalaxyName, "CloudGardenGalaxy")) {
                            isFirstOk = true;
                        } else {
                            bool isMatch = MR::isEqualString(entryAccessor.getCometName(entryAccessor.getCometScenarioId()), "Mimic");
                            if (!isMatch) {
                                for (u32 k = 0; k < 2; k++) {
                                    if (MR::isEqualString(pEntryGalaxyName, pNames[k])) {
                                        isMatch = true;
                                        break;
                                    }
                                }
                            }
                            GalaxyStatusAccessor cloudAccessor = MR::makeGalaxyStatusAccessor("CloudGardenGalaxy");
                            isFirstOk = false;
                            if (!isMatch || cloudAccessor.hasPowerStar(cloudAccessor.getCometScenarioId())) {
                                isFirstOk = true;
                            }
                        }
                        if (!isFirstOk) {
                            isEnable = false;
                        } else if (MR::isEqualString(entryAccessor.getName(), "HomeCenterGalaxy")) {
                            isEnable = true;
                        } else {
                            bool isPurple = MR::isEqualString(entryAccessor.getCometName(entryAccessor.getCometScenarioId()), "Purple");
                            GalaxyStatusAccessor homeAccessor = MR::makeGalaxyStatusAccessor("HomeCenterGalaxy");
                            isEnable = false;
                            if (!isPurple || homeAccessor.hasPowerStar(homeAccessor.getCometScenarioId())) {
                                isEnable = true;
                            }
                        }
                    }
                    if (isEnable) {
                        if (!MR::isEqualString(pName, "DimensionBigCastleGalaxy") || !MR::isEqualString(pName, "DimensionBigCastleGalaxy") ||
                            GameDataFunction::isSysConfigCounterMax()) {
                            if (MR::isEqualStringCase(pName, pStageName)) {
                                isStageFound = true;
                            } else {
                                GameDataFunction::onGalaxyFlagComet(pName);
                                return;
                            }
                        }
                    }
                }
            }
            if (isStageFound) {
                for (ScenarioDataIter iter = MR::makeBeginScenarioDataIter(); !iter.isEnd(); iter.goNext()) {
                    GalaxyStatusAccessor accessor = iter.makeAccessor();
                    if (accessor.getPowerStarNum() != 0 && !GameDataFunction::isGalaxyStateClosed(accessor.getName())) {
                        for (s32 k = 1; k <= accessor.getPowerStarNum(); k++) {
                            if (!accessor.hasPowerStar(k) && accessor.isOpenScenario(k)) {
                                return;
                            }
                        }
                    }
                }
                GameDataFunction::onGalaxyFlagComet(pStageName);
            }
        }
    }

    void initAfterResourceLoaded() {
        GAME_SYSTEM->mSequenceDirector->mGameSequence->mInGame->mInStage->initPlayerMissLeft();
        GAME_SYSTEM->mSequenceDirector->mGameSequence->mEnding->initAfterResourceLoaded();
    }

    void drawPlayerMissLeft() {
        GAME_SYSTEM->mSequenceDirector->mGameSequence->drawPlayerMissLeft();
    }
};  // namespace GameSequenceFunction
