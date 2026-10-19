#pragma once

#include "Game/System/NerveExecutor.hpp"
#include "Game/Util/Array.hpp"

enum StarPointerModeEnum {
    StarPointerMode_HomeButton = 0,
    StarPointerMode_ErrorWindow = 1,
    StarPointerMode_SaveLoad = 2,
    StarPointerMode_Logo = 3,
    StarPointerMode_Title = 4,
    StarPointerMode_FileSelect = 5,
    StarPointerMode_ScenarioSelectScene = 6,
    StarPointerMode_PictureBook = 21,  // startStarPointerModePictureBook loads 0x15 (8005E48C).
    StarPointerMode_PauseMenu = 7,
    StarPointerMode_1PInvalid2PValid = 14,
    StarPointerMode_MarioLauncher = 16,
    StarPointerMode_Demo = 11,
    StarPointerMode_DemoMarioDeath = 12,
    StarPointerMode_PowerStarGetDemo = 13,
    StarPointerMode_ChooseYesNo = 8,
    StarPointerMode_SphereSelectorFinger = 15,
    StarPointerMode_DemoWithHandPointerFinger = 10,
    StarPointerMode_BlueStar = 17,
    StarPointerMode_BlueStarReady = 18,
    StarPointerMode_StarPieceTarget = 19,
    StarPointerMode_BigBubble = 20,
    StarPointerMode_DemoWithStarPointer = 9,
    StarPointerMode_CommandStream = 22,
    StarPointerMode_Base = 23,
    StarPointerMode_Game = 24,

    StarPointerMode_Count,
};

class StarPointerOnOffController : public NerveExecutor {
public:
    struct RequestInfo {
    public:
        /* 0x00 */ void* mRequester;
        /* 0x04 */ s32 mMode;
    };

    /// @brief Creates a new `StarPointerOnOffController`.
    StarPointerOnOffController();

    void update();
    void setStateToTitle(void*);
    void setStateToFileSelect(void*);
    void setStateToBase(void*);
    void popState(void*);
    void requestMode(void*, StarPointerModeEnum);
    void setupHomeButton();
    void setupErrorWindow();
    void setupLogo();
    void setupTitle();
    void setupFileSelect();
    void setupPictureBook();
    void setupDemo();
    void setupDemoWithStarPointer();
    void setupDemoWithHandPointerFinger();
    void setupDemoMarioDeath();
    void setup1PInvalid2PValid();
    void setupMarioLauncher();
    void setupChooseYesNo();
    void setupPauseMenu();
    void setupScenarioSelectScene();
    void setupBlueStar();
    void setupBlueStarReady();
    void setupPowerStarGetDemo();
    void setupStarPieceTarget();
    void setupSphereSelectorFinger();
    void setupSphereSelectorOnReaction();
    void setupBigBubble();
    void setupCommandStream();
    void setupGame();
    void setupBase();
    void updateMode();
    void clearInfo();
    void incModeCounter(void*, s32);
    void decModeCounter(void*);

    bool isMode(StarPointerModeEnum mode) NO_INLINE {
        return mModeCounter[mode] != 0 || mRequested[mode];
    }

    bool compareMode(StarPointerModeEnum mode) const {
        return mMode == mode;
    }

    /* 0x08 */ bool _8;
    /* 0x0C */ MR::FixedArray< RequestInfo*, 16 > mRequests;
    /* 0x4C */ s32 mModeCounter[StarPointerMode_Count];
    /* 0xB4 */ bool mRequested[StarPointerMode_Count];
    /* 0xD0 */ s32 mMode;
};
