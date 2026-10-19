#include "trace.hpp"
#include "replay.hpp"
#include "Game/System/GameSystem.hpp"
#include "Game/System/GameSystemObjHolder.hpp"
#include "Game/System/GameSystemSceneController.hpp"
#include "Game/Util/SingletonHolder.hpp"
#include "Game/Util/PadRecorder.hpp"
#include "Game/Scene/Scene.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/LiveActor/AllLiveActorGroup.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/Player/MarioHolder.hpp"
#include "Game/Player/MarioActor.hpp"
#include "Game/Player/Mario.hpp"
#include "Game/Player/MarioAnimator.hpp"
#include "Game/Player/MarioPosHolder.hpp"
#include "Game/Player/MarioState.hpp"
#include <set>
#include <cstring>

namespace PetariNative::Headless {
namespace {
std::string bounded(const char* value, std::size_t limit) {
    if (!value) throw ReplayError("null observer string");
    const auto size = strnlen(value, limit);
    if (size == limit) throw ReplayError("unterminated observer string");
    for (std::size_t i=0;i<size;++i) if (static_cast<unsigned char>(value[i])>=128) throw ReplayError("non-ASCII observer string");
    return {value, size};
}
unsigned boolean(const bool& value) {
    static_assert(sizeof(bool)==1);
    unsigned char raw;
    std::memcpy(&raw,&value,1);
    if (raw>1) throw ReplayError("noncanonical observer boolean");
    return raw;
}
void vector(State& out, const char* x, const char* y, const char* z, const TVec3f& v) {
    out.real(x, v.x); out.real(y, v.y); out.real(z, v.z);
}
}
State observeGame() {
    State out;
    for (const char* key : {"scene_valid", "player_valid", "actor_count_valid", "rng_valid", "animation_valid"}) out.integer(key, 0);
    static const Scene* previousScene = nullptr;
    static std::string previousIdentity;
    static std::uint64_t epoch = 0;
    const auto* system = SingletonHolder<GameSystem>::get();
    const auto* controller = system ? system->mSceneController : nullptr;
    if (system && system->mObjHolder) {
        out.integer("rng_seed", system->mObjHolder->mRandom.mSeed);
        out.integer("rng_valid", 1);
        if (system->mObjHolder->mPadRecorder && MR::isPadRecorderReading())
            throw ReplayError("built-in PadRecorder playback would override strict KPAD replay");
    }
    if (!controller) { out.integer("scene_epoch", epoch); return out; }
    const auto& info = controller->mCurrSceneControlInfo;
    const auto sceneName = bounded(info.mScene, sizeof(info.mScene));
    const auto stage = bounded(info.mStage, sizeof(info.mStage));
    const auto identity = sceneName + '\n' + stage + '\n' + std::to_string(info.mScenarioNo);
    if (controller->mScene != previousScene || identity != previousIdentity) {
        ++epoch; previousScene = controller->mScene; previousIdentity = identity;
    }
    out.integer("scene_epoch", epoch);
    out.integer("scene_valid", 1);
    out.text("scene", sceneName); out.text("stage", stage);
    out.text("scenario", std::to_string(info.mScenarioNo));
    if (static_cast<unsigned>(controller->mSceneInitializeState)>6) throw ReplayError("invalid scene initialization enum");
    out.integer("init_state", static_cast<unsigned>(controller->mSceneInitializeState));
    if (controller->mSceneInitializeState != SceneInitializeState_End || sceneName != "Game") return out;
    if (!controller->mScene) throw ReplayError("initialized game controller has no scene");
    const auto* objects = controller->mScene->mSceneObjHolder;
    if (!objects) throw ReplayError("initialized game scene has no scene-object holder");
    const auto* group = static_cast<const AllLiveActorGroup*>(objects->mObjs[SceneObj_AllLiveActorGroup]);
    if (group) {
        if (group->mObjNum < 0 || group->mObjNum > group->mObjNumMax || group->mObjNumMax > 32768 ||
            (group->mObjNum && !group->mObjArray)) throw ReplayError("invalid live actor group");
        std::uint64_t living = 0;
        for (int i = 0; i < group->mObjNum; ++i) {
            const auto* actor = static_cast<const LiveActor*>(group->mObjArray[i]);
            if (!actor) throw ReplayError("null registered live actor");
            if (!boolean(actor->mFlags.mIsDead)) ++living;
        }
        out.integer("actor_registered", group->mObjNum);
        out.integer("actor_living", living);
        out.integer("actor_count_valid", 1);
    }
    const auto* holder = static_cast<const MarioHolder*>(objects->mObjs[SceneObj_MarioHolder]);
    const auto* actor = holder ? holder->mActor : nullptr;
    const auto* mario = actor ? actor->mMario : nullptr;
    if (!actor) return out;
    if (!mario || !mario->mPosHolder) throw ReplayError("initialized MarioActor has no Mario/position holder");
    vector(out, "x", "y", "z", actor->mPosition);
    vector(out, "sim_x", "sim_y", "sim_z", mario->mPosHolder->mPos);
    vector(out, "vx", "vy", "vz", mario->mVelocity);
    vector(out, "vel_after_x", "vel_after_y", "vel_after_z", mario->mVelocityAfter);
    vector(out, "actor_vx", "actor_vy", "actor_vz", actor->mVelocity);
    vector(out, "front_x", "front_y", "front_z", mario->mFrontVec);
    vector(out, "actor_up_x", "actor_up_y", "actor_up_z", actor->_270);
    vector(out, "gx", "gy", "gz", mario->mAirGravityVec);
    vector(out, "actor_gravity_x", "actor_gravity_y", "actor_gravity_z", actor->mGravity);
    out.integer("movement_lo", mario->mMovementStates.mLowWord); out.integer("movement_hi", mario->mMovementStates.mHighWord);
    out.integer("flags14_lo", mario->mFlags14.mLowWord); out.integer("flags14_hi", mario->mFlags14.mHighWord);
    out.integer("draw", mario->mDrawStates.mWord); out.integer("flags20", mario->mFlags20.mWord);
    out.integer("flags24_lo", mario->mFlags24.mLowWord); out.integer("flags24_hi", mario->mFlags24.mHighWord);
    out.integer("prev_draw", mario->mPrevDrawStates.mWord);
    out.text("state_flags3", std::to_string(boolean(mario->mStateFlags._0)) + ";" + std::to_string(boolean(mario->mStateFlags._1)) + ";" + std::to_string(boolean(mario->mStateFlags._2)));
    std::set<const MarioState*> visited;
    std::string states = "[";
    for (const MarioState* state = mario->_97C; state; state = state->_8) {
        if (!visited.insert(state).second || visited.size() > 64) throw ReplayError("cyclic or oversized Mario state stack");
        if (visited.size() > 1) states += ',';
        states += std::to_string(state->mStatusId);
    }
    out.text("state_ids", states + ']');
    if (mario->_980) out.integer("pending_state_id", mario->_980->mStatusId);
    out.integer("mode", actor->mPlayerMode); out.integer("hp", actor->mHealth); out.integer("dead", boolean(actor->mFlags.mIsDead));
    if (actor->mMarioAnim && actor->mMarioAnim->mCurrBck) {
        out.text("anim_name", bounded(actor->mMarioAnim->mCurrBck, 1024));
        out.integer("animation_valid", 1);
    }
    out.integer("player_valid", 1);
    return out;
}
}
