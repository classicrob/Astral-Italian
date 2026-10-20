#include "actor_trace.hpp"
#include "trace.hpp"
#include "replay.hpp"
#include "fp_mode.hpp"

#include <charconv>
#include <cstring>
#include <iomanip>
#include <limits>
#include <locale>
#include <sstream>
#include <utility>

namespace PetariNative::Headless {
namespace {
#if defined(__aarch64__)
// Decimal formatting is observation work. Its sticky exception/status bits
// must not leak back into gameplay (FPCR itself is restored by scalar()).
class ObservationStatus {
public:
    ObservationStatus() { asm volatile("mrs %0, fpsr" : "=r"(mPrevious)); }
    ~ObservationStatus() { asm volatile("msr fpsr, %0" : : "r"(mPrevious)); }
private:
    std::uint64_t mPrevious;
};
#endif
const std::string& field(const State& state, const char* key) {
    static const std::string empty;
    const auto it = state.fields.find(key);
    return it == state.fields.end() ? empty : it->second;
}
std::uint64_t integer(const std::string& text) {
    std::uint64_t value = 0;
    const auto parsed = std::from_chars(text.data(), text.data() + text.size(), value);
    if (text.empty() || parsed.ec != std::errc{} || parsed.ptr != text.data() + text.size())
        throw ReplayError("invalid actor census integer");
    return value;
}
bool flag(const State& state, const char* key) {
    const auto& text = field(state, key);
    if (text != "0" && text != "1") throw ReplayError(std::string("invalid actor census flag: ") + key);
    return text == "1";
}
void row(std::ostream& out, const std::vector<std::string>& cells) {
    bool first = true;
    for (const auto& value : cells) {
        if (!first) out << ',';
        first = false;
        if (value.find_first_of(",\"\r\n") == std::string::npos) out << value;
        else {
            out << '"';
            for (char ch : value) { if (ch == '"') out << '"'; out << ch; }
            out << '"';
        }
    }
    out << '\n';
    if (!out) throw ReplayError("cannot write actor sidecar row");
}
std::string hexBytes(const std::string& bytes) {
    static constexpr char digits[] = "0123456789abcdef";
    std::string out;
    out.reserve(bytes.size() * 2);
    for (unsigned char byte : bytes) { out += digits[byte >> 4]; out += digits[byte & 15]; }
    return out;
}
void scalar(std::vector<std::string>& cells, float value) {
    static_assert(sizeof(float) == sizeof(std::uint32_t));
    std::uint32_t bits;
    std::memcpy(&bits, &value, sizeof(bits));
    if ((bits & 0x7f800000) == 0x7f800000) throw ReplayError("nonfinite actor census scalar");
#if defined(__aarch64__)
    FloatingPointMode observation(false); // Preserve stored subnormals during f32->decimal conversion.
#endif
    std::ostringstream decimal, hex;
    decimal.imbue(std::locale::classic());
    decimal << std::setprecision(9) << value;
    hex.imbue(std::locale::classic());
    hex << "0x" << std::hex << std::setw(8) << std::setfill('0') << bits;
    cells.push_back(decimal.str()); cells.push_back(hex.str());
}
void unused(const std::filesystem::path& path) {
    if (std::filesystem::exists(path) || std::filesystem::is_symlink(path))
        throw ReplayError("refusing to overwrite actor sidecar: " + path.string());
}
} // namespace

const std::vector<std::string>& actorTraceColumns() {
    static const std::vector<std::string> columns = {
        "schema_version", "frame", "scene_epoch", "scene", "stage", "scenario", "init_state",
        "producer_handle", "actor_id", "identity_kind", "identity_status", "name_valid", "name_hex",
        "link_valid", "link_id", "link_zone", "link_context", "dead",
        "x", "x_bits", "y", "y_bits", "z", "z_bits",
        "rot_x", "rot_x_bits", "rot_y", "rot_y_bits", "rot_z", "rot_z_bits",
        "scale_x", "scale_x_bits", "scale_y", "scale_y_bits", "scale_z", "scale_z_bits"};
    return columns;
}
const std::vector<std::string>& actorFrameColumns() {
    static const std::vector<std::string> columns = {
        "schema_version", "frame", "scene_epoch", "scene", "stage", "scenario", "init_state",
        "ready", "registered", "living", "emitted", "identity_unresolved", "identity_ambiguous"};
    return columns;
}
bool actorFrameReady(const State& state) {
    const bool scene = flag(state, "scene_valid"), group = flag(state, "actor_count_valid");
    if (scene) {
        for (const char* key : {"scene", "stage", "scenario", "init_state"})
            if (!state.fields.count(key)) throw ReplayError("missing actor census scene context");
        if (integer(field(state, "init_state")) > 6) throw ReplayError("invalid actor census initialization state");
    }
    return scene && field(state, "scene") == "Game" && field(state, "init_state") == "6" && group;
}
ActorTrace::ActorTrace(std::ostream& actors, std::ostream& frames) : mActors(actors), mFrames(frames) {
    mActors.imbue(std::locale::classic()); mFrames.imbue(std::locale::classic());
    row(mActors, actorTraceColumns()); row(mFrames, actorFrameColumns());
}
void ActorTrace::write(std::uint64_t frame, const State& state, const ActorFrame& sample) {
#if defined(__aarch64__)
    ObservationStatus status;
#endif
    if (mFinished || frame != mNextFrame) throw ReplayError("actor census frames must be contiguous and zero-based");
    const auto epoch = integer(field(state, "scene_epoch"));
    if (mEpoch && epoch < *mEpoch) throw ReplayError("actor census scene epoch moved backwards");
    if (sample.ready != actorFrameReady(state)) throw ReplayError("actor census readiness disagrees with main trace");
    if ((!sample.ready && (!sample.actors.empty() || sample.registered || sample.living)) ||
        sample.actors.size() > 32768 || sample.registered != sample.actors.size())
        throw ReplayError("incomplete actor census membership");
    std::uint32_t living = 0, players = 0;
    std::set<std::uintptr_t> pointers;
    std::vector<std::string> keys, kinds, names;
    std::vector<std::vector<std::string>> values;
    std::map<std::string, std::size_t> keyCount;
    // Validate and format the complete snapshot BEFORE mutating continuity or writing rows.
    for (const auto& actor : sample.actors) {
        if (!actor.localPointer || !pointers.insert(actor.localPointer).second)
            throw ReplayError("null or duplicate registered actor pointer");
        if (actor.nameBytes.size() >= ActorNameStorageLimit || actor.nameBytes.find('\0') != std::string::npos ||
            (!actor.nameValid && !actor.nameBytes.empty())) throw ReplayError("invalid actor name bytes");
        living += !actor.dead; players += actor.isPlayer;
        const auto name = hexBytes(actor.nameBytes);
        const bool linked = actor.linkId >= 0 && actor.linkZone >= 0 && actor.linkContext >= 0;
        const std::string kind = actor.isPlayer ? "player" : (linked && actor.nameValid ? "link" : "unresolved");
        const std::string key = actor.isPlayer ? "player" : kind == "link" ?
            "link:" + std::to_string(actor.linkZone) + ':' + std::to_string(actor.linkContext) + ':' +
            std::to_string(actor.linkId) + ':' + name : "";
        if (!key.empty()) ++keyCount[key];
        keys.push_back(key); kinds.push_back(kind); names.push_back(name);
        values.emplace_back();
        for (const auto& vector : {actor.position, actor.rotation, actor.scale})
            for (float value : vector) scalar(values.back(), value);
    }
    if (players > 1 || living != sample.living || (sample.ready &&
        (integer(field(state, "actor_registered")) != sample.registered || integer(field(state, "actor_living")) != living)))
        throw ReplayError("actor census counts disagree with main trace");
    if (!mEpoch || *mEpoch != epoch) { mActive.clear(); mKeyOwners.clear(); mEpoch = epoch; }
    std::vector<std::string> context = {"1", std::to_string(frame), std::to_string(epoch), field(state, "scene"),
        field(state, "stage"), field(state, "scenario"), field(state, "init_state")};
    std::map<std::uintptr_t, Continuity> active;
    // Historical ownership includes ambiguous keys: they cannot later become a silent stable identity.
    for (std::size_t i = 0; i < sample.actors.size(); ++i) {
        const auto pointer = sample.actors[i].localPointer;
        auto found = mActive.find(pointer);
        Continuity continuity;
        if (found == mActive.end()) {
            if (mNextHandle == UINT64_MAX) throw ReplayError("actor producer handle exhausted");
            continuity = {mNextHandle++, keys[i], false};
        } else {
            continuity = found->second;
            if (continuity.key != keys[i]) continuity.changed = true;
        }
        if (!keys[i].empty()) mKeyOwners[keys[i]].insert(continuity.handle);
        active.emplace(pointer, std::move(continuity));
    }
    std::uint32_t unresolved = 0, ambiguous = 0;
    for (std::size_t i = 0; i < sample.actors.size(); ++i) {
        const auto& actor = sample.actors[i]; const auto& continuity = active.at(actor.localPointer);
        const auto& key = keys[i];
        const std::string status = continuity.changed ? "key-change" :
            key.empty() ? "unresolved" : keyCount.at(key) > 1 ? "ambiguous" :
            mKeyOwners.at(key).size() > 1 ? "key-reuse" : "source-key";
        if (status == "ambiguous") ++ambiguous;
        else if (status != "source-key") ++unresolved;
        auto cells = context;
        cells.insert(cells.end(), {std::to_string(continuity.handle), status == "source-key" ? key : "", kinds[i], status,
            actor.nameValid ? "1" : "0", names[i],
            actor.linkId >= 0 && actor.linkZone >= 0 && actor.linkContext >= 0 ? "1" : "0",
            std::to_string(actor.linkId), std::to_string(actor.linkZone), std::to_string(actor.linkContext), actor.dead ? "1" : "0"});
        cells.insert(cells.end(), values[i].begin(), values[i].end());
        row(mActors, cells);
    }
    auto census = context;
    census.insert(census.end(), {sample.ready ? "1" : "0", sample.ready ? std::to_string(sample.registered) : "",
        sample.ready ? std::to_string(living) : "", std::to_string(sample.actors.size()), std::to_string(unresolved), std::to_string(ambiguous)});
    row(mFrames, census);
    mActors.flush(); mFrames.flush();
    if (!mActors || !mFrames) throw ReplayError("cannot flush actor sidecars");
    // An unobserved gap cannot establish sampled continuity. Keep key history
    // so a returning candidate gets a fresh handle and a key-reuse HOLD.
    mActive.swap(active);
    ++mNextFrame;
}
void ActorTrace::finish() {
    if (mFinished) throw ReplayError("actor trace already finished");
    mActors.flush(); mFrames.flush();
    if (!mActors || !mFrames) throw ReplayError("cannot finish actor sidecars");
    mFinished = true;
}
ActorTraceFiles::ActorTraceFiles(const std::filesystem::path& trace)
    : mActorPath(trace.string() + ".actors.csv"), mFramePath(trace.string() + ".actor-frames.csv") {
    for (const auto& path : {mActorPath, mFramePath, std::filesystem::path(mActorPath.string() + ".partial"),
                            std::filesystem::path(mFramePath.string() + ".partial")}) unused(path);
    mActors.open(mActorPath.string() + ".partial"); mFrames.open(mFramePath.string() + ".partial");
    if (!mActors || !mFrames) throw ReplayError("cannot create actor sidecars");
    mTrace = std::make_unique<ActorTrace>(mActors, mFrames);
}
void ActorTraceFiles::write(std::uint64_t frame, const State& state, const ActorFrame& actors) {
    if (mFinished) throw ReplayError("actor sidecars already finished");
    mTrace->write(frame, state, actors);
}
void ActorTraceFiles::finish() {
    if (mFinished) throw ReplayError("actor sidecars already finished");
    mTrace->finish(); mActors.close(); mFrames.close();
    if (!mActors || !mFrames) throw ReplayError("cannot close actor sidecars");
    unused(mActorPath); unused(mFramePath);
    std::filesystem::rename(mActorPath.string() + ".partial", mActorPath);
    try {
        std::filesystem::rename(mFramePath.string() + ".partial", mFramePath);
    } catch (...) {
        std::error_code error;
        std::filesystem::rename(mActorPath, mActorPath.string() + ".partial", error);
        if (error) throw ReplayError("actor sidecar publication failed and rollback failed: " + error.message());
        throw;
    }
    mFinished = true;
}
} // namespace PetariNative::Headless
