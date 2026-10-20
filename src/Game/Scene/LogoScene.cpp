#include "Game/Scene/LogoScene.hpp"
#include "Game/Scene/NrvLogoScene.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Scene/SceneFunction.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/Screen/LogoFader.hpp"
#include "Game/Screen/SimpleLayout.hpp"
#include "Game/System/GameSequenceFunction.hpp"
#include "Game/System/GameSystemFunction.hpp"
#include "Game/System/MainLoopFramework.hpp"
#include "Game/Util/DrawUtil.hpp"
#include "Game/Util/GamePadUtil.hpp"
#include "Game/Util/LayoutUtil.hpp"
#include "Game/Util/NerveUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/SystemUtil.hpp"
#ifdef GALAXY_NATIVE
#include <petari/launch_stage.hpp>
#endif


namespace {
    const s32 FADEINOUT_FRAME = 30;
    const s32 CENSORSHIP_DISPLAY_FRAME = 180;
    const s32 STRAP_DISPLAY_MIN_FRAME = 450;
    const s32 STRAP_DISPLAY_MAX_FRAME = 1200;


};  // namespace

namespace NrvLogoScene {
    LogoSceneNrvStrapFadein LogoSceneNrvStrapFadein::sInstance;
    LogoSceneNrvStrapDisplay LogoSceneNrvStrapDisplay::sInstance;
    LogoSceneNrvStrapFadeout LogoSceneNrvStrapFadeout::sInstance;
    LogoSceneNrvMountGameData LogoSceneNrvMountGameData::sInstance;
    LogoSceneNrvWaitReadDoneSystemArchive LogoSceneNrvWaitReadDoneSystemArchive::sInstance;
    LogoSceneNrvDeactive LogoSceneNrvDeactive::sInstance;
}

LogoScene::LogoScene()
    : Scene("LogoScene"),
      mStrapLayout(), mLogoFader(), mInputBuffered(false) {
    MR::offUseVFilter();
    MR::offUseVFilterDirect();
}

LogoScene::~LogoScene() {
    MR::onUseVFilter();
    MR::onUseVFilterDirect();
}

void LogoScene::init() {
        initNerve(GET_NERVE(LogoScene, LogoSceneNrvStrapFadein));

    SceneFunction::createHioBasicNode(this);
    SceneFunction::initForNameObj();
    MR::createSceneObj(SceneObj_CameraContext);
    MR::createSceneObj(SceneObj_NameObjGroup);
    initLayout();
}

void LogoScene::update() {
    updateNerve();
    SceneFunction::executeMovementList();
}

void LogoScene::calcAnim() {
    SceneFunction::executeCalcAnimList();
    SceneFunction::executeCalcViewAndEntryList2D();
}

void LogoScene::draw() const {
    MR::drawInit();

    GXColor color;
    color.r = 0;
    color.g = 0;
    color.b = 0;
    color.a = 255;

    MR::fillScreen(color);
    MR::clearZBuffer();
    MR::drawInitFor2DModel();


    CategoryList::drawOpa(MR::DrawBufferType_Model3DFor2D);
    CategoryList::drawXlu(MR::DrawBufferType_Model3DFor2D);
    CategoryList::execute(MR::DrawType_Layout);
    CategoryList::execute(MR::DrawType_EffectDraw2D);
    CategoryList::execute(MR::DrawType_EffectDrawFor2DModel);
    CategoryList::execute(MR::DrawType_CometScreenFilter);
    CategoryList::execute(MR::DrawType_WipeLayout);
}

bool LogoScene::isDisplayStrapRemineder() const {
    return isNerve(GET_NERVE(LogoScene, LogoSceneNrvDeactive)) ||
           isNerve(GET_NERVE(LogoScene, LogoSceneNrvStrapFadein)) || isNerve(GET_NERVE(LogoScene, LogoSceneNrvStrapDisplay)) ||
           isNerve(GET_NERVE(LogoScene, LogoSceneNrvStrapFadeout)) || isNerve(GET_NERVE(LogoScene, LogoSceneNrvWaitReadDoneSystemArchive));
}


void LogoScene::exeStrapFadein() {
    if (MR::isFirstStep(this)) {
        MR::startAnim(mStrapLayout, "Strap", 0);
        MR::setAnimFrameAndStop(mStrapLayout, 0.0f, 0);
    }

    if (tryFadeinLayout(mStrapLayout)) {
        setNerve(GET_NERVE(LogoScene, LogoSceneNrvStrapDisplay));
    }
}

void LogoScene::exeStrapDisplay() {
    if (MR::isFirstStep(this)) {
        MR::startAnim(mStrapLayout, "Strap", 0);
#ifdef GALAXY_NATIVE
        // A --stage startup goes straight to its galaxy: the reminder leaves at its earliest step, as if pressed.
        namespace Launch = PetariNative::App::LaunchStage;
        const Launch::StartupStatus status = Launch::startupStatus();
        if (status == Launch::StartupStatus::PendingCatalog || status == Launch::StartupStatus::PendingFileLoad) {
            mInputBuffered = true;
        }
#endif
    }

    if (MR::isStep(this, STRAP_DISPLAY_MAX_FRAME - 1)) {
        MR::setAnimRate(mStrapLayout, 0.0f, 0);
    }

    if (MR::isLessStep(this, 98)) {
        if (!mInputBuffered && MR::testCorePadTriggerAnyWithoutHome(WPAD_CHAN0)) {
            mInputBuffered = true;
        }
    } else if (mInputBuffered || MR::testCorePadTriggerAnyWithoutHome(WPAD_CHAN0) ||
               MR::isGreaterEqualStep(this, STRAP_DISPLAY_MAX_FRAME)) {
        setNerve(GET_NERVE(LogoScene, LogoSceneNrvStrapFadeout));
    }
}





void LogoScene::initLayout() {
    mStrapLayout = MR::createSimpleLayout("ストラップ着用画面", "WiiRemoteStrap", 1);
    mStrapLayout->kill();

    mLogoFader = new LogoFader("ロゴフェーダ");
    mLogoFader->initWithoutIter();
    mLogoFader->setBlank();
    mLogoFader->mMaxStep = 50;
    mLogoFader->appear();
    MR::connectToSceneLayout(mLogoFader);

}

bool LogoScene::tryFadeinLayout(LayoutActor* pActor) {
    if (MR::isFirstStep(this)) {
        mLogoFader->startFadeIn();
        pActor->appear();
    }

    if (mLogoFader->isFadeEnd()) {
        return true;
    }

    return false;
}

bool LogoScene::tryFadeoutLayout(LayoutActor* pActor) {
    if (MR::isFirstStep(this)) {
        mLogoFader->startFadeOut();
    }

    if (mLogoFader->isFadeEnd()) {
        pActor->kill();
        return true;
    }

    return false;
}

void LogoScene::exeMountGameData() {
    if (MR::isFirstStep(this)) {
        GameSequenceFunction::startPreLoadSaveDataSequence();
    }

    if (GameSequenceFunction::isActiveSaveDataHandleSequence()) {
        return;
    }

    setNerve(GET_NERVE(LogoScene, LogoSceneNrvDeactive));
}
