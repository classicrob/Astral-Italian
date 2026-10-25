#pragma once
// Exact tape recording, app side (docs: native/PLAYTEST.md "Recording a tape"): next to the route CSV
// (PETARI_ROUTE_RECORD=<x>.csv) the app writes <x>.kpd (every KPADReadEx result, native/input/kpad_record.hpp),
// <x>.actions (mod buttons) and <x>.tape.json (ready seams, stage/scenario, RNG seed, mods, exe sha256).
// PETARI_TAPE_RECORD=<dir> is the same with <dir>/route.csv, and it also works in background mode
// (scripted sessions). tools/port/replay_tape.py replays such a tape headless from a ready seam.
#include <cstdint>
#include <string>
#include "tape_phase.hpp"

namespace PetariNative::App::TapeRecord {

struct Seam {
    const char* scene = "";   // controller scene name ("Game", ...)
    const char* stage = "";
    int scenario = -1;
    bool initEnd = false;     // SceneInitializeState_End
    bool playReady = false;   // Host::playerInPlay() as of the previous seam's update (the headless --sync-play seam is the first true one)
    bool seedValid = false;
    std::uint32_t seed = 0;   // game RNG seed at this seam
};

// The route CSV path to use: PETARI_TAPE_RECORD's <dir>/route.csv, else PETARI_ROUTE_RECORD, else null.
const char* routePath();
bool background();  // PETARI_TAPE_RECORD: recording is allowed in background mode
// Once per post-endFrame seam, right after the route row of that seam was written.
void seam(const Seam& s);
// Normal app shutdown only: flush the final CSV/KPAD frames and write complete=true. Safe to call repeatedly;
// atexit remains a backup. Crash/forced-exit paths must leave the last incomplete checkpoint untouched.
void finish();
// Game-defined state, before channel 0's input; never guesses from coordinates.
void beforeRead(const TapePhase::State& state);
// Only a power request inside this post-route snapshot scope has unambiguous
// ordering before the next GameSystem::update. Out-of-scope/cross-thread requests
// are recorded as errors, never guessed from a row or a host timestamp.
void beginControlSeam();
void endControlSeam();
void neutralTailBegin();
void rejectSessionControl(const char* reason);
bool powerRequest();
void acknowledgePower(bool actualPowerFlag);
void setLaunch(const std::string& stage, int scenario);  // actual parsed launch, before game boot

// For tests: the sidecar text for a state.
struct Ready {
    std::uint64_t row;  // route row (= seam index) of the first Game/End seam of a scene visit
    std::string stage;
    int scenario;
    std::uint32_t seed;
};

}  // namespace PetariNative::App::TapeRecord
