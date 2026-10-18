#pragma once

#include <revolution/types.h>

class GalaxyMoveArgument;
class UserFile;

namespace GameSequenceFunction {
    bool isGreenStarEventOnly();
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
