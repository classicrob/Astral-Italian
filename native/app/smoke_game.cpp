// Observation for the smoke run (smoke.hpp): read-only queries of game and VI
// state. SDK side: no Aurora or SDL headers here.

#include "tape_scene.hpp"

#include "Game/System/GameDataFunction.hpp"
#include "Game/System/GameSequenceFunction.hpp"
#include "Game/System/GameSequenceDirector.hpp"
#include "Game/System/GameSystem.hpp"
#include "Game/System/GalaxyStatusAccessor.hpp"
#include "Game/System/GameSystemObjHolder.hpp"
#include "Game/System/GameSystemFunction.hpp"
#include "Game/System/GameSystemSceneController.hpp"
#include "Game/System/WPadHolder.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/Player/Mario.hpp"
#include "Game/Player/MarioActor.hpp"
#include "Game/Player/MarioHolder.hpp"
#include "Game/Scene/GameScene.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/Camera/CameraDirector.hpp"
#include "Game/Util/CameraUtil.hpp"
#include "Game/Util/DemoUtil.hpp"
#include "Game/Util/GamePadUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/SingletonHolder.hpp"

#include <petari/milestone.hpp>
#include <petari/platform/vi.hpp>

#include "smoke.hpp"
#include "smoke_observe_safety.hpp"
#include "actor_observe_store.hpp"
#include "ui_observe_store.hpp"

namespace PetariNative::App::Smoke {

namespace {
unsigned long gMilestonesRead = 0;
}

bool warpPlayer(const Step& step) {
    GameSystem* pGameSystem = SingletonHolder< GameSystem >::get();
    if (pGameSystem == nullptr || pGameSystem->mSceneController == nullptr ||
        !pGameSystem->mSceneController->isSceneInitializeState(SceneInitializeState_End) ||
        !MR::isExistSceneObj(SceneObj_MarioHolder)) {
        return false;
    }
    const MarioHolder* pHolder = MR::getSceneObj< MarioHolder >(SceneObj_MarioHolder);
    if (pHolder == nullptr || pHolder->getMarioActor() == nullptr || pHolder->getMarioActor()->getMario() == nullptr) {
        return false;
    }
    if (!step.warpName.empty()) {
        MR::setPlayerPos(step.warpName.c_str());
    } else {
        MR::setPlayerPosAndWait(TVec3f(step.warpX, step.warpY, step.warpZ), true);
    }
    return true;
}

Observation observeGame(bool wantPlayer, bool takeEvents) {
    Observation observation;

    const PetariNative::Platform::VI::DisplayState display = PetariNative::Platform::VI::displayState();
    observation.videoConfigured = display.configured;
    observation.videoBlack = display.black;

    const unsigned long count = takeEvents ? petari_milestone_count() : gMilestonesRead;
    for (; gMilestonesRead < count; ++gMilestonesRead) {
        const char* name = petari_milestone_at(gMilestonesRead);
        observation.milestones.push_back(name != nullptr ? name : "(milestone history overflowed)");
    }

    std::vector<UiObserve::Target> targets;
    std::vector<UiObserve::Prompt> prompts;
    if (takeEvents) UiObserve::take(&targets, &prompts);
    for (const UiObserve::Target& target : targets) {
        observation.targets.push_back({target.id, target.index, target.u, target.v, target.flags});
    }
    for (const UiObserve::Prompt& prompt : prompts) {
        observation.prompts.push_back({prompt.messageId, prompt.type});
    }
    // Taken every frame, so the fixed store never fills with stale frames.
    std::vector<ActorObserve::Actor> actors;
    if (takeEvents) ActorObserve::take(&actors);
    for (const ActorObserve::Actor& actor : actors) {
        observation.actors.push_back({actor.kind, actor.x, actor.y, actor.z, actor.dx, actor.dy, actor.dz, actor.state, actor.flags});
    }

    GameSystem* pGameSystem = SingletonHolder< GameSystem >::get();
    if (pGameSystem == nullptr || pGameSystem->mSceneController == nullptr) {
        return observation;
    }
    // Construction and deletion windows (audited):
    // - The scene is created and initialised on the async executor thread
    //   (GameSystemSceneController::initializeScene). A seam can fall between
    //   createScene and Scene::init, when the scene exists but its nerve does
    //   not. SceneInitializeState_End is set on this thread only after that
    //   initialisation ended. Ordinary requestChangeScene clears End before
    //   deletion; prepareReset instead raises mIsResetProcessing on the game
    //   thread. Both must gate observations before touching the scene slot.
    // - The scene/stage names are values in the controller: always readable.
    // - The save-data sequence is created with its nerve in
    //   GameSequenceDirector's constructor (GameSystem::init, before the first
    //   frame) and never deleted.
    const GameSystemSceneController* pController = pGameSystem->mSceneController;
    observation.scene = pController->mCurrSceneControlInfo.mScene;
    observation.stage = pController->mCurrSceneControlInfo.mStage;
    observation.scenario = pController->mCurrSceneControlInfo.mScenarioNo;
    observation.selectedScenario = pController->mCurrSceneControlInfo.mSelectedScenarioNo;
    observation.sceneReady = canObserveScene(pController,SceneInitializeState_End);
    observation.strap = observation.sceneReady && GameSystemFunction::isDisplayStrapRemineder();
    if (pGameSystem->mSequenceDirector != nullptr) {
        observation.saveSequence = GameSequenceFunction::isActiveSaveDataHandleSequence();
    }
    // Mario: only when asked (after Prologue.GameStart), in a ready Game scene,
    // through the scene's MarioHolder when it and its actor exist.
    if (wantPlayer && observation.sceneReady && observation.scene == "Game" && MR::isExistSceneObj(SceneObj_MarioHolder) &&
        MR::isExistSceneObj(SceneObj_CameraContext) && MR::isExistSceneObj(SceneObj_DemoDirector)) {
        const MarioHolder* pHolder = MR::getSceneObj< MarioHolder >(SceneObj_MarioHolder);
        const MarioActor* pMario = pHolder != nullptr ? pHolder->getMarioActor() : nullptr;
        if (canObservePlayer(pMario)) {
            observation.playerValid = true;
            observation.playerX = pMario->mPosition.x;
            observation.playerY = pMario->mPosition.y;
            observation.playerZ = pMario->mPosition.z;
            // The MR:: player queries go through the same MarioHolder actor.
            observation.playerOnGround = MR::isOnGroundPlayer();
            // The gravity FIELD at Mario (Mario::mAirGravityVec, from the gravity
            // system). MR::getPlayerGravity() is contact-relative instead: on
            // the ground it is the negated ground normal, so it tilts on slopes.
            // No Mario core: the gravity stays zero, which the story route
            // reports as "no gravity direction" rather than guessing.
            observation.gravityX = observation.gravityY = observation.gravityZ = 0.0f;
            if (const Mario* pCore = pMario->getMario()) {
                const TVec3f& gravity = pCore->getAirGravityVec();
                observation.gravityX = gravity.x;
                observation.gravityY = gravity.y;
                observation.gravityZ = gravity.z;
                observation.shadowX = pCore->mShadowPos.x;
                observation.shadowY = pCore->mShadowPos.y;
                observation.shadowZ = pCore->mShadowPos.z;
                const TVec3f* pMarioGravity = pCore->getGravityVec();
                observation.marioGravityX = pMarioGravity->x;
                observation.marioGravityY = pMarioGravity->y;
                observation.marioGravityZ = pMarioGravity->z;
                observation.marioStatus = static_cast<int>(pCore->getCurrentStatus());
            }
            observation.playerMode = static_cast<int>(pMario->mPlayerMode);
            observation.beeWallWalk = pMario->isBeeWallWalk();
            observation.playerOffControl = MR::isOffPlayerControl();
            observation.playerInRush = MR::isPlayerInRush();
            observation.rushActor = observedRushActorName(pMario, observation.playerInRush);
            observation.demoActive = MR::isDemoActive();
            observation.padA = MR::testCorePadButtonA(WPAD_CHAN0);
            observation.padB = MR::testCorePadButtonB(WPAD_CHAN0);
            observation.padPlus = MR::testCorePadButtonPlus(WPAD_CHAN0);
            observation.padMinus = MR::testCorePadButtonMinus(WPAD_CHAN0);
            observation.padOperating = MR::isOperatingWPad(WPAD_CHAN0);
            // CameraContext and TalkDirector are Game-scene SceneObjs: safe here.
            const TVec3f camX = MR::getCamXdir();
            const TVec3f camZ = MR::getCamZdir();
            observation.camXx = camX.x;
            observation.camXy = camX.y;
            observation.camXz = camX.z;
            observation.camZx = camZ.x;
            observation.camZy = camZ.y;
            observation.camZz = camZ.z;
            observation.padLeftTrigger = MR::testCorePadTriggerLeft(WPAD_CHAN0);
            observation.padRightTrigger = MR::testCorePadTriggerRight(WPAD_CHAN0);
            if (const CameraDirector* pDirector = MR::getCameraDirector()) {
                observation.camRoundLeft = pDirector->isEnableToRoundLeft();
                observation.camRoundRight = pDirector->isEnableToRoundRight();
            }
            observation.talkActive = MR::isSystemTalking();
            observation.playerDead = MR::isPlayerDead();
            observation.playerLife = static_cast< int >(pMario->getHealth());
            // SMG2 has no MR::isPlayerInBind (an SMG1 helper): not observed.
            observation.playerInBind = false;
            observation.playerSwinging = MR::isPlayerSwingAction();
            // The loaded file's game data (GameDataHolder), read-only; a file
            // is loaded whenever Mario is in a Game scene.
            observation.starEggStar1 = GameDataFunction::hasPowerStar("EggStarGalaxy", 1);
            observation.powerStars = GameDataFunction::calcCurrentPowerStarNum();
            observation.stageResult = GameSequenceFunction::hasStageResultSequence();
            // The Game scene is a GameScene (SceneFactory: "Game").
            // The Game scene is a GameScene (SceneFactory: "Game"); SMG2's check is permitsStartPauseMenu.
            if (pController->mScene != nullptr) {
                observation.pausePermitted = static_cast< const GameScene* >(pController->mScene)->permitsStartPauseMenu();
            }
        }
    }
    stepCollisionProbe(observation.stage, observation.sceneReady && observation.scene == "Game", observation.playerValid,
                       observation.playerX, observation.playerZ);
    return observation;
}

}  // namespace PetariNative::App::Smoke

#include "host.hpp"
#include "route_record.hpp"
#include "tape_record.hpp"
#include "tape_play.hpp"
#include <petari/player_launch.hpp>
#include "route_record_writer.hpp"
#include "playtest_capture.hpp"
#include "Game/Map/HitInfo.hpp"
#include "Game/Util/SceneUtil.hpp"
#include <petari/host_allocation.hpp>
#include <petari/input.hpp>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace PetariNative::App {
namespace Host {
void stallContext(char* stage, std::size_t size, int* scenario, int* zone) {
    stage[0] = '\0';
    *scenario = *zone = -1;
    const auto* system = SingletonHolder<GameSystem>::get();
    const auto* controller = system ? system->mSceneController : nullptr;
    if (controller == nullptr) return;
    std::snprintf(stage, size, "%s", controller->mCurrSceneControlInfo.mStage);
    *scenario = controller->mCurrSceneControlInfo.mScenarioNo;
    if (Smoke::canObserveScene(controller,SceneInitializeState_End) &&
        std::strcmp(controller->mCurrSceneControlInfo.mScene, "Game") == 0 && MR::isExistSceneObj(SceneObj_MarioHolder)) {
        const auto* holder = MR::getSceneObj<MarioHolder>(SceneObj_MarioHolder);
        const auto* actor = holder ? holder->getMarioActor() : nullptr;
        if (Smoke::canObservePlayer(actor)) {
            const auto* ground = actor->getMario()->mGroundPolygon;
            if (ground && ground->isValid()) *zone = ground->getHostPlacementZoneID();
        }
    }
}
}  // namespace Host

void recordRouteFrame(bool background) {
    static const char* path = TapeRecord::routePath();  // PETARI_TAPE_RECORD=<dir> or PETARI_ROUTE_RECORD=<csv>
    const bool record = path && *path;
    const bool capture = PlaytestCapture::enabled(background);
    const bool play = TapePlay::active();  // replaying a tape (--replay-tape): the seam feeds it the same row the recorder writes
    if (!record && !capture && !play) return;
    PetariNative::HostAllocationScope host;
    static RouteRecordWriter writer;
    const bool write = record && writer.open(path, background && !TapeRecord::background());  // a scripted tape session may record in the background
    if (!write && !capture && !play) return;
    RouteRecordFrame row;
    const auto* system = SingletonHolder<GameSystem>::get();
    const auto* controller = system ? system->mSceneController : nullptr;
    if (controller) {
        row.stage = controller->mCurrSceneControlInfo.mStage;
        row.scenario = controller->mCurrSceneControlInfo.mScenarioNo;
        if (Smoke::canObserveScene(controller,SceneInitializeState_End) &&
            std::strcmp(controller->mCurrSceneControlInfo.mScene, "Game") == 0 &&
            MR::isExistSceneObj(SceneObj_MarioHolder) && MR::isExistSceneObj(SceneObj_CameraContext)) {
            const auto* holder = MR::getSceneObj<MarioHolder>(SceneObj_MarioHolder);
            const auto* actor = holder ? holder->getMarioActor() : nullptr;
            if (Smoke::canObservePlayer(actor)) {
                row.valid = true;
                row.x = actor->mPosition.x; row.y = actor->mPosition.y; row.z = actor->mPosition.z;
                row.grounded = MR::isOnGroundPlayer();
                const auto& gravity = actor->getMario()->getAirGravityVec();
                row.gx = gravity.x; row.gy = gravity.y; row.gz = gravity.z;
                const auto camera = MR::getCamZdir();
                row.cx = camera.x; row.cy = camera.y; row.cz = camera.z;
                row.yaw = std::atan2(camera.x, camera.z);
                const auto* ground = actor->getMario()->mGroundPolygon;
                if (ground && ground->isValid()) row.zone = ground->getHostPlacementZoneID();
                row.bound = false;  // SMG2 has no MR::isPlayerInBind
                row.dead = MR::isPlayerDead();
                row.powerStars = GameDataFunction::calcCurrentPowerStarNum();
            }
        }
    }
    row.input = PetariNative::Input::boundState();
    TapeRecord::beginControlSeam(); // exact post-row snapshot, before the next update
    if (write) Smoke::inputScriptRecordSeam(row.stage ? row.stage : "",row.valid);
    if (play) {
        TapePlay::SeamState state;
        state.stage = row.stage ? row.stage : "";
        state.scenario = row.scenario;
        state.valid = row.valid;
        state.grounded = row.grounded;
        state.dead = row.dead;
        state.x = row.x; state.y = row.y; state.z = row.z;
        state.playReady = Host::playerInPlay();  // as of the previous seam's update, like the recorder and the headless driver
        state.initEnd = Smoke::canObserveScene(controller,SceneInitializeState_End);
        if (system && system->mObjHolder && system->mObjHolder->mWPadHolder) {
            const auto* pad = system->mObjHolder->mWPadHolder;
            const auto* sample = pad->mReadDataInfoArray ? pad->mReadDataInfoArray[0].getKPadStatus(0) : nullptr;
            if (sample) {
                state.pointerObserved = true;
                state.pointerX = sample->pos.x; state.pointerY = sample->pos.y;
                state.pointerValid = sample->dpd_valid_fg;
            }
        }
        if (system && system->mObjHolder) { state.rngObserved = true; state.rngSeed = system->mObjHolder->mRandom.mSeed; }
        auto step = TapePlay::seam(state);
        if (step.kind == TapePlay::Step::Kind::Start && step.restoreSeed && system && system->mObjHolder) {
            system->mObjHolder->mRandom.mSeed = step.seed;
            if (!TapePlay::acknowledgeSeed(system->mObjHolder->mRandom.mSeed)) step.kind = TapePlay::Step::Kind::GaveUp;
        }
        if (step.neutralTailBegin || (step.kind==TapePlay::Step::Kind::Finished &&
            (!TapePlay::meta() || !TapePlay::meta()->powerProtocol))) TapeRecord::neutralTailBegin();
        if (step.powerRequest) Host::requestQuit();
        if (step.kind == TapePlay::Step::Kind::Warp) {
            // A jump cut to the next segment's galaxy: the game's own stage change (what the Level Select warp calls), taken from
            // wherever the game is (the world map included). The name must outlive the move: PlayerLaunch::warpStage.
            std::snprintf(PetariNative::PlayerLaunch::warpStage, sizeof(PetariNative::PlayerLaunch::warpStage), "%s", step.stage.c_str());
            const GalaxyStatusAccessor status = MR::makeGalaxyStatusAccessor(PetariNative::PlayerLaunch::warpStage);
            if (status.mScenarioData != nullptr && !status.isNotNormalStar(step.scenario)) {
                GameSequenceFunction::requestChangeStageInStageScenario(PetariNative::PlayerLaunch::warpStage, step.scenario, step.scenario);
                std::fprintf(stderr, "PETARI TAPE PLAY: the game did not reach the next segment by itself: jump cut to %s scenario %d\n",
                             step.stage.c_str(), step.scenario);
            } else {
                std::fprintf(stderr, "PETARI TAPE PLAY: cannot jump to %s scenario %d (not a normal star of a known galaxy)\n", step.stage.c_str(), step.scenario);
            }
        }
        // PETARI_TAPE_PLAY_QUIT=<loops>: close the app that many loops after the replay finished or was given up (scripted runs);
        // otherwise the window stays open for the viewer, with neutral input.
        static long quitIn = -1;
        if ((step.kind == TapePlay::Step::Kind::Finished && !TapePlay::meta()->powerProtocol) ||
            (step.kind == TapePlay::Step::Kind::GaveUp && !step.powerRequest)) {
            const char* after = std::getenv("PETARI_TAPE_PLAY_QUIT");
            if (after && *after) quitIn = std::atol(after);
        }
        if (quitIn == 0) {
            if (!TapePlay::observationPending()) { quitIn = -1; Host::requestQuit(); }
        }
        else if (quitIn > 0) --quitIn;
        if (step.kind == TapePlay::Step::Kind::Start)
            std::fprintf(stderr, "PETARI TAPE PLAY: segment starts at %s scenario %d (seed %u)\n", row.stage, row.scenario, step.seed);
    }
    if (write) {
        writer.write(row);
        TapeRecord::Seam seam;
        if (controller) {
            seam.scene = controller->mCurrSceneControlInfo.mScene;
            seam.stage = controller->mCurrSceneControlInfo.mStage;
            seam.scenario = controller->mCurrSceneControlInfo.mScenarioNo;
            seam.initEnd = Smoke::canObserveScene(controller,SceneInitializeState_End);
        }
        if (system && system->mObjHolder) {
            seam.seedValid = true;
            seam.seed = system->mObjHolder->mRandom.mSeed;
        }
        seam.playReady = Host::playerInPlay();
        TapeRecord::seam(seam);
    }
    TapeRecord::endControlSeam();
    if (capture) PlaytestCapture::frame(row);
    TapeScene::nextFrame();
}
}
