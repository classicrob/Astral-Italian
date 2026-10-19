#include "Game/Util/BgmUtil.hpp"
#include "Game/Util/PadRecorder.hpp"
#include "Game/AudioLib/AudStageUseResourceList.hpp"
#include "Game/System/AudSystemWrapper.hpp"
#include "Game/System/GameSystemObjHolder.hpp"
#include "Game/System/GameDataFunction.hpp"
#include "Game/System/WorldMapFunction.hpp"
#include "Game/System/WorldMapResultCommand.hpp"
#include "Game/System/WorldMapOwner.hpp"
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
    bool clearPadMode() NO_INLINE;
    bool clearPadMode() {
        if (GameSequenceFunction::isPadModeDreamer()) {
            GameSequenceFunction::clearPadModeDreamer();
            return true;
        }
        if (GameSequenceFunction::isPadModeSuperDreamer()) {
            GameSequenceFunction::clearPadModeSuperDreamer();
        }
        return false;
    }
};  // namespace

namespace NrvGameScene {
    class GameSceneScenarioOpeningCamera : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            GameScene* actor = reinterpret_cast< GameScene* >(pSpine->mExecutor);
            actor->exeScenarioOpeningCamera();
        }
        static GameSceneScenarioOpeningCamera sInstance;
    };
    GameSceneScenarioOpeningCamera GameSceneScenarioOpeningCamera::sInstance ATTRIBUTE_WEAK;
    class GameSceneScenarioStarter : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            GameScene* actor = reinterpret_cast< GameScene* >(pSpine->mExecutor);
            actor->exeAction();
        }
        static GameSceneScenarioStarter sInstance;
    };
    GameSceneScenarioStarter GameSceneScenarioStarter::sInstance ATTRIBUTE_WEAK;
    class GameSceneAction : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            GameScene* actor = reinterpret_cast< GameScene* >(pSpine->mExecutor);
            actor->exeAction();
        }
        static GameSceneAction sInstance;
    };
    GameSceneAction GameSceneAction::sInstance ATTRIBUTE_WEAK;
    class GameScenePauseMenu : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            GameScene* actor = reinterpret_cast< GameScene* >(pSpine->mExecutor);
            MR::isFirstStep(actor); actor->mPauseSeq->movement(); CategoryList::execute(static_cast< MR::MovementType >(0x11));
        }
        static GameScenePauseMenu sInstance;
    };
    GameScenePauseMenu GameScenePauseMenu::sInstance ATTRIBUTE_WEAK;
    class GameScenePowerStarGet : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            GameScene* actor = reinterpret_cast< GameScene* >(pSpine->mExecutor);
            actor->stageClear();
        }
        static GameScenePowerStarGet sInstance;
    };
    GameScenePowerStarGet GameScenePowerStarGet::sInstance ATTRIBUTE_WEAK;
    class GameSceneGrandStarGet : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            GameScene* actor = reinterpret_cast< GameScene* >(pSpine->mExecutor);
            actor->stageClear();
        }
        static GameSceneGrandStarGet sInstance;
    };
    GameSceneGrandStarGet GameSceneGrandStarGet::sInstance ATTRIBUTE_WEAK;
    class GameSceneGameOver : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            GameScene* actor = reinterpret_cast< GameScene* >(pSpine->mExecutor);
            actor->exeAction();
        }
        static GameSceneGameOver sInstance;
    };
    GameSceneGameOver GameSceneGameOver::sInstance ATTRIBUTE_WEAK;
    class GameSceneSaveAfterGameOver : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            GameScene* actor = reinterpret_cast< GameScene* >(pSpine->mExecutor);
            actor->exeSaveAfterGameOver();
        }
        static GameSceneSaveAfterGameOver sInstance;
    };
    GameSceneSaveAfterGameOver GameSceneSaveAfterGameOver::sInstance ATTRIBUTE_WEAK;
    class GameSceneSuperDreamerClear : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            GameScene* actor = reinterpret_cast< GameScene* >(pSpine->mExecutor);
            CategoryList::execute(static_cast< MR::MovementType >(0xa)); CategoryList::execute(static_cast< MR::MovementType >(0x2d)); CategoryList::execute(static_cast< MR::MovementType >(0xe)); CategoryList::execute(static_cast< MR::MovementType >(0xf));
        }
        static GameSceneSuperDreamerClear sInstance;
    };
    GameSceneSuperDreamerClear GameSceneSuperDreamerClear::sInstance ATTRIBUTE_WEAK;
};  // namespace NrvGameScene

GameScene::GameScene()
    : Scene("GameScene"), _14(0), mScenarioCamera(nullptr), mPauseCtrl(nullptr), mPauseSeq(nullptr), mStageClearSeq(nullptr), mDraw3D(true), _29(0), _2A(0) {
}

GameScene::~GameScene() {
    MR::stopReplayPad();
    MR::endToCaptureScreen("Camera");
    MR::onStarPointerSceneOut();
}


void GameScene::start() {
    _2A = MR::isIslandFleetGalaxy1FirstTime();
    AudWrap::getSceneMgr()->resetForScene();
    if (GameSequenceFunction::isPadMode()) {
        void* data;
        s32 size;
        MR::getDreamerPadData(&data, &size);
        MR::prepareReplay(static_cast< const char* >(data), size);
        MR::startReplayPad();
    }
    MR::forceOpenSystemWipeFade();
    MR::startStarPointerModeGame(this);
    if (!isValidScenarioOpeningCamera() && !MR::isBeginScenarioStarter()) {
        startWorldMapBgm();
    }
    setNerve(GET_NERVE(GameScene, GameSceneAction));
}

void GameScene::update() {
    mPauseCtrl->updateNerve();
    updateNerve();
    CategoryList::execute(static_cast< MR::MovementType >(0x1a));
}

void GameScene::draw() const {
    MR::drawInit();
    LightFunction::initLightRegisterAll();
    CategoryList::execute(MR::DrawType_MiiFaceIcon);
    CategoryList::execute(MR::DrawType_MiiFaceNew);
    drawMirror();
    draw3D();
    draw2D();
    MR::reinitGX();
}

void GameScene::calcAnim() {
    if (MR::isStageWorldMap()) {
        MR::calcAnimWorldMapBig();
        if (MR::getSceneObj< WorldMapOwner >(0x33)->isGalaxyMap()) {
            WorldMapFunction::calcAnimMini();
        }
    }
    bool active = false;
    if (MR::isExistSceneObj(0x34) && MR::getSceneObj< WorldMapResultCommand >(0x34)->isActive()) {
        active = true;
    }
    if (active) {
        MR::getSceneObj< WorldMapResultCommand >(0x34)->calcAnimDraw();
        SceneFunction::executeCalcAnimListWithWorldMap();
    } else {
        SceneFunction::executeCalcAnimList();
    }
    CategoryList::execute(static_cast< MR::CalcAnimType >(0x13));
    calcViewAndEntry();
}

void GameScene::notifyEndScenarioStarter() {
    setNerve(GET_NERVE(GameScene, GameSceneAction));
}


void GameScene::requestStartGameOverDemo() {
    if (!MR::isStageStaffRoll() && !isNerve(GET_NERVE(GameScene, GameSceneGameOver))) {
        setNerve(GET_NERVE(GameScene, GameSceneGameOver));
    }
}

void GameScene::requestEndGameOverDemo() {
    if (!clearPadMode() && !MR::isStageStaffRoll()) {
        setNerve(GET_NERVE(GameScene, GameSceneSaveAfterGameOver));
    }
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
    exeAction();
    MR::setSeVolumeSetLevel(9, 0);
    if (mScenarioCamera->isDone()) {
        if (!MR::isBeginScenarioStarter()) {
            MR::activateDefaultGameLayout();
            MR::appearDestroyCounter();
            MR::tryFrameToScreenCinemaFrame();
            startWorldMapBgm();
            MR::executeOnWelcomeAndRetry();
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

    mPauseSeq->initWindowMenu(MR::Functor_Inline(mPauseCtrl, &GameScenePauseControl::requestPauseMenuOff));
}

// NONMATCHING: regalloc (FP order/grouping checked: no FP instructions).
void GameScene::initEffect() {
    struct EffectInfo { const char* name; u32 particles; u32 emitters; };
    static const EffectInfo info[] = {
        {"DummyGalaxy", 3072, 512},
        {"StarCreekGalaxy", 3072, 512},
        {"MokumokuValleyGalaxy", 3072, 512},
        {"VsKoopaGravityGalaxy", 3072, 512},
    };
    const EffectInfo* found = nullptr;
    for (u32 i = 0; i < 4; i++) {
        if (MR::isEqualStageName(info[i].name)) {
            found = &info[i];
            break;
        }
    }
    if (found) {
        SceneFunction::initEffectSystem(found->particles, found->emitters);
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
        CategoryList::drawOpa(static_cast< MR::DrawBufferType >(0x2c));
        CategoryList::drawXlu(static_cast< MR::DrawBufferType >(0x2c));
        CategoryList::execute(MR::DrawType_CaptureScreenIndirect);
        MR::clearZBuffer();
        GXColor c = {0, 0, 255, 0};
        MR::fillScreen(c);
    }
}

// inline


void GameScene::draw2D() const {
    SceneFunction::executeDrawList2DNormal();
    CategoryList::execute(static_cast< MR::DrawType >(0x42));
    CategoryList::execute(static_cast< MR::DrawType >(0x50));
    CategoryList::execute(static_cast< MR::DrawType >(0x51));
    if (MR::isExistSceneObj(0x89)) {
        MR::loadViewMtx();
        f32 fovy = MR::getFovy();
        MR::setFovy(15.0f);
        MR::loadProjectionMtx();
        CategoryList::drawOpa(static_cast< MR::DrawBufferType >(0x26));
        CategoryList::drawXlu(static_cast< MR::DrawBufferType >(0x26));
        MR::setFovy(fovy);
    }
    CategoryList::execute(static_cast< MR::DrawType >(0x5c));
    CategoryList::execute(static_cast< MR::DrawType >(0x48));
    mPauseSeq->draw();
    MR::drawInitFor2DModel();
    CategoryList::drawOpa(static_cast< MR::DrawBufferType >(0x2b));
    CategoryList::drawXlu(static_cast< MR::DrawBufferType >(0x2b));
    CategoryList::execute(static_cast< MR::DrawType >(0x4a));
}

// NONMATCHING: equivalent branch block order
bool GameScene::isValidScenarioOpeningCamera() const {
    if (!MR::hasStartAnimCamera()) {
        return false;
    }
    return !_2A;
}


void GameScene::startStagePlayFirst() {
    if (isValidScenarioOpeningCamera()) {
        setNerve(GET_NERVE(GameScene, GameSceneScenarioOpeningCamera));
    } else if (MR::isBeginScenarioStarter()) {
        MR::forceToBlankCinemaFrame();
        MR::forceOpenWipeFade();
        setNerve(GET_NERVE(GameScene, GameSceneScenarioStarter));
    } else {
        startWorldMapBgm();
        MR::executeOnWelcomeAndRetry();
    }
}

void GameScene::startStagePlayRetry() {
    startWorldMapBgm();
    MR::executeOnWelcomeAndRetry();
}




// NONMATCHING: equivalent branch block order
bool GameScene::isDrawMirror() const {
    if (MR::isExistMirrorCamera()) {
        return MR::isPlayerInAreaObj("MirrorArea");
    }
    return false;
}

void GameScene::stageClear() {
    if (MR::isFirstStep(this)) {
        if (isNerve(GET_NERVE(GameScene, GameScenePowerStarGet))) {
            mStageClearSeq->startPowerStarGetDemo();
        } else if (isNerve(GET_NERVE(GameScene, GameSceneGrandStarGet))) {
            mStageClearSeq->startGrandStarGetDemo();
        }
        if (MR::isProductGameScene()) {
            if (GameSequenceFunction::isClearedIslandFleetGalaxyFirstTime()) {
                AudWrap::getStageUseResourceList()->loadIslandFleetClear();
            } else if (GameSequenceFunction::isClearedVsKoopaLv3GalaxyFirstTime()) {
                AudWrap::getStageUseResourceList()->loadEndingClear();
            } else if (GameSequenceFunction::isClearedVsKoopaLv3Galaxy1()) {
                AudWrap::getStageUseResourceList()->loadKoopaClear();
            } else {
                AudWrap::getStageUseResourceList()->loadGalaxyClear();
            }
        }
    }
    mStageClearSeq->movement();
    exeAction();
}









namespace GameSequenceFunction {
    bool isNeedMoviePlayerForStorySequenceEvent();
};  // namespace GameSequenceFunction

void GameScene::init() {
    MR::isStageFileSelect();
    SceneFunction::createHioBasicNode(this);
    SceneFunction::startStageFileLoad();
    bool changedPlayer = MR::requestChangeArchivePlayer(!MR::isPlayerLuigi());
    bool loadPlayer = true;
    if (!changedPlayer && MR::isPlayerArchiveChangePending()) {
        loadPlayer = false;
    }
    initNerve(GET_NERVE(GameScene, GameSceneScenarioOpeningCamera), 0);
    SceneFunction::initForNameObj();
    SceneFunction::initForLiveActor();
    initEffect();
    MR::createSceneObj(0x18);
    MR::createSceneObj(0x19);
    MR::createSceneObj(0x3b);
    MR::createSceneObj(0x15);
    MR::createSceneObj(0x11);
    MR::createSceneObj(0x12);
    MR::createSceneObj(0x13);
    MR::createSceneObj(0x14);
    MR::createSceneObj(0x2c);
    MR::createSceneObj(0x1b);
    MR::createSceneObj(0x28);
    MR::createSceneObj(0x29);
    MR::createScreenAlphaSceneObj(0, 1.0f);
    MR::createSceneObj(0x72);
    MR::createSceneObj(0x7a);
    MR::createSceneObj(0x7e);
    MR::createSceneObj(0x7f);
    MR::createSceneObj(0x90);
    if (MR::isStageMarioFaceShipOrWorldMap()) {
        MR::createSceneObj(0x37);
    }
    if (MR::isStageStaffRoll() || MR::isStageMarioFaceShip()) {
        MR::createSceneObj(0x98);
    }
    SceneFunction::waitDoneStageFileLoad();
    if (changedPlayer) {
        MR::waitEndChangeArchivePlayer();
    }
    SceneFunction::startActorFileLoadCommon();
    if (MR::isStageWorldMap()) {
        WorldMapFunction::mountWorldMapResources();
    }
    if (MR::isScenarioSelecting()) {
        MR::receiveAllRequestedFile();
        MR::getGameSystemObjHolder()->mAudioSystem->setStageResourceLoading(loadPlayer);
        while (!GameSceneFunction::isLoadDoneScenarioWaveData()) {
        }
    }
    MR::suspendAsyncExecuteThread("シーン初期化");
    if (!MR::isScenarioDecided()) {
        MR::receiveAllRequestedFile();
        MR::getGameSystemObjHolder()->mAudioSystem->resetStageResources();
    } else {
        SceneFunction::initAfterScenarioSelected();
        SceneFunction::startActorFileLoadScenario();
        SceneFunction::startActorFileLoadPostCreation();
        MR::createSceneObj(0x4);
        MR::createSceneObj(0x3);
        MR::initSceneMessage();
        MR::createSceneObj(0x3a);
        MR::createSceneObj(0x3d);
        MR::createSceneObj(0x44);
        MR::createSceneObj(0x25);
        MR::createSceneObj(0x2a);
        MR::createSceneObj(0x81);
        if (MR::isStageScoreAttack()) {
            MR::createSceneObj(0x35);
        }
        if (MR::isStageMarioFaceShip()) {
            MR::createSceneObj(0x97);
            MR::createSceneObj(0x93);
            MR::createSceneObj(0x95);
            MR::createSceneObj(0x96);
        }
        GameSequenceFunction::setupNameEntryTableIfRetryGalaxy();
        LightFunction::initLightData();
        initSequences();
        if (MR::isStageMarioFaceShipNormal()) {
            MR::setCurrentPlacementZoneId(0);
            MR::createSceneObj(0x34);
            MR::clearCurrentPlacementZoneId();
        }
        MR::createStarPiece();
        SceneFunction::startActorPlacement();
        if (MR::isStageStoryBook()) {
            MR::createSceneObj(0x89);
        }
        if (!MR::isStageFileSelect() && !MR::isStageWorldMap()) {
            MR::createSceneObj(0x62);
        }
        if (MR::isStageWorldMap()) {
            MR::setCurrentPlacementZoneId(0);
            MR::createSceneObj(0x33);
            MR::clearCurrentPlacementZoneId();
        }
        GameSceneFunction::loadScenarioWaveData(loadPlayer);
        MR::completeCameraParameters();
        MR::initStarPointerGameScene();
        MR::initEventSystemAfterPlacement();
        MR::endInitLiveActorSystemInfo();
        MR::setInitializeStateAfterPlacement();
        MR::callMethodAllSceneNameObj(&NameObj::initAfterPlacement);
        AwakeControlFunc::initSyncSleepController();
        if (MR::isStageWorldMap()) {
            MR::deactivateDefaultGameLayout();
        }
        while (!GameSceneFunction::isLoadDoneScenarioWaveData()) {
        }
        MR::getGameSystemObjHolder()->mAudioSystem->finishStageResourceLoading();
    }
}


void GameScene::draw3D() const {
    if (!mDraw3D) {
        return;
    }
    CategoryList::execute(static_cast< MR::DrawType >(0x1f));
    MR::loadViewMtx();
    MR::loadProjectionMtx();
    MR::setDefaultViewportAndScissor();
    GXSetColorUpdate(GX_TRUE);
    GXSetAlphaUpdate(GX_TRUE);
    GXSetDstAlpha(GX_TRUE, 0);
    if (MR::isExistScreenAlphaTexture(1)) {
        GXSetDstAlpha(GX_FALSE, 0);
        CategoryList::drawXlu(static_cast< MR::DrawBufferType >(0x1));
        MR::captureScreenAlpha(1);
        MR::clearAlphaBuffer(0);
        MR::loadViewMtx();
        MR::loadProjectionMtx();
        GXSetColorUpdate(GX_TRUE);
        GXSetAlphaUpdate(GX_TRUE);
        GXSetDstAlpha(GX_TRUE, 0);
    }
    CategoryList::drawOpa(static_cast< MR::DrawBufferType >(0x0));
    CategoryList::execute(static_cast< MR::DrawType >(0x2e));
    CategoryList::execute(static_cast< MR::DrawType >(0x2f));
    bool active = false;
    if (MR::isExistSceneObj(0x34) && MR::getSceneObj< WorldMapResultCommand >(0x34)->isActive()) {
        active = true;
    }
    if (!active) {
        drawNormalObjects();
    }
    CategoryList::execute(static_cast< MR::DrawType >(0x52));
    CategoryList::execute(static_cast< MR::DrawType >(0x58));
    CategoryList::execute(static_cast< MR::DrawType >(0x34));
    CategoryList::execute(static_cast< MR::DrawType >(0x31));
    active = false;
    if (MR::isExistSceneObj(0x34) && MR::getSceneObj< WorldMapResultCommand >(0x34)->isActive()) {
        active = true;
    }
    if (active) {
        MR::getSceneObj< WorldMapResultCommand >(0x34)->drawDraw();
    }
    SceneFunction::executeDrawAfterIndirect();
    CategoryList::execute(static_cast< MR::DrawType >(0x38));
    MR::setStarPointerDrawSyncToken();
    MR::setTalkDirectorDrawSyncToken();
    SceneFunction::executeDrawImageEffect();
    GXSetDither(GX_TRUE);
    MR::loadViewMtx();
    MR::loadProjectionMtx();
    CategoryList::execute(static_cast< MR::DrawType >(0x59));
    MR::loadViewMtx();
    MR::loadProjectionMtx();
    MR::loadLightPlayer();
    CategoryList::execute(static_cast< MR::DrawType >(0x27));
    if (MR::isExistSceneObj(0x89)) {
        CategoryList::execute(static_cast< MR::DrawType >(0x33));
    }
    CategoryList::execute(static_cast< MR::DrawType >(0x40));
    if (MR::isStageWorldMap()) {
        if (MR::getSceneObj< WorldMapOwner >(0x33)->isGalaxyMap()) {
            MR::drawWorldMapInfo();
            WorldMapFunction::drawMini();
        }
    }
    MR::clearZBuffer();
}

void GameScene::requestEndMissDemo() {
    if (!clearPadMode() && !MR::isStageStaffRoll()) {
        GameSequenceFunction::requestChangeStageAfterMiss();
    }
}

void GameScene::requestStartSuperDreamerTalk() {
    setNerve(GET_NERVE(GameScene, GameSceneSuperDreamerClear));
}

void GameScene::requestEndSuperDreamerTalk() {
    setNerve(GET_NERVE(GameScene, GameSceneAction));
}

bool GameScene::isExecAction() const {
    return isNerve(GET_NERVE(GameScene, GameSceneAction));
}

bool GameScene::isExecSuperDreamerTalk() const {
    return isNerve(GET_NERVE(GameScene, GameSceneSuperDreamerClear));
}

bool GameScene::isExecPauseMenu() const {
    return isNerve(GET_NERVE(GameScene, GameScenePauseMenu));
}

bool GameScene::permitsStartPauseMenu() const {
    bool result = false;
    if (isNerve(GET_NERVE(GameScene, GameSceneAction)) && !MR::isDemoActive() && !MR::isWipeActive() && !MR::isWipeBlank()
        && !MR::isSystemWipeActive() && !MR::isSystemWipeBlank() && !MR::isPlayerDead() && !MR::isPlayerDamaging()
        && MR::isHiddenModelWipe() && !MR::isHomeButtonMenuActiveOrPreparing()) {
        result = true;
    }
    return result;
}

void GameScene::exeSaveAfterGameOver() {
    if (MR::isFirstStep(this)) {
        MR::incPlayerGameOverNum();
        GameSequenceFunction::startGameDataSaveSequenceAtGameOverEnd();
    }
    if (!GameSequenceFunction::isActiveSaveDataHandleSequence()) {
        MR::resetSystemAndGameStatus();
        GameSequenceFunction::startReloadUserFileSequence();
        GameDataFunction::resetPlayerLeftAndTryOnDimensionBigCastleGalaxyComet();
        GameSequenceFunction::requestChangeStageAfterFileSelect();
        if (GameDataFunction::calcCurrentPowerStarNum() > 0) {
            GameSequenceFunction::setFaceShipRespondGameOver();
        }
    }
}

void GameScene::exeAction() {
    SceneFunction::movementStopSceneController();
    SceneFunction::executeMovementList();
    if (MR::isStageWorldMap()) {
        MR::movementWorldMapBig();
        if (MR::getSceneObj< WorldMapOwner >(0x33)->isGalaxyMap()) {
            WorldMapFunction::movementMini();
        }
    }
    if (MR::isExistSceneObj(0x34)) {
        MR::getSceneObj< WorldMapResultCommand >(0x34)->movementDraw();
    }
}

void GameScene::calcViewAndEntry() {
    if (MR::isStageWorldMap()) {
        MR::entryWorldMapBig();
        if (MR::getSceneObj< WorldMapOwner >(0x33)->isGalaxyMap()) {
            WorldMapFunction::entryMini();
        }
    }
    bool active = false;
    if (MR::isExistSceneObj(0x34) && MR::getSceneObj< WorldMapResultCommand >(0x34)->isActive()) {
        active = true;
    }
    if (active) {
        MR::getSceneObj< WorldMapResultCommand >(0x34)->calcViewAndEntryDraw();
    } else {
        SceneFunction::executeCalcViewAndEntryList();
    }
}

void GameScene::startWorldMapBgm() const {
    if (!_29) {
        _29 = true;
        if (MR::isEqualStageName("WorldMap01Galaxy")) {
            if (GameDataFunction::isOnGameEventFlagEnding120Stars()) {
                MR::startStageBGM("MBGM_SMG2_WORLDMAP_05", false);
                return;
            }
            if (MR::isUseScenarioSelectMusicOneStar()) {
                MR::startDepartBGM();
                return;
            }
        }
        MR::startStageBGMFromStageName("Game", MR::getCurrentStageName(), MR::getCurrentScenarioNo(), false, false);
    }
}

void GameScene::drawNormalObjects() const {
    SceneFunction::executeDrawBufferListNormalOpaBeforeVolumeShadow();
    GXSetAlphaUpdate(GX_TRUE);
    CategoryList::execute(static_cast< MR::DrawType >(0x2b));
    GXSetColorUpdate(GX_TRUE);
    GXSetDstAlpha(GX_TRUE, 0);
    SceneFunction::executeDrawBufferListNormalOpaBeforeSilhouette();
    CategoryList::execute(static_cast< MR::DrawType >(0x2c));
    SceneFunction::executeDrawSilhouetteAndFillShadow();
    if (MR::isExistScreenAlphaTexture(1)) {
        MR::drawScreenAlphaShadow(1, false, false);
        MR::loadViewMtx();
        MR::loadProjectionMtx();
    }
    SceneFunction::executeDrawAlphaShadow();
    CategoryList::execute(static_cast< MR::DrawType >(0x1c));
    GXSetAlphaUpdate(GX_FALSE);
    GXSetDstAlpha(GX_FALSE, 0);
    SceneFunction::executeDrawBufferListNormalOpa();
    SceneFunction::executeDrawListOpa();
    CategoryList::drawOpa(static_cast< MR::DrawBufferType >(0x19));
    GXSetColorUpdate(GX_TRUE);
    GXSetDstAlpha(GX_TRUE, 0);
    CategoryList::execute(static_cast< MR::DrawType >(0x3e));
    MR::setLensFlareDrawSyncToken();
    GXSetAlphaUpdate(GX_FALSE);
    GXSetDstAlpha(GX_FALSE, 0);
    SceneFunction::executeDrawBufferListNormalXlu();
    SceneFunction::executeDrawListXlu();
    CategoryList::drawXlu(static_cast< MR::DrawBufferType >(0x19));
    if (MR::isStageWorldMap()) {
        MR::drawWorldMapBig();
        if (!MR::getSceneObj< WorldMapOwner >(0x33)->isGalaxyMap()) {
            MR::drawWorldMapMiniEffects();
        }
    }
    CategoryList::execute(static_cast< MR::DrawType >(0x2a));
}
