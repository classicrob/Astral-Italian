#include "stage_sync.hpp"
#include "replay.hpp"
#include <charconv>
#include <limits>
#include <utility>

namespace PetariNative::Headless {
namespace {
const std::string& field(const State& state, const char* key) {
    static const std::string empty;
    const auto found = state.fields.find(key);
    return found == state.fields.end() ? empty : found->second;
}
std::uint64_t decimal(const std::string& value) {
    std::uint64_t result = 0;
    const auto parsed = std::from_chars(value.data(), value.data() + value.size(), result);
    if (value.empty() || parsed.ec != std::errc{} || parsed.ptr != value.data() + value.size())
        throw ReplayError("invalid unsigned stage-sync field");
    return result;
}
}
std::uint32_t syncInteger(const std::string& value) {
    const bool hex = value.size() > 2 && value[0] == '0' && (value[1] == 'x' || value[1] == 'X');
    const auto start = value.data() + (hex ? 2 : 0);
    std::uint32_t result = 0;
    const auto parsed = std::from_chars(start, value.data() + value.size(), result, hex ? 16 : 10);
    if (start == value.data() + value.size() || parsed.ec != std::errc{} || parsed.ptr != value.data() + value.size())
        throw ReplayError("stage-sync number must be a decimal or 0x unsigned 32-bit integer");
    return result;
}
StageSync::StageSync(StageSyncOptions options) : mOptions(std::move(options)) {
    if (!enabled()) {
        if (mOptions.scenario || mOptions.seed) throw ReplayError("stage-sync scenario/seed requires --sync-stage");
        return;
    }
    if (!mOptions.scenario || mOptions.scenario > static_cast<std::uint32_t>(INT32_MAX))
        throw ReplayError("--sync-stage requires a positive placed --sync-scenario");
    if (mOptions.stage.size() >= 128) throw ReplayError("stage-sync internal name is too long");
    for (unsigned char ch : mOptions.stage)
        if (!((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') ||
              (ch >= '0' && ch <= '9') || ch == '_'))
            throw ReplayError("--sync-stage requires a canonical internal stage name");
}
bool StageSync::matches(const State& state) const {
    return field(state, "scene_valid") == "1" && field(state, "scene") == "Game" &&
           field(state, "init_state") == "6" && field(state, "stage") == mOptions.stage &&
           field(state, "scenario") == std::to_string(mOptions.scenario) &&
           (!mOptions.play || mPlayReady);
}
std::optional<std::uint64_t> StageSync::select(State& state, std::uint64_t sourceFrame,
                                              std::uint64_t ticks, std::uint64_t viCount,
                                              const ForceSeed& forceSeed) {
    if (!enabled()) return sourceFrame;
    if (!mOrigin) {
        if (!matches(state)) return {};
        if (field(state, "rng_valid") != "1") throw ReplayError("first stage-sync seam has no valid RNG seed");
        auto seed = decimal(field(state, "rng_seed"));
        if (seed > UINT32_MAX) throw ReplayError("stage-sync RNG seed exceeds u32");
        const auto seedBefore = static_cast<std::uint32_t>(seed);
        const auto epoch = decimal(field(state, "scene_epoch"));
        if (mOptions.seed) {
            if (!forceSeed) throw ReplayError("stage-sync RNG setter is unavailable");
            state = forceSeed(*mOptions.seed);
            if (!matches(state) || field(state, "rng_valid") != "1" ||
                decimal(field(state, "rng_seed")) != *mOptions.seed || decimal(field(state, "scene_epoch")) != epoch)
                throw ReplayError("stage-sync RNG write did not survive immediate observation");
            seed = *mOptions.seed;
        }
        mOrigin = Origin{sourceFrame, epoch, ticks, viCount, static_cast<std::uint32_t>(seed), seedBefore};
    }
    const auto epoch = decimal(field(state, "scene_epoch"));
    if (sourceFrame < mOrigin->frame || epoch < mOrigin->epoch) throw ReplayError("stage-sync source clock moved backwards");
    state.integer("scene_epoch", epoch - mOrigin->epoch);
    return sourceFrame - mOrigin->frame;
}
void StageSync::writeOrigin(std::ostream& out) const {
    if (!mOrigin) throw ReplayError("stage-sync target was never reached");
    out << "{\n  \"schema_version\": 1,\n  \"phase\": \"post-endFrame\",\n  \"stage\": \"" << mOptions.stage
        << "\",\n  \"scenario\": " << mOptions.scenario << ",\n  \"source_frame\": " << mOrigin->frame
        << ",\n  \"source_scene_epoch\": " << mOrigin->epoch << ",\n  \"source_ticks\": " << mOrigin->ticks
        << ",\n  \"source_vi_count\": " << mOrigin->viCount << ",\n  \"rng_seed\": " << mOrigin->seed
        << ",\n  \"rng_seed_before\": " << mOrigin->seedBefore
        << ",\n  \"rng_valid\": true,\n  \"rng_forced\": " << (mOptions.seed ? "true" : "false") << "\n}\n";
    out.flush();
    if (!out) throw ReplayError("cannot write stage-sync sidecar");
}
}
