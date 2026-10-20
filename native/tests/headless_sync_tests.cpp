#include "stage_sync.hpp"
#include "replay.hpp"
#include <cstdio>
#include <cstdlib>
#include <sstream>
using namespace PetariNative::Headless;
namespace {
int checks = 0;
void check(bool value, const char* message) {
    ++checks;
    if (!value) { std::fprintf(stderr, "stage sync: %s\n", message); std::exit(1); }
}
template<class F> void rejects(F call, const char* message) {
    try { call(); } catch (const ReplayError&) { ++checks; return; }
    check(false, message);
}
State row() {
    State state;
    state.integer("scene_valid", 1); state.text("scene", "Game");
    state.text("stage", "TestGalaxy"); state.integer("scenario", 2); state.integer("init_state", 6);
    state.integer("scene_epoch", 7); state.integer("rng_valid", 1); state.integer("rng_seed", 99);
    state.integer("player_valid", 0);
    return state;
}
}
int main() {
    check(syncInteger("0") == 0 && syncInteger("0xffffffff") == UINT32_MAX && syncInteger("012") == 12, "u32 decimal/hex parsing including zero");
    for (const auto* value : {"", "-1", "+1", " 1", "1 ", "0x", "4294967296", "0x100000000"})
        rejects([&] { syncInteger(value); }, "invalid seed accepted");
    rejects([] { StageSync bad({"TestGalaxy", 0, {}}); }, "missing scenario");
    rejects([] { StageSync bad({"", 2, {}}); }, "orphan scenario");
    rejects([] { StageSync bad({"", 0, 0}); }, "orphan zero seed");
    rejects([] { StageSync bad({"Invalid\"Name", 1, {}}); }, "invalid canonical stage");
    StageSync off; auto state = row();
    check(off.select(state, 8, 90, 10) == 8 && state.fields.at("scene_epoch") == "7", "unsynced clock preserved");
    StageSync sync({"TestGalaxy", 2, 0}); int writes = 0;
    auto force = [&](std::uint32_t seed) { ++writes; auto result = row(); result.integer("rng_seed", seed); return result; };
    state.text("scene", "FileSelect");
    check(!sync.select(state, 2, 20, 3, force), "FileSelect background not selected");
    state = row(); state.integer("init_state", 5);
    check(!sync.select(state, 3, 30, 4, force), "initialization not finished");
    state = row(); state.integer("scenario", 1);
    check(!sync.select(state, 4, 40, 5, force), "selected rather than placed scenario rejected");
    state = row();
    check(sync.select(state, 5, 50, 6, force) == 0 && writes == 1, "first seam selected and seed forced once");
    check(state.fields.at("player_valid") == "0" && state.fields.at("rng_seed") == "0" && state.fields.at("scene_epoch") == "0", "no player delay and seed/epoch readback");
    std::ostringstream origin; sync.writeOrigin(origin);
    check(origin.str().find("\"source_frame\": 5") != std::string::npos && origin.str().find("\"source_scene_epoch\": 7") != std::string::npos && origin.str().find("\"rng_forced\": true") != std::string::npos, "origin provenance and forced status");
    state = row(); state.integer("scene_epoch", 8); state.integer("rng_seed", 100);
    check(sync.select(state, 6, 60, 7, force) == 1 && writes == 1 && state.fields.at("rng_seed") == "100" && state.fields.at("scene_epoch") == "1", "later state evolves without another reset");
    state = row(); state.integer("scene_epoch", 6);
    rejects([&] { sync.select(state, 7, 70, 8, force); }, "backwards epoch");
    StageSync missing({"TestGalaxy", 2, {}}); state = row(); state.integer("rng_valid", 0);
    rejects([&] { missing.select(state, 0, 0, 0); }, "missing first-seam RNG must fail");
    check(!missing.started(), "failed seam not committed");
    StageSync unforced({"TestGalaxy", 2, {}}); state = row();
    check(unforced.select(state, 9, 100, 11) == 0 && state.fields.at("rng_seed") == "99", "unforced mode observes existing seed");
    std::ostringstream plain; unforced.writeOrigin(plain);
    check(plain.str().find("\"rng_forced\": false") != std::string::npos, "unforced provenance");
    StageSync wrong({"TestGalaxy", 2, 123}); state = row();
    rejects([&] { wrong.select(state, 0, 0, 0, [](std::uint32_t) { return row(); }); }, "failed RNG readback");
    StageSync unreached({"OtherGalaxy", 2, {}});
    rejects([&] { std::ostringstream out; unreached.writeOrigin(out); }, "unreached target cannot complete");
    std::printf("stage sync: %d checks passed\n", checks);
}
