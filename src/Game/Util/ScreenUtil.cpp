#include "Game/System/WorldMapOwner.hpp"
#include "Game/Util/ScreenUtil.hpp"
#include "Game/Util.hpp"
#include "Game/Scene/GameSceneFunction.hpp"
#include "Game/Scene/PlayTimerScene.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/Screen/BloomEffect.hpp"
#include "Game/Screen/BloomEffectSimple.hpp"
#include "Game/Screen/CaptureScreenDirector.hpp"
#include "Game/Screen/CounterLayoutController.hpp"
#include "Game/Screen/CoinCounter.hpp"
#include "Game/Screen/DestroyCounter.hpp"
#include "Game/Screen/InformationMessage.hpp"
#include "Game/Screen/YesNoLayout.hpp"
#include "Game/Scene/ModelWipeScene.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/Screen/CenterScreenBlur.hpp"
#include "Game/Screen/CinemaFrame.hpp"
#include "Game/Screen/CounterLayoutController.hpp"
#include "Game/Screen/DepthOfFieldBlur.hpp"
#include "Game/Screen/GameSceneLayoutHolder.hpp"
#include "Game/Screen/ImageEffectDirector.hpp"
#include "Game/Screen/ImageEffectSystemHolder.hpp"
#include "Game/Screen/InformationMessage.hpp"
#include "Game/Screen/LensFlare.hpp"
#include "Game/Screen/MoviePlayerSimple.hpp"
#include "Game/Screen/PurpleCoinCounter.hpp"
#include "Game/Screen/SceneWipeHolder.hpp"
#include "Game/Screen/ScreenBlurEffect.hpp"
#include "Game/Screen/StarCounter.hpp"
#include "Game/Screen/StarPieceCounter.hpp"
#include "Game/Screen/SystemWipeHolder.hpp"
#include "Game/Screen/YesNoController.hpp"
#include "Game/Screen/YesNoLayout.hpp"
#include "Game/System/GameSystem.hpp"
#include "Game/System/GameSystemObjHolder.hpp"
#include "Game/System/GameSystemSceneController.hpp"
#include "Game/System/RenderMode.hpp"
#include "Game/Util/LayoutUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/SingletonHolder.hpp"
#include "Game/Util/SoundUtil.hpp"
#include "Game/Util/SystemUtil.hpp"


namespace {
    char sWipeCircle[] = "円ワイプ";
    char sWipeFade[] = "フェードワイプ";
    char sWipeWhiteFade[] = "白フェードワイプ";
    char sWipeGameOver[] = "ゲームオーバー";
    char sWipeKoopa[] = "クッパ";
    char sWipeMario[] = "マリオワイプ";
    char sSoundFiveUp[] = "SE_SY_5UP";
    char sSoundOneUp[] = "SE_SY_1UP";

    CaptureScreenDirector* getCaptureScreenDirector() NO_INLINE {
        return SingletonHolder< GameSystem >::get()->mObjHolder->mCaptureScreenDirector;
    }

    void appearInformationMessage(bool hasButtonLayout) NO_INLINE {
        if (hasButtonLayout) {
            MR::getGameSceneLayoutHolder()->mInformationMessage->setAButtonPos();
        } else {
            MR::getGameSceneLayoutHolder()->mInformationMessage->appear();
        }
    }

    StarCounter* getStarCounter() NO_INLINE {
        return MR::getGameSceneLayoutHolder()->mCounterLayoutCtrl->mStarCounter;
    }

    StarPieceCounter* getStarPieceCounter() NO_INLINE {
        return MR::getGameSceneLayoutHolder()->mCounterLayoutCtrl->mStarPieceCounter;
    }

    CoinCounter* getCoinCounter() NO_INLINE {
        return MR::getGameSceneLayoutHolder()->mCounterLayoutCtrl->mCoinCounter;
    }

    ModelWipeScene* getModelWipeScene() NO_INLINE {
        return SingletonHolder< GameSystem >::get()->mSceneController->mModelWipeScene;
    }

    SystemWipeHolder* getSystemWipeHolder() {
        return SingletonHolder< GameSystem >::get()->mSystemWipeHolder;
    }

    BloomEffect* getNormalBloom() {
        return MR::getSceneObj< BloomEffect >(SceneObj_BloomEffect);
    }

    CinemaFrame* getCinemaFrame() {
        return MR::getSceneObj< CinemaFrame >(SceneObj_CinemaFrame);
    }

};  // namespace

void ScreenUtil_FORCE_MATCH_SDATA2() {
    (void)0.87f;
    (void)0.844f;
}
namespace MR {


    s32 getScreenWidth() {
        return isScreen16Per9() ? 812 : 608;  // SMG2 16:9 width (0x32C); SMG1 used 832
    }

// NONMATCHING: regalloc (same calls and order; GPR/FPR assignment differs)
    s32 getSafetyFrameWidth() {
        s32 screenWidth = getScreenWidth();
        f32 screenWidthF = static_cast< f32 >(screenWidth);
        f32 viWidthF = static_cast< f32 >(static_cast< s32 >(getViWidth()));
        s32 viWidthMax = static_cast< u16 >(RenderModeObj::getViWidthMax());
        f32 safetyFrameWidthRatio = viWidthMax * getSafetyVIScreenWidthRatio();
        f32 viWidthRatio = screenWidthF / viWidthF;

        return safetyFrameWidthRatio * viWidthRatio + 0.5f;
    }

    s32 getSafetyFrameLeft() {
        return (getScreenWidth() - getSafetyFrameWidth()) / 2;
    }

    s32 getSafetyFrameRight() {
        return (getScreenWidth() + getSafetyFrameWidth()) / 2;
    }

    u32 getViWidth() {
        return SingletonHolder< GameSystem >::get()->mObjHolder->getRenderModeObj()->viWidth;
    }

#pragma dont_inline on
    f32 getSafetyVIScreenWidthRatio() {
        return isScreen16Per9() ? 0.87f : 0.844f;
    }
#pragma dont_inline reset

    void convertFrameBufferPosToScreenPos(TVec2f* pScreenPos, const TVec2f& rFrameBufferPos) {
        f32 posRatio = rFrameBufferPos.x / getFrameBufferWidth();

        pScreenPos->set(posRatio * getScreenWidth(), rFrameBufferPos.y);
    }

    void convertScreenPosToFrameBufferPos(TVec2f* pFrameBufferPos, const TVec2f& rScreenPos) {
        f32 posRatio = rScreenPos.x / getScreenWidth();

        pFrameBufferPos->set(posRatio * getFrameBufferWidth(), rScreenPos.y);
    }

    void startToCaptureScreen(const char* pParam1) {
        ::getCaptureScreenDirector()->requestCaptureTiming(pParam1);
    }

    void endToCaptureScreen(const char* pParam1) {
        ::getCaptureScreenDirector()->invalidateCaptureTiming(pParam1);
    }

    void captureScreenIfAllow(const char* pParam1) {
        ::getCaptureScreenDirector()->captureIfAllow(pParam1);
    }

// NONMATCHING: target inlines a 0x14 pointer load from CaptureScreenDirector; header layout puts the texture pointer at 0x1C and getResTIMG is out of line
    bool isExistCaptureScreenTexture() {
        return ::getCaptureScreenDirector()->getResTIMG() != nullptr;
    }

    const ResTIMG* getScreenResTIMG() {
        return ::getCaptureScreenDirector()->getResTIMG();
    }


    void closeWipeCircle(s32 frame) {
        SceneWipeHolderFunction::closeWipe(::sWipeCircle, frame);
    }

    void forceOpenWipeCircle() {
        SceneWipeHolderFunction::forceOpenWipe(::sWipeCircle);
    }

    void forceCloseWipeCircle() {
        SceneWipeHolderFunction::forceCloseWipe(::sWipeCircle);
    }

    void closeWipeFade(s32 frame) {
        SceneWipeHolderFunction::closeWipe(::sWipeFade, frame);
    }

    void forceOpenWipeFade() {
        SceneWipeHolderFunction::forceOpenWipe(::sWipeFade);
    }

    void forceCloseWipeFade() {
        SceneWipeHolderFunction::forceCloseWipe(::sWipeFade);
    }

    void closeWipeWhiteFade(s32 frame) {
        SceneWipeHolderFunction::closeWipe(::sWipeWhiteFade, frame);
    }



    bool isWipeActive() {
        return SceneWipeHolderFunction::getSceneWipeHolder()->isWipeIn() || SceneWipeHolderFunction::getSceneWipeHolder()->isWipeOut();
    }

    bool isWipeBlank() {
        return SceneWipeHolderFunction::getSceneWipeHolder()->isClose();
    }

    bool isWipeOpen() {
        return SceneWipeHolderFunction::getSceneWipeHolder()->isOpen();
    }

    void openSystemWipeCircle(s32 frame) {
        ::getSystemWipeHolder()->forceClose(::sWipeCircle);
        ::getSystemWipeHolder()->wipe(nullptr, frame);
    }

    void openSystemWipeCircleOnPlayer(s32 frame) {  // PLACEHOLDER(core): name guessed
        openSystemWipeCircle(frame);
        ::getSystemWipeHolder()->setWipeRingCenterOnPlayer();
    }

    void closeSystemWipeCircle(s32 frame) {
        ::getSystemWipeHolder()->forceOpen(::sWipeCircle);
        ::getSystemWipeHolder()->wipe(nullptr, frame);
    }

    void forceOpenSystemWipeCircle() {
        ::getSystemWipeHolder()->forceOpen(::sWipeCircle);
    }

    void forceCloseSystemWipeCircle() {
        ::getSystemWipeHolder()->forceClose(::sWipeCircle);
    }

    void openSystemWipeFade(s32 frame) {
        ::getSystemWipeHolder()->forceClose(::sWipeFade);
        ::getSystemWipeHolder()->wipe(nullptr, frame);
    }

    void closeSystemWipeFade(s32 frame) {
        ::getSystemWipeHolder()->forceOpen(::sWipeFade);
        ::getSystemWipeHolder()->wipe(nullptr, frame);
    }

    void forceOpenSystemWipeFade() {
        ::getSystemWipeHolder()->forceOpen(::sWipeFade);
    }

    void forceCloseSystemWipeFade() {
        ::getSystemWipeHolder()->forceClose(::sWipeFade);
    }

    void openSystemWipeWhiteFade(s32 frame) {
        ::getSystemWipeHolder()->forceClose(::sWipeWhiteFade);
        ::getSystemWipeHolder()->wipe(nullptr, frame);
    }

    void closeSystemWipeWhiteFade(s32 frame) {
        ::getSystemWipeHolder()->forceOpen(::sWipeWhiteFade);
        ::getSystemWipeHolder()->wipe(nullptr, frame);
    }

    void forceOpenSystemWipeWhiteFade(s32) {
        ::getSystemWipeHolder()->forceOpen(::sWipeWhiteFade);
    }

    void forceCloseSystemWipeWhiteFade() {
        ::getSystemWipeHolder()->forceClose(::sWipeWhiteFade);
    }

    void openSystemWipeMario(s32 frame) {
        ::getSystemWipeHolder()->forceClose(::sWipeMario);
        ::getSystemWipeHolder()->wipe(nullptr, frame);
    }

    void closeSystemWipeMario(s32 frame) {
        ::getSystemWipeHolder()->forceOpen(::sWipeMario);
        ::getSystemWipeHolder()->wipe(nullptr, frame);
    }

    void forceOpenSystemWipeMario() {
        ::getSystemWipeHolder()->forceOpen(::sWipeMario);
    }

    bool isSystemWipeActive() {
        return ::getSystemWipeHolder()->isWipeIn() || ::getSystemWipeHolder()->isWipeOut();
    }

    bool isSystemWipeBlank() {  // PLACEHOLDER(core): name guessed
        return ::getSystemWipeHolder()->isClose();
    }

    bool isSystemWipeOpen() {  // PLACEHOLDER(core): name guessed
        return ::getSystemWipeHolder()->isOpen();
    }

    void closeSystemWipeCircleWithCaptureScreen(s32 frame) {
        closeSystemWipeCircle(frame);
        ::getSystemWipeHolder()->startGameScreenCapture();
    }

    void startSystemWipeCircleWithCaptureScreen(s32 frame) {
        closeSystemWipeFade(frame);
        ::getSystemWipeHolder()->startGameScreenCapture();
    }

    void closeSystemWipeFadeWithCaptureScreen(s32 frame) {
        closeSystemWipeWhiteFade(frame);
        ::getSystemWipeHolder()->startGameScreenCapture();
    }

    void setWipeCircleCenterPos(const TVec3f& rCenterPos) {
        ::getSystemWipeHolder()->setWipeRingCenter(rCenterPos);
    }

    void startCloudWipeForward() {  // PLACEHOLDER(core): MR name guessed
        getModelWipeScene()->startCloudWipeForward();
    }

    void startCloudWipeBackward() {  // PLACEHOLDER(core): MR name guessed
        getModelWipeScene()->startCloudWipeBackward();
    }

    void startGalaxyWipeForward() {  // PLACEHOLDER(core): MR name guessed
        getModelWipeScene()->startGalaxyWipeForward();
    }

    void startGalaxyWipeBackward() {  // PLACEHOLDER(core): MR name guessed
        getModelWipeScene()->startGalaxyWipeBackward();
    }

    void startSpaceWipeForward() {  // PLACEHOLDER(core): MR name guessed
        getModelWipeScene()->startSpaceWipeForward();
    }

    void startSpaceWipeBackward() {  // PLACEHOLDER(core): MR name guessed
        getModelWipeScene()->startSpaceWipeBackward();
    }

    void requestEndModelWipe() {  // PLACEHOLDER(core): MR name guessed
        getModelWipeScene()->requestEnd();
    }

    void tryEndModelWipe() {  // PLACEHOLDER(core): MR name guessed
        getModelWipeScene()->tryEnd();
    }

    void forceHideModelWipe() {  // PLACEHOLDER(core): MR name guessed
        getModelWipeScene()->forceHide();
    }

    bool isHideOrEndModelWipe() {  // PLACEHOLDER(core): MR name guessed
        return getModelWipeScene()->isHideOrEnd();
    }

    bool isHiddenModelWipe() {  // PLACEHOLDER(core): MR name guessed
        return getModelWipeScene()->isHidden();
    }

    void requestMovementOnImageEffect() {
        if (isExistSceneObj(SceneObj_ImageEffectSystemHolder)) {
            getImageEffectSystemHolder()->pauseOff();
        }

        if (isExistSceneObj(SceneObj_BloomEffect)) {
            requestMovementOn(::getNormalBloom());
        }

        if (isExistSceneObj(SceneObj_BloomEffectSimple)) {
            requestMovementOn(getSceneObj< BloomEffectSimple >(SceneObj_BloomEffectSimple));
        }

        if (isExistSceneObj(SceneObj_ScreenBlurEffect)) {
            requestMovementOn(getSceneObj< ScreenBlurEffect >(SceneObj_ScreenBlurEffect));
        }

        if (isExistSceneObj(SceneObj_DepthOfFieldBlur)) {
            requestMovementOn(getSceneObj< DepthOfFieldBlur >(SceneObj_DepthOfFieldBlur));
        }
    }

    void setImageEffectControlAuto() {
        getImageEffectDirector()->setAuto();
    }

    void turnOffDOFInSubjective() {
        if (isExistImageEffectDirector()) {
            getImageEffectDirector()->turnOffDOFInSubjective();
        }
    }

    void turnOnDOFInSubjective() {
        if (isExistImageEffectDirector()) {
            getImageEffectDirector()->turnOnDOFInSubjective();
        }
    }

    void turnOnNormalBloom() {
        getImageEffectDirector()->turnOnNormal();
    }

    void turnOnDepthOfField(bool param1) {
        getImageEffectDirector()->turnOnDepthOfField(param1);
    }

    void turnOffImageEffect() {
        getImageEffectDirector()->turnOff();
    }


    void createNormalBloom() {
        createSceneObj(SceneObj_BloomEffect);
    }

    bool isNormalBloomOn() {
        return isExistSceneObj(SceneObj_BloomEffect) && ::getNormalBloom()->isSomething();
    }

    bool isNormalBloomForEachModel() {
        return isExistSceneObj(SceneObj_BloomEffect) && ::getNormalBloom()->isForEachModel();
    }

    void setNormalBloomIntensity(u8 intensity) {
        getImageEffectDirector()->setNormalBloomIntensity(intensity);
    }

    void setNormalBloomThreshold(u8 threshold) {
        getImageEffectDirector()->setNormalBloomThreshold(threshold);
    }

    void setNormalBloomBlurIntensity1(u8 intensity1) {
        getImageEffectDirector()->setNormalBloomBlurIntensity1(intensity1);
    }

    void setNormalBloomBlurIntensity2(u8 intensity2) {
        getImageEffectDirector()->setNormalBloomBlurIntensity2(intensity2);
    }



    void createDepthOfFieldBlur() {
        createSceneObj(SceneObj_DepthOfFieldBlur);
    }

    void setDepthOfFieldBlurIntensity(f32 intensity) {
        getImageEffectDirector()->setDepthOfFieldIntensity(intensity);
    }

    void setDepthOfFieldBlurDistance(s32 nearDistance, s32 farDistance) {
        getImageEffectDirector()->setDepthOfFieldBlurDistance(nearDistance, farDistance);
    }

    void createPostFog() {
        createSceneObj(SceneObj_PostFog);
    }

    void createPostOutline() {
        createSceneObj(SceneObj_PostOutline);
    }

    void createCenterScreenBlur() {
        createSceneObj(SceneObj_CenterScreenBlur);
    }

    void startCenterScreenBlur(s32 time, f32 offset, u8 alpha, s32 fadeIn, s32 fadeOut) {
        getSceneObj< CenterScreenBlur >(SceneObj_CenterScreenBlur)->start(time, offset, alpha, fadeIn, fadeOut);
    }





    void killCenterScreenBlur() {
        LiveActor* pBlur = static_cast< LiveActor* >(MR::getSceneObjHolder()->getObj(SceneObj_CenterScreenBlur));
        if (!MR::isDead(pBlur)) {
            pBlur->kill();
        }
    }

    void requestOneUpBoard(s32 num) {
        if (num >= 5) {
            startSystemSE(sSoundFiveUp, -1, -1);
        } else {
            startSystemSE(sSoundOneUp, -1, -1);
        }
        getGameSceneLayoutHolder()->requestOneUp(num);
    }

    void requestPowerUpHPMeter() {
        getGameSceneLayoutHolder()->requestPowerUpHPMeter();
    }

    void requestForceAppearHPMeter() {
        getGameSceneLayoutHolder()->requestForceAppearHPMeter();
    }

    void startMissLayout() {
        getGameSceneLayoutHolder()->startMiss();
    }

    bool isMissLayoutAnimEnd() {
        return getGameSceneLayoutHolder()->isMissAnimEnd();
    }

    void activateDefaultGameLayout() {
        getGameSceneLayoutHolder()->activateDefaultGameLayout();
    }

    void deactivateDefaultGameLayout() {
        getGameSceneLayoutHolder()->deactivateDefaultGameLayout();
    }

    bool isActiveDefaultGameLayout() {
        return getGameSceneLayoutHolder()->mIsActiveDefaultGameLayout;
    }

// NONMATCHING: target never sets the bool argument register (r4) before forceAppear(bool); the C form here passes false
    void forceAppearDefaultGameLayout() {
        getGameSceneLayoutHolder()->activateDefaultGameLayout();
        getGameSceneLayoutHolder()->mCounterLayoutCtrl->forceAppear(false);
    }

    void hideWorldMapInfoLayout() {
        if (isExistSceneObj(SceneObj_WorldMapOwner)) {
            static_cast< WorldMapOwner* >(getSceneObjHolder()->getObj(SceneObj_WorldMapOwner))->killCounters();
        }
    }

    void pauseOffLensFlare() {
        if (!isExistSceneObj(SceneObj_LensFlareDirector)) {
            return;
        }

        getSceneObj< LensFlareDirector >(SceneObj_LensFlareDirector)->pauseOff();
    }

    void tryScreenToFrameCinemaFrame() {
        ::getCinemaFrame()->tryScreenToFrame();
    }

    void tryFrameToBlankCinemaFrame() {
        ::getCinemaFrame()->tryFrameToBlank();
    }

    void tryBlankToFrameCinemaFrame() {
        ::getCinemaFrame()->tryBlankToFrame();
    }

    void tryFrameToScreenCinemaFrame() {
        ::getCinemaFrame()->tryFrameToScreen();
    }


    void forceToFrameCinemaFrame() {
        ::getCinemaFrame()->forceToFrame();
    }

    void forceToBlankCinemaFrame() {
        ::getCinemaFrame()->forceToBlank();
    }

    bool isStopCinemaFrame() {
        return ::getCinemaFrame()->isStop();
    }

    void appearInformationMessageUp(const char* pMessageId, bool hasButtonLayout) {
        getGameSceneLayoutHolder()->mInformationMessage->setMessage(pMessageId);
        getGameSceneLayoutHolder()->mInformationMessage->setWindowType(0);
        ::appearInformationMessage(hasButtonLayout);
    }

    void appearInformationMessageCenter(const char* pMessageId, bool hasButtonLayout) {
        getGameSceneLayoutHolder()->mInformationMessage->setMessage(pMessageId);
        getGameSceneLayoutHolder()->mInformationMessage->setWindowType(1);
        ::appearInformationMessage(hasButtonLayout);
    }

    void appearInformationMessageDown(const char* pMessageId, bool hasButtonLayout) {
        getGameSceneLayoutHolder()->mInformationMessage->setMessage(pMessageId);
        getGameSceneLayoutHolder()->mInformationMessage->setWindowType(2);
        ::appearInformationMessage(hasButtonLayout);
    }

    void appearInformationMessage(const wchar_t* pMessage, bool hasButtonLayout) {
        getGameSceneLayoutHolder()->mInformationMessage->setMessage(pMessage);
        getGameSceneLayoutHolder()->mInformationMessage->setWindowType(0);
        ::appearInformationMessage(hasButtonLayout);
    }

    void setInformationMessageReplaceString(const wchar_t* pMessage, s32 param2) {
        getGameSceneLayoutHolder()->mInformationMessage->setReplaceString(pMessage, param2);
    }

    void disappearInformationMessage() {
        getGameSceneLayoutHolder()->mInformationMessage->disappear();
    }

    void disableCloseInformationMessage() {
        getGameSceneLayoutHolder()->mInformationMessage->setAButtonPosFlag(false);
    }

    bool isDeadInformationMessage() {
        return isDead(getGameSceneLayoutHolder()->mInformationMessage);
    }


    void appearYesNoSelector(const char* pMessage1, const char* pMessage2) {
        getGameSceneLayoutHolder()->mYesNoLayout->appear();
        getGameSceneLayoutHolder()->mYesNoLayout->setSelectMessage(pMessage1, pMessage2);
    }

    bool isDeadYesNoLayout() {
        return isDead(getGameSceneLayoutHolder()->mYesNoLayout);
    }

    bool isYesNoSelected() {
        return getGameSceneLayoutHolder()->mYesNoLayout->mController->isSelected();
    }

    bool isYesNoSelectedYes() {
        return getGameSceneLayoutHolder()->mYesNoLayout->mController->isSelectedYes();
    }

    void setYesNoSelectorSE(const char* pCursorSE, const char* pYesSE, const char* pNoSE) {
        getGameSceneLayoutHolder()->mYesNoLayout->mController->setSE(pCursorSE, pYesSE, pNoSE);
    }

    void resetYesNoSelectorSE() {
        getGameSceneLayoutHolder()->mYesNoLayout->mController->setSE(nullptr, nullptr, nullptr);
    }










    void forceSyncStarPieceCounter() {
        ::getStarPieceCounter()->forceSync();
    }

    void forceAppearStarPieceCounterForTicoFat() {
        ::getStarPieceCounter()->forceAppear(true);
    }

    void disappearStarPieceCounterForTicoFat() {
        ::getStarPieceCounter()->disappear(true);
    }

    void forceSyncCoinCounter() {
        getCoinCounter()->forceSync();
    }

    void forceAppearCoinCounterForTicoFat() {
        getCoinCounter()->forceAppear(true);
    }

    void disappearCoinCounterForTicoFat() {
        getCoinCounter()->disappear(true);
    }

    void createPurpleCoinCounter() {
        getGameSceneLayoutHolder()->createPurpleCoinCounter();
    }

    void validatePurpleCoinCounter() {
        getGameSceneLayoutHolder()->getPurpleCoinCounter()->validate();
    }

    void createDestroyCounter(s32 maximum) {
        getGameSceneLayoutHolder()->createDestroyCounter(maximum);
    }

    void activateDestroyCounter() {
        getGameSceneLayoutHolder()->getDestroyCounter()->activate();
    }

    void setDestroyCounterCount(s32 count) {  // PLACEHOLDER(core): name guessed
        getGameSceneLayoutHolder()->getDestroyCounter()->mCount = count;
    }

    void appearDestroyCounter() {
        getGameSceneLayoutHolder()->appearDestroyCounter();
    }

    void killDestroyCounter() {
        getGameSceneLayoutHolder()->killDestroyCounter();
    }

    void killScoreAttackResults() {
        hideScoreAttackLayouts();
        killAllScoreResults();
    }

    void showScoreAttackResults() {  // PLACEHOLDER(core): name guessed
        showScoreAttackLayouts();
    }

    void requestCounterLayoutAppearanceForTicoEat(bool param1) {
        getGameSceneLayoutHolder()->mCounterLayoutCtrl->requestedTicoEat(param1);
    }

    void openWipeCircle(s32 frame) {
        SceneWipeHolderFunction::openWipe(::sWipeCircle, frame);
    }

    void openWipeFade(s32 frame) {
        SceneWipeHolderFunction::openWipe(::sWipeFade, frame);
    }

    void openWipeWhiteFade(s32 frame) {
        SceneWipeHolderFunction::openWipe(::sWipeWhiteFade, frame);
    }

    void startGameOverWipe() {
        SceneWipeHolderFunction::getSceneWipeHolder()->wipe(::sWipeGameOver, -1);
    }

    void startDownWipe() {
        SceneWipeHolderFunction::getSceneWipeHolder()->wipe(::sWipeKoopa, -1);
    }

};  // namespace MR
