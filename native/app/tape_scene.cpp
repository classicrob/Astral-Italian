#include "tape_scene.hpp"
#include "tape_record.hpp"
#include "tape_play.hpp"
#include "smoke_observe_safety.hpp"
#include "Game/System/GameSystem.hpp"
#include "Game/System/GameSystemObjHolder.hpp"
#include "Game/System/GameSystemSceneController.hpp"
#include "Game/System/WorldMapOwner.hpp"
#include "Game/System/WorldMapSequence.hpp"
#include "Game/System/WorldMapResultLocal.hpp"
#include "Game/Scene/ScenarioSelectScene.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/Player/MarioActor.hpp"
#include "Game/Player/MarioHolder.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/SingletonHolder.hpp"
#include <petari/kpad_record.hpp>
#include <petari/tape_input_observer.hpp>
#include <petari/host_allocation.hpp>
#include <cstdlib>
#include <cstring>

namespace PetariNative::App::TapeScene {
namespace {
std::uint64_t gFrame = 0, gLastRead = UINT64_MAX;
bool gInstalled = false, gCaptureReplay = false;
TapePhase::State observe() {
    TapePhase::State s; s.inputFrame = gFrame;
    const auto* system = SingletonHolder<GameSystem>::get();
    const auto* c = system ? system->mSceneController : nullptr;
    if (!c) return s;
    s.scene = c->mCurrSceneControlInfo.mScene;
    s.stage = c->mCurrSceneControlInfo.mStage;
    s.scenario = c->mCurrSceneControlInfo.mScenarioNo;
    const bool initialized = c->mScene && c->isSceneInitializeState(SceneInitializeState_End);
    const bool normal = initialized && c->getCurrentSceneForExecute() == c->mScene;
    if (c->mScenarioSelectScene && c->mScenarioSelectScene->mSpine && c->mScenarioSelectScene->isExecForeground())
        s.kind = "scenario-foreground";
    else if (normal) s.kind = "controller-normal";
    s.firstNormal = normal && c->isFirstUpdateSceneNerveNormal();
    if (system->mObjHolder) { s.rngObserved = true; s.rng = system->mObjHolder->mRandom.mSeed; }
    if (initialized && s.scene == "Game" && MR::isExistSceneObj(SceneObj_MarioHolder) &&
        MR::isExistSceneObj(SceneObj_CameraContext)) {
        const auto* holder = MR::getSceneObj<MarioHolder>(SceneObj_MarioHolder);
        const auto* actor = holder ? holder->getMarioActor() : nullptr;
        if (Smoke::canObservePlayer(actor)) {
            s.valid = true; s.grounded = MR::isOnGroundPlayer(); s.dead = MR::isPlayerDead();
            std::memcpy(&s.position[0], &actor->mPosition.x, sizeof(float));
            std::memcpy(&s.position[1], &actor->mPosition.y, sizeof(float));
            std::memcpy(&s.position[2], &actor->mPosition.z, sizeof(float));
        }
        if (normal && s.stage.compare(0,8,"WorldMap") == 0 && MR::isExistSceneObj(0x33)) {
            const auto* owner = MR::getSceneObj<WorldMapOwner>(0x33);
            if (owner && owner->mSequence) {
                s.galaxyMap = owner->mSequence->isGalaxyMap();
                s.mapWait = owner->mSequence->mStateWait && !owner->mSequence->mStateWait->mIsDead;
            }
        }
    }
    return s;
}
void before(std::int32_t channel, std::uint32_t, void*) {
    if (channel != 0 || gLastRead == gFrame) return;
    gLastRead = gFrame;
    PetariNative::HostAllocationScope scope;
    auto s = observe();
    const auto step = TapePlay::beforeRead(s);
    if (step.kind == TapePlay::Step::Kind::Start && step.restoreSeed) {
        auto* system = SingletonHolder<GameSystem>::get();
        if (system && system->mObjHolder) {
            system->mObjHolder->mRandom.mSeed = step.seed;
            TapePlay::acknowledgeSeed(system->mObjHolder->mRandom.mSeed);
            s.rng = system->mObjHolder->mRandom.mSeed;
        }
    }
    TapeRecord::beforeRead(s);
}
void after(std::int32_t channel, std::uint32_t capacity, std::int32_t returned, std::int32_t error,
           std::int32_t probe, std::uint32_t device, const void* buffer, void*) {
    // This is the ACTUAL decoded output of the replay hook. The ordinary SDK
    // recorder already records its own results, so never duplicate those reads.
    if (gCaptureReplay && TapePlay::active() && Input::KpadRecord::active())
        Input::KpadRecord::read(channel,capacity,returned,error,probe,device,static_cast<const KPADStatus*>(buffer));
}
}
void install() {
    if (gInstalled) return;
    const char* capture = std::getenv("PETARI_TAPE_CAPTURE_REPLAY");
    gCaptureReplay = capture && std::strcmp(capture,"1") == 0;
    const bool recording = TapeRecord::routePath() != nullptr;
    const bool phasedReplay = TapePlay::meta() && TapePlay::meta()->schema == 2;
    if (!recording && !phasedReplay) return;
    gInstalled = true;
    Input::TapeObserver::install({before,after,nullptr});
}
void nextFrame() { if (gInstalled) ++gFrame; }
}  // namespace PetariNative::App::TapeScene
