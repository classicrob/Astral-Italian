#pragma once

#include <JSystem/JGeometry/TVec.hpp>
#include <JSystem/JUtility/JUTVideo.hpp>
#include <revolution/types.h>

struct ResTIMG;

namespace MR {
    bool isDeadYesNoLayout();
    bool requestTicoFatGuidance();
    void hideWorldMapInfoLayout();
    void forceSyncCoinCounter();
    void forceAppearCoinCounterForTicoFat();
    void disappearCoinCounterForTicoFat();
    void requestPauseBlurMovementOn();
    void incrementPauseBlurDelay();
    void decrementPauseBlurDelay();
    s32 getScreenWidth();
    inline s32 getScreenHeight() NO_INLINE {
        return JUTVideo::getManager()->getEfbHeight();
    }
    const ResTIMG* getScreenResTIMG();
};  // namespace MR

// Merged from Petari (core lane): declarations missing here


namespace MR {
    u32 getViWidth();
    f32 getSafetyVIScreenWidthRatio();
    s32 getSafetyFrameWidth();
    s32 getSafetyFrameLeft();
    s32 getSafetyFrameRight();
    void convertFrameBufferPosToScreenPos(TVec2f*, const TVec2f&);
    void convertScreenPosToFrameBufferPos(TVec2f*, const TVec2f&);
    void startToCaptureScreen(const char*);
    void endToCaptureScreen(const char*);
    void captureScreenIfAllow(const char*);
    u8* getScreenTexImage();
    void closeWipeCircle(s32 frame = -1);
    void forceOpenWipeCircle();
    void forceCloseWipeCircle();
    void closeWipeFade(s32 frame = -1);
    void forceOpenWipeFade();
    void forceCloseWipeFade();
    void closeSystemWipeMario(s32 frame = -1);
    void openSystemWipeMario(s32 frame = -1);
    void forceOpenSystemWipeMario();
    void closeWipeWhiteFade(s32 frame = -1);
    void forceOpenWipeWhiteFade();
    void forceCloseWipeWhiteFade();
    bool isWipeActive();
    bool isWipeBlank();
    bool isWipeOpen();
    bool isWipeBlank();
    bool isWipeOpen();
    void closeSystemWipeCircle(s32 frame = -1);
    void forceOpenSystemWipeCircle();
    void openSystemWipeFade(s32 frame = -1);
    void closeSystemWipeFade(s32 frame = -1);
    void forceOpenSystemWipeFade();
    void openSystemWipeWhiteFade(s32 frame = -1);
    void closeSystemWipeWhiteFade(s32 frame = -1);
    void forceCloseSystemWipeWhiteFade();
    void forceOpenSystemWipeWhiteFade(s32);
    void forceCloseSystemWipeFade();
    void forceCloseSystemWipeCircle();
    bool isSystemWipeActive();
    void closeSystemWipeCircleWithCaptureScreen(s32 frame = -1);
    void closeSystemWipeFadeWithCaptureScreen(s32 frame = -1);
    void startSystemWipeCircleWithCaptureScreen(s32 frame = -1);
    void setWipeCircleCenterPos(const TVec3f&);
    void requestMovementOnImageEffect();
    void setImageEffectControlAuto();
    void turnOffDOFInSubjective();
    void turnOnDOFInSubjective();
    void turnOnNormalBloom();
    void turnOnDepthOfField(bool);
    void turnOffImageEffect();
    bool isExistImageEffectDirector();
    void forceOffImageEffect();
    void createNormalBloom();
    bool isNormalBloomOn();
    bool isNormalBloomForEachModel();
    void setNormalBloomIntensity(u8);
    void setNormalBloomThreshold(u8);
    void setNormalBloomBlurIntensity1(u8);
    void setNormalBloomBlurIntensity2(u8);
    void createSimpleBloom();
    void createScreenBlur();
    void createDepthOfFieldBlur();
    void setDepthOfFieldBlurIntensity(f32);
    void setDepthOfFieldBlurDistance(s32, s32);
    void createCenterScreenBlur();
    void startCenterScreenBlur(s32, f32, u8, s32, s32);
    void startGlobalTimer();
    void resetGlobalTimer();
    bool isGlobalTimerEnd();
    void startGlobalTimerTimeUp();
    void requestPowerUpHPMeter();
    void requestForceAppearHPMeter();
    void startMissLayout();
    bool isMissLayoutAnimEnd();
    void activateDefaultGameLayout();
    void deactivateDefaultGameLayout();
    bool isActiveDefaultGameLayout();
    void forceAppearDefaultGameLayout();
    void pauseOffLensFlare();
    void tryScreenToFrameCinemaFrame();
    void tryFrameToBlankCinemaFrame();
    void tryBlankToFrameCinemaFrame();
    void tryFrameToScreenCinemaFrame();
    void forceToScreenCinemaFrame();
    void forceToFrameCinemaFrame();
    void forceToBlankCinemaFrame();
    bool isStopCinemaFrame();
    void appearInformationMessage(const char*, bool);
    void appearInformationMessageCenter(const char*, bool);
    void appearInformationMessageUp(const char*, bool);
    void appearInformationMessageDown(const char*, bool);
    void setInformationMessageReplaceString(const wchar_t*, s32);
    void disappearInformationMessage();
    void disableCloseInformationMessage();
    bool isDeadInformationMessage();
    void appearYesNoSelector(const char*, const char*, const char*);
    void setYesNoSelectorSE(const char*, const char*, const char*);
    void resetYesNoSelectorSE();
    void startMoviePlayer(const char*);
    void stopMoviePlayer();
    bool isActiveMoviePlayer();
    bool isMoviePlayerPlaying();
    s32 getMovieCurrentFrame();
    u32 getMovieTotalFrame();
    void setMovieVolume(f32, s32);
    void appearStarCounter();
    void disappearStarCounter();
    void forceSyncStarPieceCounter();
    void forceAppearStarPieceCounterForTicoFat();
    void disappearStarPieceCounterForTicoFat();
    void createPurpleCoinCounter();
    void validatePurpleCoinCounter();
    void requestCounterLayoutAppearanceForTicoEat(bool);
    void openWipeCircle(s32 frame = -1);
    void openWipeFade(s32 frame = -1);
    void openWipeWhiteFade(s32 frame = -1);
    void startGameOverWipe();
    void startDownWipe();
    void requestOneUp();
};  // namespace MR

// Merged from Petari (core lane): overloads missing here


namespace MR {
    void requestOneUpBoard(s32);  // owner: ScreenUtil unit (requestOneUpBoard__2MRFl)
    s32 getFrameBufferWidth() NO_INLINE;
    s32 getFrameBufferHeight() NO_INLINE;
    u32 getViWidth() NO_INLINE;
    f32 getSafetyVIScreenWidthRatio() NO_INLINE;
    void appearInformationMessage(const wchar_t*, bool);
    bool isYesNoSelected() NO_INLINE;
    bool isYesNoSelectedYes() NO_INLINE;
};  // namespace MR

namespace MR { void appearYesNoSelector(const char*,const char*); }

namespace MR {
    void killDestroyCounter();
}

namespace MR { bool isExistCaptureScreenTexture(); }

namespace MR {
    void openSystemWipeCircleAtPlayer(s32);  // SMG2 fn_80058C70 (core4 name): openSystemWipeCircle + SystemWipeHolder::setWipeRingCenterOnPlayer
    bool isSystemWipeBlank();                // SMG2 fn_80059100 (core4 name): SystemWipeHolder vfunc 0x40
    bool isSystemWipeOpen();                 // SMG2 fn_80059120 (core4 name): SystemWipeHolder vfunc 0x3C
    void requestEndModelWipeScene();         // SMG2 fn_80059300 (core4 name)
}

// core4 (ScreenUtil missing functions): declarations for symbols defined in ScreenUtil.cpp
class NameObj;
namespace MR {
    void openSystemWipeCircle(s32 frame = -1);
    void openSystemWipeCircleOnPlayer(s32 frame);  //
    bool isSystemWipeBlank();                      //
    bool isSystemWipeOpen();                       //
    void killCenterScreenBlur();
    void createPostFog();
    void createPostOutline();
    void createDestroyCounter(s32 maximum);
    void activateDestroyCounter();
    void setDestroyCounterCount(s32 count);        // PLACEHOLDER(core): name guessed (stores DestroyCounter+0x34 = mCount)
    void appearDestroyCounter();
    void killDestroyCounter();
    void showScoreAttackResults();                 //
    void hideScoreAttackLayouts();
    void showScoreAttackLayouts();
    void killAllScoreResults();
    // ModelWipeScene wrappers. PLACEHOLDER(core): MR names guessed from the ModelWipeScene methods they forward to.
    void startCloudWipeForward();
    void startCloudWipeBackward();
    void startGalaxyWipeForward();
    void startGalaxyWipeBackward();
    void startSpaceWipeForward();
    void startSpaceWipeBackward();
    void requestEndModelWipe();
    void tryEndModelWipe();
    void forceHideModelWipe();
    bool isHideOrEndModelWipe();
    bool isHiddenModelWipe();
};  // namespace MR
