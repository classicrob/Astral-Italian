#include "Game/System/GameSystem.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/NameObj/NameObjRegister.hpp"
#include "Game/Screen/HomeButtonLayout.hpp"
#include "Game/Screen/SystemWipeHolder.hpp"
#include "Game/System/AudSystemWrapper.hpp"
#include "Game/System/DrawSyncManager.hpp"
#include "Game/System/FileRipper.hpp"
#include "Game/System/GameSequenceDirector.hpp"
#include "Game/System/GameSequenceFunction.hpp"
#include "Game/System/GameSystemDataHolder.hpp"
#include "Game/System/GameSystemDimmingWatcher.hpp"
#include "Game/System/GameSystemErrorWatcher.hpp"
#include "Game/System/GameSystemException.hpp"
#include "Game/System/GameSystemFontHolder.hpp"
#include "Game/System/GameSystemFrameControl.hpp"
#include "Game/System/GameSystemFunction.hpp"
#include "Game/System/GameSystemObjHolder.hpp"
#include "Game/System/GameSystemResetAndPowerProcess.hpp"
#include "Game/System/GameSystemSceneController.hpp"
#include "Game/Scene/ModelWipeScene.hpp"
#include "Game/System/GameSystemStationedArchiveLoader.hpp"
#include "Game/System/HeapMemoryWatcher.hpp"
#include "Game/System/HomeButtonStateNotifier.hpp"
#include "Game/System/MainLoopFramework.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/MemoryUtil.hpp"
#include "Game/Util/MutexHolder.hpp"
#include "Game/Util/NerveUtil.hpp"
#include "Game/Util/SequenceUtil.hpp"
#include "Game/Util/SingletonHolder.hpp"
#include "Game/Util/SystemUtil.hpp"
#include <JSystem/JKernel/JKRAram.hpp>
#include <JSystem/JKernel/JKRExpHeap.hpp>
#include <nw4r/lyt/init.h>
#include <revolution.h>

#define GX_FIFO_SIZE 0x80000

#define INIT_AUDIO_KEY "オーディオ初期化"  // "Audio Initialization"

namespace NrvGameSystem {
    NEW_NERVE(GameSystemInitializeAudio, GameSystem, InitializeAudio);
    NEW_NERVE(GameSystemInitializeLogoScene, GameSystem, InitializeLogoScene);
    NEW_NERVE(GameSystemLoadStationedArchive, GameSystem, LoadStationedArchive);
    NEW_NERVE(GameSystemWaitForReboot, GameSystem, WaitForReboot);
    NEW_NERVE(GameSystemNormal, GameSystem, Normal);
};  // namespace NrvGameSystem

#ifdef GALAXY_NATIVE
// The native host entry point initializes the platform, then calls this game main.
extern "C" void petari_game_main(void) {
#else
void main(void) {
#endif
    OSInitFastCast();
    DVDInit();
    VIInit();
    HeapMemoryWatcher::createRootHeap();
    OSInitMutex(&MR::MutexHolder< 0 >::sMutex);
    OSInitMutex(&MR::MutexHolder< 1 >::sMutex);
    OSInitMutex(&MR::MutexHolder< 2 >::sMutex);
    nw4r::lyt::LytInit();
    MR::setLayoutDefaultAllocator();
    SingletonHolder< HeapMemoryWatcher >::init();
    SingletonHolder< HeapMemoryWatcher >::get()->setCurrentHeapToStationedHeap();
    FileRipper::setup(0x20000, MR::getStationedHeapNapa());
    GameSystemException::init();
    MR::initAcosTable();
    SingletonHolder< GameSystem >::init();
    SingletonHolder< GameSystem >::get()->init();

    GameSystem* pGameSystem = SingletonHolder< GameSystem >::get();

    while (true) {
        pGameSystem->frameLoop();
    }
}

GameSystem::GameSystem()
    : NerveExecutor("GameSystem"), mFifoBase(nullptr), mSequenceDirector(nullptr), mErrorWatcher(nullptr), mFontHolder(nullptr),
      mFrameControl(nullptr), mObjHolder(nullptr), mSceneController(nullptr), mStationedArchiveLoader(nullptr), mHomeButtonLayout(nullptr),
      mSystemWipeHolder(nullptr), mDataHolder(nullptr), mIsExecuteLoadSystemArchive(false) {
}

void GameSystem::init() {
    JKRAram::create(0xDB0000, 0xFFFFFFFF, 8, 7, 3);
    mObjHolder = new GameSystemObjHolder();
    mFontHolder = new GameSystemFontHolder();
    mFontHolder->createFontFromEmbeddedData();
    mDataHolder = new GameSystemDataHolder();
    initNerve(GET_NERVE(GameSystem, GameSystemInitializeAudio));
    mSequenceDirector = new GameSequenceDirector(mDataHolder);
    initGX();
    DrawSyncManager::start(0x300, 15);
    mSceneController = new GameSystemSceneController();
    mObjHolder->init();
    mErrorWatcher = new GameSystemErrorWatcher();
    mFrameControl = new GameSystemFrameControl();
    SingletonHolder< GameSystemResetAndPowerProcess >::init();
    SingletonHolder< GameSystemResetAndPowerProcess >::get()->initWithoutIter();
    mStationedArchiveLoader = new GameSystemStationedArchiveLoader();
    mHomeButtonLayout = new HomeButtonLayout();
    mDimmingWatcher = new GameSystemDimmingWatcher();
    setNerve(GET_NERVE(GameSystem, GameSystemInitializeAudio));
}

bool GameSystem::isExecuteLoadSystemArchive() const {
    return mIsExecuteLoadSystemArchive;
}

bool GameSystem::isDoneLoadSystemArchive() const {
    return isNerve(GET_NERVE(GameSystem, GameSystemNormal));
}

void GameSystem::startToLoadSystemArchive() {
    mIsExecuteLoadSystemArchive = true;
#ifdef GALAXY_NATIVE
    TRACE_BOOT("[boot] stationed archive loading: start\n");
    if (isTraceBoot()) {
        traceBootHeaps();
    }
#endif

    SingletonHolder< HeapMemoryWatcher >::get()->setCurrentHeapToStationedHeap();
    SingletonHolder< NameObjRegister >::get()->setCurrentHolder(mObjHolder->mObjHolder);
    setNerve(GET_NERVE(GameSystem, GameSystemLoadStationedArchive));
}

void GameSystem::exeInitializeAudio() {
    if (MR::isFirstStep(this)) {
        MainLoopFramework::sManager->_3F = false;
        MR::startFunctionAsyncExecute(MR::FunctorV0M< GameSystemObjHolder*, void (GameSystemObjHolder::*)() >(mObjHolder, &GameSystemObjHolder::createAudioSystem), 14, INIT_AUDIO_KEY);
    }

    updateSceneController();

#ifdef GALAXY_NATIVE
    u32 traceFrames, traceSeconds;
    if (isTraceBoot() && isTraceBootHeartbeat("InitializeAudio", &traceFrames, &traceSeconds)) {
        // The wave-data query reads the audio system, so only ask after creation ended.
        bool isCreated = MR::isEndFunctionAsyncExecute(INIT_AUDIO_KEY);
        OSReport("[boot] InitializeAudio: waiting %us (%u frames), audio system created %d, system wave data loaded %s\n",
                 traceSeconds, traceFrames, isCreated, isCreated ? (mObjHolder->mAudioSystem->isLoadDoneWaveDataAtSystemInit() ? "1" : "0") : "-");
    }
#endif

    if (MR::isEndFunctionAsyncExecute(INIT_AUDIO_KEY) && mObjHolder->mAudioSystem->isLoadDoneWaveDataAtSystemInit()) {
        mObjHolder->mAudioSystem->loadSystemWaveData();
        MR::waitForEndFunctionAsyncExecute(INIT_AUDIO_KEY);
        MainLoopFramework::sManager->_3F = true;
        setNerve(GET_NERVE(GameSystem, GameSystemInitializeLogoScene));
    }
}

void GameSystem::exeInitializeLogoScene() {
    if (GameSystemFunction::isResetProcessing()) {
        TRACE_BOOT("[boot] InitializeLogoScene: reset in progress, waiting for reboot\n");
        setNerve(GET_NERVE(GameSystem, GameSystemWaitForReboot));
    } else {
        if (MR::isFirstStep(this)) {
            GameSequenceFunction::requestChangeScene("Logo");
        }

        updateSceneController();

#ifdef GALAXY_NATIVE
        u32 traceFrames, traceSeconds;
        if (isTraceBoot() && isTraceBootHeartbeat("InitializeLogoScene", &traceFrames, &traceSeconds)) {
            OSReport("[boot] InitializeLogoScene: %us (%u frames), stationed loading requested %d\n", traceSeconds, traceFrames,
                     mIsExecuteLoadSystemArchive);
        }
#endif
    }
}

void GameSystem::exeLoadStationedArchive() {
    mStationedArchiveLoader->update();
    updateSceneController();

    if (mStationedArchiveLoader->isInitialized() || mStationedArchiveLoader->isDone()) {
        setNerve(GET_NERVE(GameSystem, GameSystemNormal));
    }
}

void GameSystem::exeWaitForReboot() {
}

void GameSystem::exeNormal() {
    updateSceneController();
    mStationedArchiveLoader->update();
}

void GameSystem::initGX() {
    if (mFifoBase == nullptr) {
        mFifoBase = new (32) u8[GX_FIFO_SIZE];
    }

    GXInit(mFifoBase, GX_FIFO_SIZE);
}

void GameSystem::initAfterStationedResourceLoaded() {
    TRACE_BOOT("[boot] initAfterStationedResourceLoaded: start\n");
    mFontHolder->createFontFromFile();
    mObjHolder->initAfterStationedResourceLoaded();
    mHomeButtonLayout->initWithoutIter();
    mErrorWatcher->initAfterResourceLoaded();
    mSystemWipeHolder = MR::createSystemWipeHolder();
    mSceneController->initAfterStationedResourceLoaded();
    mSequenceDirector->initAfterResourceLoaded();
    mDataHolder->init();
}

void GameSystem::prepareReset() {
    mStationedArchiveLoader->prepareReset();
}

inline bool isSystemWaitForReboot(const GameSystem* pGameSystem) {
    return pGameSystem->isNerve(GET_NERVE(GameSystem, GameSystemWaitForReboot));
}

inline bool isSystemNormal(const GameSystem* pGameSystem) {
    return pGameSystem->isNerve(GET_NERVE(GameSystem, GameSystemNormal));
}

bool GameSystem::isPreparedReset() const {
    bool isPrepared = false;
    if (isSystemWaitForReboot(this) || mStationedArchiveLoader->isPreparedReset()) {
        isPrepared = true;
    }
    return isPrepared;
}

void GameSystem::frameLoop() {
    MainLoopFramework::sManager->beginRender();
    draw();
    MainLoopFramework::sManager->endRender();
    update();
    calcAnim();
    mObjHolder->captureIfAllowForScreenPreserver();
#ifdef GALAXY_NATIVE
    petari_host_frame_mark(PETARI_FRAME_MARK_END_FRAME);  // frame-time statistics
#endif
    MainLoopFramework::sManager->endFrame();
#ifdef GALAXY_NATIVE
    // Host window, events and Aurora frame boundary (native/app).
    petari_host_frame_seam();
#endif
    MainLoopFramework::sManager->waitForRetrace();
#ifdef GALAXY_NATIVE
    petari_host_frame_mark(PETARI_FRAME_MARK_RETRACE_DONE);
    // Draw done no longer waits for the frame's token-time EFB captures; the
    // token callbacks (star pointer, lens flare, talk Z peeks) finish here,
    // before any of the next frame's game code reads what they wrote.
    petari_gx_sync_wait_tokens_delivered();
#endif
}

void GameSystem::draw() {
    mSceneController->drawScene();
    mSequenceDirector->draw();
    mObjHolder->drawStarPointer();
    mObjHolder->drawBeforeEndRender();

    if (mSystemWipeHolder != nullptr) {
        mSystemWipeHolder->draw();
    }

    GameSequenceFunction::drawPlayerMissLeft();

    if (mSceneController->mModelWipeScene != nullptr) {
        mSceneController->mModelWipeScene->draw();
    }

    mErrorWatcher->draw();
    mHomeButtonLayout->draw();
    SingletonHolder< GameSystemResetAndPowerProcess >::get()->draw();
}

void GameSystem::update() {
    SingletonHolder< GameSystemResetAndPowerProcess >::get()->movement();
    mSceneController->checkRequestAndChangeScene();
    mHomeButtonLayout->movement();

    if (!mHomeButtonLayout->isActive()) {
        mErrorWatcher->movement();
    }

    mObjHolder->update();

    mDimmingWatcher->_5 = mErrorWatcher->isWarning() || mHomeButtonLayout->isActive() || GameSequenceFunction::isActiveSaveDataHandleSequence();
    mDimmingWatcher->update();
    updateNerve();
}

void GameSystem::updateSceneController() {
    bool isSceneUpdate = true;
    bool isResetProcessing = SingletonHolder< GameSystemResetAndPowerProcess >::get()->isActive();

    if (isResetProcessing) {
        isSceneUpdate = false;
    }

    if (mHomeButtonLayout->isActive()) {
        isSceneUpdate = false;
    }

    if (GameSystemFunction::isOccurredSystemWarning()) {
        isSceneUpdate = false;
    }

    if (isSceneUpdate || isResetProcessing) {
        mSequenceDirector->update();
    }

    if (isSceneUpdate || mSceneController->isFirstUpdateSceneNerveNormal()) {
        if (mSystemWipeHolder != nullptr) {
            mSystemWipeHolder->movement();
        }

        mSceneController->updateScene();
    }

    if (isResetProcessing) {
        mSceneController->updateSceneDuringResetProcessing();
    }

    mObjHolder->updateAudioSystem();
}

void GameSystem::calcAnim() {
    mSceneController->calcAnimScene();

    if (mSystemWipeHolder != nullptr) {
        mSystemWipeHolder->calcAnim();
    }
}
