#include "Game/Scene/LogoScene.hpp"
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


namespace {
    const s32 FADEINOUT_FRAME = 30;
    const s32 CENSORSHIP_DISPLAY_FRAME = 180;
    const s32 STRAP_DISPLAY_MIN_FRAME = 450;
    const s32 STRAP_DISPLAY_MAX_FRAME = 1200;


    NEW_NERVE(LogoSceneStrapFadein, LogoScene, StrapFadein);
    NEW_NERVE(LogoSceneStrapDisplay, LogoScene, StrapDisplay);
    NEW_NERVE(LogoSceneStrapFadeout, LogoScene, StrapFadeout);
    NEW_NERVE(LogoSceneMountGameData, LogoScene, MountGameData);
    NEW_NERVE(LogoSceneWaitReadDoneSystemArchive, LogoScene, WaitReadDoneSystemArchive);
    NEW_NERVE(LogoSceneDeactive, LogoScene, Deactive);
};  // namespace

LogoScene::LogoScene()
    : Scene("LogoScene"),
      mStrapLayout(), mLogoFader() {
    MainLoopFramework::getManager()->mUseVFilter = false;
    MainLoopFramework::getManager()->mUseAlpha = false;
}

LogoScene::~LogoScene() {
    MainLoopFramework::getManager()->mUseVFilter = true;
    MainLoopFramework::getManager()->mUseAlpha = true;
}

void LogoScene::init() {
        initNerve(GET_NERVE_ANON(LogoSceneStrapFadein));

    SceneFunction::createHioBasicNode(this);
    SceneFunction::initForNameObj();
    MR::createSceneObj(SceneObj_CameraContext);
    MR::createSceneObj(SceneObj_NameObjGroup);
    initLayout();
}

void LogoScene::update() {
    GameSystemFunction::restartControllerLeaveWatcher();
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
    return isNerve(GET_NERVE_ANON(LogoSceneDeactive)) ||
           isNerve(GET_NERVE_ANON(LogoSceneStrapFadein)) || isNerve(GET_NERVE_ANON(LogoSceneStrapDisplay)) ||
           isNerve(GET_NERVE_ANON(LogoSceneStrapFadeout)) || isNerve(GET_NERVE_ANON(LogoSceneWaitReadDoneSystemArchive));
}


void LogoScene::exeStrapFadein() {
    if (MR::isFirstStep(this)) {
        MR::startAnim(mStrapLayout, "Strap", 0);
        MR::setAnimFrameAndStop(mStrapLayout, 0.0f, 0);

        mLogoFader->mMaxStep = FADEINOUT_FRAME;
    }

    if (tryFadeinLayout(mStrapLayout)) {
        setNerve(GET_NERVE_ANON(LogoSceneStrapDisplay));
    }
}

void LogoScene::exeStrapDisplay() {
    if (MR::isFirstStep(this)) {
        MR::startAnim(mStrapLayout, "Strap", 0);
    }

    if (MR::isStep(this, STRAP_DISPLAY_MAX_FRAME - 1)) {
        MR::setAnimRate(mStrapLayout, 0.0f, 0);
    }

    if (MR::isGreaterStep(this, STRAP_DISPLAY_MIN_FRAME) &&
        (MR::testCorePadTriggerAnyWithoutHome(WPAD_CHAN0) || MR::isGreaterEqualStep(this, STRAP_DISPLAY_MAX_FRAME))) {
        setNerve(GET_NERVE_ANON(LogoSceneStrapFadeout));
    }
}





void LogoScene::initLayout() {
    mStrapLayout = MR::createSimpleLayout("ストラップ着用画面", "WiiRemoteStrap", 1);
    mStrapLayout->kill();

    mLogoFader = new LogoFader("ロゴフェーダ");
    mLogoFader->initWithoutIter();
    mLogoFader->setBlank();
    mLogoFader->appear();
    MR::connectToSceneLayout(mLogoFader);

}

bool LogoScene::tryFadeinLayout(LayoutActor* pActor) {
    if (MR::isFirstStep(this)) {
        pActor->appear();
    }

    if (tryFadeinLayout()) {
        return true;
    }

    return false;
}

bool LogoScene::tryFadeoutLayout(LayoutActor* pActor) {
    if (tryFadeoutLayout()) {
        pActor->kill();
        return true;
    }

    return false;
}

