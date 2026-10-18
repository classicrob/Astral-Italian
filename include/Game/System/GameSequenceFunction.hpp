#pragma once

#include <revolution/types.h>
#include <JSystem/JGeometry/TVec.hpp>

class GalaxyMoveArgument;
class UserFile;

namespace GameSequenceFunction {
    bool isLuigiIntrusivelyAppear();
    void prepareStageResetLuigiIntrusivelyLuigi(const TVec3f&);
    void prepareStageResetLuigiIntrusivelyMario(const TVec3f&);
    bool isResultMiss();
    bool isGreenStarEventOnly();
    bool isFaceShipSkipBankInfo();
    void setFaceShipSkipBankInfo();
    bool getPlayResultMarioFaceShipFlag12();
    s32 getTicoFatStarPieceId();
    s32 getTicoFatStarPieceRequirement();
    bool isTicoFatStarPieceActive();
    bool isNoSaveMode();
    bool hasResultPowerStar();
    bool isResultStarNotGreen();
    bool isResultNotifyTicoCoin();
    bool isResultNewBestTime();
    bool isResultUnlockGhostLuigi();
    s32 getResultCoinOneUpCount();
    u32 getResultBestTime();
    void restartCurrentStageAndClearAfterMiss();
    const char* getClearedStageName();
    void resetPlayResultInStage();
    void collectTicoCoinInStage();
    bool isCollectTicoCoinInStage();
    bool isCollectTicoCoinSaved();
    bool isPadMode();
    bool isPadModeSuperDreamer();
    bool tryNANDErrorSequence(s32);
    void requestChangeScenarioSelect(const char*);
    bool isAllowDreamerInStage();
    bool isPadMode2();
    void setClearedPowerStarNum(s32);
};

// Merged from Petari (core lane, symbols.txt-verified)

namespace GameSequenceFunction {
    void requestChangeStageWorldMap();
    void requestChangeStageStaffRoll();
    void requestChangeStageChildRoom();
    void startGameDataSaveSequence(bool, bool, bool);
    bool isActiveSaveDataHandleSequence();
};


namespace GameSequenceFunction {
    void notifyToGameSequenceProgressToEndScene();
    void requestChangeScene(const char*);
    s32 getClearedPowerStarId();
    int getClearedStarPieceNum();
    int getClearedCoinNum();
    bool hasRetryGalaxySequence();
    void startPreLoadSaveDataSequence();
    void startCreateUserFileSequence(int);
    void startDeleteUserFileSequence(int);
    void storeMiiOrIconIdUserFileSequence(int, const void*, const u32*);
    void startSaveAllUserFileSequence();
    bool isActiveSaveDataHandleSequence();
    bool isSuccessSaveDataHandleSequence();
    void restoreUserFile(UserFile*, int);
    void requestPrepareResetNWC24();
    bool isEnableToResetNWC24();
    void resetNWC24();
    bool hasStageResultSequence();
    bool isGalaxyCometLand(const char*);
};  // namespace GameSequenceFunction

namespace GameSequenceFunction {
    s32 getElapsedStageFrame();
}

namespace GameSequenceFunction {
    s32 getPlacedGreenStarScenarioNo(const char*, s32);
}

namespace GameSequenceFunction {
    bool isPadModeDreamer();
}

namespace GameSequenceFunction { s32 getResultMissCount(); }

namespace GameSequenceFunction { bool isFaceShipRespondGameOver(); void clearFaceShipClearedScenariosCount(); s32 getFaceShipClearedScenariosCount(); void incFaceShipClearedScenariosCount(); }

class NameObj;
namespace GameSequenceFunction {
    void setPadModeSuperDreamer(const NameObj*, s32);
    void clearPadModeSuperDreamer();
    bool isAllowDreamerMissNum(s32);
}
