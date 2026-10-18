#include "Game/Scene/GameScene.hpp"
#include "Game/Util.hpp"
#include "Game/AudioLib/AudSceneMgr.hpp"
#include "Game/AudioLib/AudWrap.hpp"
#include "Game/LiveActor/AllLiveActorGroup.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Map/LightFunction.hpp"
#include "Game/Map/SleepControllerHolder.hpp"
#include "Game/MapObj/StarPieceDirector.hpp"
#include "Game/NPC/EventDirector.hpp"
#include "Game/NPC/NPCFunction.hpp"
#include "Game/NPC/TalkDirector.hpp"
#include "Game/Scene/GameSceneFunction.hpp"
#include "Game/Scene/GameScenePauseControl.hpp"
#include "Game/Scene/GameSceneScenarioOpeningCameraState.hpp"
#include "Game/Scene/SceneFunction.hpp"
#include "Game/Scene/SceneNameObjMovementController.hpp"
#include "Game/Screen/CometRetryButton.hpp"
#include "Game/Screen/GamePauseSequence.hpp"
#include "Game/Screen/GameStageClearSequence.hpp"
#include "Game/Screen/LensFlare.hpp"
#include "Game/Screen/MoviePlayingSequence.hpp"
#include "Game/Screen/OdhConverter.hpp"
#include "Game/Screen/ScreenAlphaCapture.hpp"
#include "Game/System/GalaxyMapController.hpp"
#include "Game/System/GameSequenceFunction.hpp"
#include "Game/Util/CameraUtil.hpp"
#include "Game/Util/DemoUtil.hpp"
#include "Game/Util/DrawUtil.hpp"
#include "Game/Util/EventUtil.hpp"
#include "Game/Util/FileUtil.hpp"
#include "Game/Util/JMapIdInfo.hpp"
#include "Game/Util/LightUtil.hpp"
#include "Game/Util/NerveUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/SceneUtil.hpp"
#include "Game/Util/ScreenUtil.hpp"
#include "Game/Util/SequenceUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include "Game/Util/StarPointerUtil.hpp"
#include "Game/Util/SystemUtil.hpp"
#include <JSystem/J3DGraphBase/J3DSys.hpp>

namespace {
};  // namespace

namespace NrvGameScene {
    NEW_NERVE(GameSceneScenarioOpeningCamera, GameScene, ScenarioOpeningCamera);
    NEW_NERVE(GameSceneScenarioStarter, GameScene, ScenarioStarter);
    NEW_NERVE(GameSceneAction, GameScene, SceneAction);
    NEW_NERVE(GameScenePauseMenu, GameScene, PauseMenu);
    NEW_NERVE(GameScenePowerStarGet, GameScene, PowerStarGet);
    NEW_NERVE(GameSceneGrandStarGet, GameScene, GrandStarGet);
    NEW_NERVE(GameSceneGameOver, GameScene, GameOver);
    NEW_NERVE(GameSceneCometRetryAfterMiss, GameScene, CometRetryAfterMiss);
    NEW_NERVE(GameSceneSaveAfterGameOver, GameScene, SaveAfterGameOver);
    NEW_NERVE(GameScenePlayMovie, GameScene, PlayMovie);
    NEW_NERVE(GameSceneTimeUp, GameScene, TimeUp);
    NEW_NERVE(GameSceneGalaxyMap, GameScene, GalaxyMap);
    NEW_NERVE(GameSceneStaffRoll, GameScene, StaffRoll);
};  // namespace NrvGameScene

GameScene::GameScene()
    : Scene("GameScene"), _14(0), mScenarioCamera(nullptr), mPauseCtrl(nullptr), mPauseSeq(nullptr), mStageClearSeq(nullptr), mDraw3D(true), _29(1) {
}

GameScene::~GameScene() {
    MR::destroySceneMessage();
    NPCFunction::deleteNPCData();
    MR::onStarPointerSceneOut();
}


void GameScene::start() {
    AudWrap::getSceneMgr()->startScene();

    if (MR::isGlobalTimerEnd()) {
        MR::forceCloseWipeCircle();
    } else if (!MR::hasRetryGalaxySequence()) {
        startStagePlayFirst();
    } else {
        startStagePlayRetry();
    }
}

void GameScene::update() {
    mPauseCtrl->updateNerve();
    updateNerve();

    bool isTimeUp = MR::isGlobalTimerEnd() && !isNerve(GET_NERVE(GameScene, GameSceneTimeUp)) && MR::isGreaterEqualStep(this, 2);

    if (isTimeUp) {
        setNerve(GET_NERVE(GameScene, GameSceneTimeUp));
    }
}

void GameScene::draw() const {
    MR::drawInit();
    LightFunction::initLightRegisterAll();
    drawOdhCapture();
    CategoryList::execute(MR::DrawType_MiiFaceIcon);
    CategoryList::execute(MR::DrawType_MiiFaceNew);
    drawMirror();
    draw3D();
    draw2D();
    MR::reinitGX();
}

void GameScene::calcAnim() {
    if (isPlayMovie()) {
        SceneFunction::executeCalcAnimListOnPlayingMovie();
    } else {
        SceneFunction::executeCalcAnimList();
    }

    if (!isNerve(GET_NERVE(GameScene, GameSceneTimeUp))) {
        CategoryList::execute(MR::CalcAnimType_AnimParticleIgnorePause);
    }

    SceneFunction::executeCalcViewAndEntryList();
}

void GameScene::notifyEndScenarioStarter() {
    setNerve(GET_NERVE(GameScene, GameSceneAction));
}


void GameScene::requestStartGameOverDemo() {
    if (!isNerve(GET_NERVE(GameScene, GameSceneGameOver))) {
        setNerve(GET_NERVE(GameScene, GameSceneGameOver));
    }
}

void GameScene::requestEndGameOverDemo() {
    setNerve(GET_NERVE(GameScene, GameSceneSaveAfterGameOver));
}


void GameScene::requestPowerStarGetDemo() {
    setNerve(GET_NERVE(GameScene, GameScenePowerStarGet));
}

void GameScene::requestGrandStarGetDemo() {
    setNerve(GET_NERVE(GameScene, GameSceneGrandStarGet));
}

void GameScene::setNerveAfterPauseMenu() {
    setNerve(GET_NERVE(GameScene, GameSceneAction));
}

bool GameScene::isExecScenarioOpeningCamera() const {
    return isNerve(GET_NERVE(GameScene, GameSceneScenarioOpeningCamera));
}

bool GameScene::isExecScenarioStarter() const {
    return isNerve(GET_NERVE(GameScene, GameSceneScenarioStarter));
}

bool GameScene::isExecStageClearDemo() const {
    return isNerve(GET_NERVE(GameScene, GameScenePowerStarGet)) || isNerve(GET_NERVE(GameScene, GameSceneGrandStarGet));
}

void GameScene::exeScenarioOpeningCamera() {
    mScenarioCamera->update();
    SceneFunction::movementStopSceneController();
    SceneFunction::executeMovementList();

    if (mScenarioCamera->isDone()) {
        if (!MR::isBeginScenarioStarter()) {
            MR::activateDefaultGameLayout();
            MR::tryFrameToScreenCinemaFrame();
            MR::stopSubBGM(0);
            MR::stopStageBGM(0);
            MR::startStageBGMFromStageName("Game", MR::getCurrentStageName(), MR::getCurrentScenarioNo());
            setNerve(GET_NERVE(GameScene, GameSceneAction));
        } else {
            setNerve(GET_NERVE(GameScene, GameSceneScenarioStarter));
        }
    }
}





void GameScene::initSequences() {
    GameStageClearSequence* pStageClearSeq = new GameStageClearSequence();
    pStageClearSeq->initWithoutIter();
    mStageClearSeq = pStageClearSeq;

    GamePauseSequence* pPauseSeq = new GamePauseSequence();
    pPauseSeq->initWithoutIter();
    mPauseSeq = pPauseSeq;

    mScenarioCamera = new GameSceneScenarioOpeningCameraState();
    mPauseCtrl = new GameScenePauseControl(this);
    mPauseCtrl->registerNervePauseMenu(GET_NERVE(GameScene, GameScenePauseMenu));

    mPauseSeq->initWindowMenu(MR::Functor(mPauseCtrl, &GameScenePauseControl::requestPauseMenuOff));
}

void GameScene::initEffect() {
    if (MR::isEqualStageName("CosmosGardenGalaxy")) {
        SceneFunction::initEffectSystem(5120, 384);
    } else if (MR::isEqualStageName("KoopaBattleVs1Galaxy")) {
        SceneFunction::initEffectSystem(6144, 512);
    } else if (MR::isEqualStageName("AstroGalaxy")) {
        SceneFunction::initEffectSystem(3072, 512);
    } else {
        SceneFunction::initEffectSystem(3072, 256);
    }
}

void GameScene::drawMirror() const {
    if (isDrawMirror()) {
        PSMTXCopy(MR::getMirrorCameraViewMtx(), j3dSys.mViewMtx);
        MR::loadProjectionMtx();
        CategoryList::entryDrawBufferMirror();
        GXSetAlphaUpdate(GX_FALSE);
        GXSetColorUpdate(GX_TRUE);
        CategoryList::drawOpa(MR::DrawBufferType_MirrorMapObj);
        CategoryList::drawXlu(MR::DrawBufferType_MirrorMapObj);
        CategoryList::execute(MR::DrawType_CaptureScreenIndirect);
        MR::clearZBuffer();
        GXColor c = {0, 0, 255, 0};
        MR::fillScreen(c);
    }
}

// inline


void GameScene::draw2D() const {
    if (isPlayMovie()) {
        SceneFunction::executeDrawList2DMovie();
    } else {
        SceneFunction::executeDrawList2DNormal();
    }

    CategoryList::execute(MR::DrawType_CameraCover);
    CategoryList::execute(MR::DrawType_LayoutOnPause);
    mPauseSeq->draw();
    MR::drawInitFor2DModel();
    CategoryList::drawOpa(MR::DrawBufferType_0x26);
    CategoryList::drawXlu(MR::DrawBufferType_0x26);
    CategoryList::execute(MR::DrawType_WipeLayout);
}

bool GameScene::isValidScenarioOpeningCamera() const {
    return MR::hasStartAnimCamera() != false;
}


void GameScene::startStagePlayFirst() {
    if (isValidScenarioOpeningCamera()) {
        setNerve(GET_NERVE(GameScene, GameSceneScenarioOpeningCamera));

        if (!MR::isBeginScenarioStarter()) {
            MR::stopSubBGM(0);
            MR::stopStageBGM(0);
            MR::startStageBGMFromStageName("Game", MR::getCurrentStageName(), MR::getCurrentScenarioNo());
        }
    } else if (MR::isBeginScenarioStarter()) {
        MR::forceToBlankCinemaFrame();
        MR::forceOpenWipeFade();
        setNerve(GET_NERVE(GameScene, GameSceneScenarioStarter));
    } else {
        if (MR::isStageBeginFadeWipe()) {
            MR::openWipeFade(180);
        } else if (MR::isStageBeginTitleWipe()) {
            MR::openWipeFade(30);
        } else if (MR::isStageBeginWithoutWipe()) {
            MR::forceOpenWipeFade();
        } else {
            MR::openWipeCircle();
        }

        MR::stopSubBGM(0);
        MR::stopStageBGM(0);
        MR::startStageBGMFromStageName("Game", MR::getCurrentStageName(), MR::getCurrentScenarioNo());
        MR::executeOnWelcomeAndRetry();
        setNerve(GET_NERVE(GameScene, GameSceneAction));
    }
}

void GameScene::startStagePlayRetry() {
    MR::openWipeCircle();

    if (MR::isEqualStageName("SurfingLv1Galaxy") && MR::getPlayerRestartIdInfo()->_0 == 1) {
        MR::stopSubBGM(0);
        MR::stopStageBGM(0);
    } else if (MR::isEqualStageName("SurfingLv2Galaxy") && MR::getPlayerRestartIdInfo()->_0 == 1) {
        MR::stopSubBGM(0);
        MR::stopStageBGM(0);
    } else {
        MR::stopSubBGM(0);
        MR::stopStageBGM(0);
        MR::startStageBGMFromStageName("Game", MR::getCurrentStageName(), MR::getCurrentScenarioNo());
    }

    MR::executeOnWelcomeAndRetry();
    setNerve(GET_NERVE(GameScene, GameSceneAction));
}




bool GameScene::isDrawMirror() const {
    if (!MR::isExistMirrorCamera()) {
        return false;
    }

    if (isPlayMovie()) {
        return false;
    }

    return MR::isPlayerInAreaObj("MirrorArea");
}

void GameScene::stageClear() {
    if (MR::isFirstStep(this)) {
        if (isNerve(GET_NERVE(GameScene, GameScenePowerStarGet))) {
            mStageClearSeq->startPowerStarGetDemo();
        } else if (isNerve(GET_NERVE(GameScene, GameSceneGrandStarGet))) {
            mStageClearSeq->startGrandStarGetDemo();
        }
    }

    mStageClearSeq->movement();
    SceneFunction::movementStopSceneController();
    SceneFunction::executeMovementList();
}








