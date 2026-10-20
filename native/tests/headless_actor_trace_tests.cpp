#include "actor_trace.hpp"
#include "trace.hpp"
#include "replay.hpp"
#include "fp_mode.hpp"
#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <limits>
#include <sstream>

using namespace PetariNative::Headless;
namespace {
int checks = 0;
void check(bool value, const char* message) {
    ++checks;
    if (!value) { std::fprintf(stderr, "actor trace: %s\n", message); std::exit(1); }
}
template<class F> void rejects(F call, const char* message) {
    try { call(); } catch (const std::exception&) { ++checks; return; }
    check(false, message);
}
// Independent CSV parser for synthetic producer output, including quoted context.
using Table = std::vector<std::vector<std::string>>;
Table csv(const std::string& input) {
    Table out; std::vector<std::string> cells; std::string cell; bool quoted = false;
    for (std::size_t i = 0; i < input.size(); ++i) {
        const char ch = input[i];
        if (ch == '"') {
            if (quoted && i + 1 < input.size() && input[i + 1] == '"') { cell += '"'; ++i; }
            else quoted = !quoted;
        } else if (!quoted && (ch == ',' || ch == '\n')) {
            cells.push_back(cell); cell.clear();
            if (ch == '\n') { out.push_back(cells); cells.clear(); }
        } else cell += ch;
    }
    check(!quoted && cell.empty() && cells.empty(), "complete CSV rows");
    return out;
}
const std::string& at(const Table& table, std::size_t row, const std::string& column) {
    const auto found = std::find(table[0].begin(), table[0].end(), column);
    if (found == table[0].end()) throw std::runtime_error("missing test column");
    return table.at(row).at(found - table[0].begin());
}
ActorSample actor(std::uintptr_t pointer = 1, std::int16_t id = 3) {
    ActorSample value;
    value.localPointer = pointer; value.nameValid = true; value.nameBytes = "SyntheticActor";
    value.linkId = id; value.linkZone = 2; value.linkContext = 1;
    value.position = {1.0f, 2.0f, 3.0f}; value.scale = {1.0f, 1.0f, 1.0f};
    return value;
}
ActorFrame snapshot(std::vector<ActorSample> actors = {}) {
    ActorFrame frame; frame.ready = true; frame.actors = std::move(actors);
    frame.registered = frame.actors.size();
    for (const auto& item : frame.actors) frame.living += !item.dead;
    return frame;
}
State state(const ActorFrame& frame, unsigned epoch = 1) {
    State value;
    value.integer("scene_epoch", epoch); value.integer("scene_valid", 1);
    value.text("scene", "Game"); value.text("stage", "SyntheticGalaxy");
    value.integer("scenario", 2); value.integer("init_state", 6);
    value.integer("actor_count_valid", frame.ready);
    if (frame.ready) { value.integer("actor_registered", frame.registered); value.integer("actor_living", frame.living); }
    return value;
}
struct Capture {
    std::ostringstream actors, frames;
    ActorTrace trace{actors, frames};
    void write(unsigned index, const ActorFrame& frame, unsigned epoch = 1) { trace.write(index, state(frame, epoch), frame); }
};
struct Temp {
    std::filesystem::path path;
    Temp() {
        const auto base = std::filesystem::temp_directory_path();
        const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
        for (unsigned i = 0; i < 100; ++i) {
            path = base / ("petari-actor-synthetic-" + std::to_string(stamp) + "-" + std::to_string(i));
            if (std::filesystem::create_directory(path)) return;
        }
        throw std::runtime_error("cannot create synthetic test directory");
    }
    ~Temp() { std::error_code ignored; std::filesystem::remove_all(path, ignored); }
};
float raw(std::uint32_t bits) { float value; std::memcpy(&value, &bits, sizeof(value)); return value; }
void schemasAndCensus() {
    check(actorTraceColumns().size() == 36 && actorFrameColumns().size() == 13, "v1 column counts");
    check(traceColumns().size() == 86, "default main schema unchanged");
    Capture c; ActorFrame unavailable;
    c.write(0, unavailable); c.write(1, snapshot()); c.trace.finish();
    const auto frames = csv(c.frames.str()), actors = csv(c.actors.str());
    check(actors.size() == 1 && actors[0] == actorTraceColumns(), "no fabricated actor rows");
    check(frames.size() == 3 && frames[0] == actorFrameColumns(), "census every emitted main frame");
    check(at(frames, 1, "ready") == "0" && at(frames, 1, "registered").empty() && at(frames, 1, "living").empty(), "unready counts unknown");
    check(at(frames, 1, "emitted") == "0" && at(frames, 2, "ready") == "1" && at(frames, 2, "registered") == "0", "valid empty differs from unavailable");
    rejects([&] { c.write(2, snapshot()); }, "write after finish");
    rejects([&] { c.trace.finish(); }, "repeat finish");
    Capture gap; rejects([&] { gap.write(1, snapshot()); }, "nonzero first frame");
    gap.write(0, snapshot(), 2);
    rejects([&] { gap.write(1, snapshot(), 1); }, "backward epoch");
    auto context = state(snapshot()); context.text("scene_valid", "2");
    rejects([&] { actorFrameReady(context); }, "noncanonical flag");
    context = state(snapshot()); context.integer("init_state", 7);
    rejects([&] { actorFrameReady(context); }, "invalid initialization state");
    context = state(snapshot()); context.text("scene", "FileSelect");
    check(!actorFrameReady(context), "FileSelect background not ready");
}
void membershipAndValues() {
    auto p = actor(); p.isPlayer = true; p.linkId = p.linkZone = p.linkContext = -1;
    p.nameBytes = std::string("\x82\xa0\xff", 3); p.position = {raw(0x80000000), raw(1), raw(0x80000001)};
    auto dead = actor(2); dead.dead = true; dead.nameValid = false; dead.nameBytes.clear();
    auto empty = actor(3, 4); empty.nameBytes.clear();
    Capture c; auto frame = snapshot({p, dead, empty}); auto context = state(frame);
    context.text("stage", "Synthetic,\"Galaxy");
#if defined(__aarch64__)
    FloatingPointMode fz(true);
    const bool previous = FloatingPointMode::flushToZero();
    std::uint64_t fpcr, fpsr;
    asm volatile("mrs %0, fpcr" : "=r"(fpcr));
    asm volatile("mrs %0, fpsr" : "=r"(fpsr));
#endif
    c.trace.write(0, context, frame);
#if defined(__aarch64__)
    check(FloatingPointMode::flushToZero() == previous, "FPCR.FZ restored after subnormal formatting");
    std::uint64_t afterControl, afterStatus;
    asm volatile("mrs %0, fpcr" : "=r"(afterControl));
    asm volatile("mrs %0, fpsr" : "=r"(afterStatus));
    check(fpcr == afterControl && fpsr == afterStatus, "entire FPCR and FPSR restored after observation");
#endif
    const auto a = csv(c.actors.str()), f = csv(c.frames.str());
    check(a.size() == 4 && at(f, 1, "registered") == "3" && at(f, 1, "living") == "2", "all entries including dead retained");
    check(at(a, 1, "actor_id") == "player" && at(a, 1, "identity_kind") == "player" && at(a, 1, "link_valid") == "0", "canonical player independent of placement");
    check(at(a, 1, "name_hex") == "82a0ff" && at(a, 1, "stage") == "Synthetic,\"Galaxy", "lossless name bytes and CSV context escaping");
    check(at(a, 1, "x_bits") == "0x80000000" && at(a, 1, "x") == "-0", "signed zero retained");
    check(at(a, 1, "y_bits") == "0x00000001" && at(a, 1, "y") == "1.40129846e-45", "positive subnormal retained");
    check(at(a, 1, "z_bits") == "0x80000001" && at(a, 1, "z") == "-1.40129846e-45", "negative subnormal retained");
    check(at(a, 2, "name_valid") == "0" && at(a, 2, "identity_status") == "unresolved", "null name unresolved");
    check(at(a, 3, "name_valid") == "1" && at(a, 3, "actor_id") == "link:2:1:4:", "valid empty name distinct");
    check(at(a, 2, "link_valid") == "1" && at(a, 2, "dead") == "1", "raw link validity independent of name");
    std::vector<ActorSample> many;
    for (unsigned i = 0; i < 300; ++i) many.push_back(actor(i + 1, i));
    Capture large; large.write(0, snapshot(many));
    check(csv(large.actors.str()).size() == 301, "no 256-entry truncation");
}
void identities() {
    auto a = actor(), b = actor(2, 4);
    Capture c; c.write(0, snapshot({a, b})); c.write(1, snapshot({b, a}));
    auto rows = csv(c.actors.str());
    check(at(rows, 1, "producer_handle") == at(rows, 4, "producer_handle") && at(rows, 2, "producer_handle") == at(rows, 3, "producer_handle"), "registration order is not identity");
    a.linkId = 5; c.write(2, snapshot({a})); a.linkId = 3; c.write(3, snapshot({a}));
    rows = csv(c.actors.str());
    check(at(rows, 5, "identity_status") == "key-change" && at(rows, 6, "identity_status") == "key-change" && at(rows, 6, "actor_id").empty(), "key changes sticky even after restore");
    c.write(4, snapshot({a}), 2); rows = csv(c.actors.str());
    check(at(rows, 7, "identity_status") == "source-key" && at(rows, 7, "producer_handle") != at(rows, 1, "producer_handle"), "epoch resets history without recycling handles");
    Capture duplicate; b = actor(2); duplicate.write(0, snapshot({a, b})); duplicate.write(1, snapshot({a}));
    rows = csv(duplicate.actors.str());
    check(at(rows, 1, "identity_status") == "ambiguous" && at(rows, 2, "actor_id").empty(), "duplicate keys retained ambiguous");
    check(at(rows, 3, "identity_status") == "key-reuse", "ambiguous key never silently becomes stable");
    check(at(csv(duplicate.frames.str()), 1, "identity_ambiguous") == "2", "ambiguity count exact");
    for (bool unready : {false, true}) {
        Capture missing; missing.write(0, snapshot({a}));
        missing.write(1, unready ? ActorFrame{} : snapshot()); missing.write(2, snapshot({a}));
        rows = csv(missing.actors.str());
        check(at(rows, 1, "producer_handle") != at(rows, 2, "producer_handle"), "absence/unready gap allocates fresh sampled handle");
        check(at(rows, 2, "identity_status") == "key-reuse" && at(csv(missing.frames.str()), 3, "identity_unresolved") == "1", "gap retains key history and HOLD");
    }
}
void invalidSamples() {
    auto invalid = [](ActorFrame frame) {
        Capture c; rejects([&] { c.write(0, frame); }, "invalid complete snapshot accepted");
        check(csv(c.actors.str()).size() == 1 && csv(c.frames.str()).size() == 1, "invalid snapshot wrote no partial rows");
    };
    invalid(snapshot({actor(), actor()}));
    invalid(snapshot({actor(0)}));
    auto a = actor(); a.nameBytes.assign(ActorNameStorageLimit, 'a'); invalid(snapshot({a}));
    a = actor(); a.nameBytes = std::string("a\0b", 3); invalid(snapshot({a}));
    a = actor(); a.nameValid = false; invalid(snapshot({a}));
    a = actor(); a.position[0] = std::numeric_limits<float>::infinity(); invalid(snapshot({a}));
    a = actor(); a.rotation[2] = raw(0x7fc00001); invalid(snapshot({a}));
    a = actor(); a.isPlayer = true; auto b = a; b.localPointer = 2; invalid(snapshot({a, b}));
    auto frame = snapshot({actor()}); frame.living = 0; invalid(frame);
    frame = snapshot({actor()}); frame.registered = 0; invalid(frame);
    Capture mismatch; frame = snapshot({actor()}); auto context = state(frame); context.integer("actor_registered", 2);
    rejects([&] { mismatch.trace.write(0, context, frame); }, "main membership mismatch");
    context = state(frame); context.integer("actor_count_valid", 0);
    rejects([&] { mismatch.trace.write(0, context, frame); }, "main readiness mismatch");
    a = actor(); a.nameBytes.assign(ActorNameStorageLimit - 1, 'a'); Capture max; max.write(0, snapshot({a}));
    check(csv(max.actors.str()).size() == 2, "maximum bounded name accepted");
    std::ostringstream broken, good; broken.setstate(std::ios::badbit);
    rejects([&] { ActorTrace bad(broken, good); }, "header write failure");
    Capture fail; fail.actors.setstate(std::ios::badbit);
    rejects([&] { fail.write(0, snapshot({actor()})); }, "actor row failure");
    Capture finish; finish.frames.setstate(std::ios::badbit);
    rejects([&] { finish.trace.finish(); }, "finish flush failure");
}
void publication() {
    Temp temp; const auto base = temp.path / "synthetic.csv";
    const auto actors = std::filesystem::path(base.string() + ".actors.csv");
    const auto frames = std::filesystem::path(base.string() + ".actor-frames.csv");
    auto partial = [](const auto& path) { return std::filesystem::path(path.string() + ".partial"); };
    {
        ActorTraceFiles files(base); files.write(0, state(snapshot()), snapshot());
        check(!std::filesystem::exists(actors) && std::filesystem::exists(partial(actors)), "not published before finish");
        files.finish();
        check(std::filesystem::exists(actors) && std::filesystem::exists(frames) && !std::filesystem::exists(partial(frames)), "both finalized before caller completion");
        rejects([&] { files.finish(); }, "repeat file finish");
    }
    rejects([&] { ActorTraceFiles existing(base); }, "overwrite finalized files");
    const auto abandoned = temp.path / "abandoned";
    { ActorTraceFiles files(abandoned); }
    check(std::filesystem::exists(abandoned.string() + ".actors.csv.partial") && !std::filesystem::exists(abandoned.string() + ".actors.csv"), "destructor never publishes");
    rejects([&] { ActorTraceFiles existing(abandoned); }, "overwrite partial files");
    const auto linked = temp.path / "linked";
    std::filesystem::create_symlink(temp.path / "missing", linked.string() + ".actors.csv");
    rejects([&] { ActorTraceFiles symlink(linked); }, "dangling symlink destination");
    const auto rollback = temp.path / "rollback";
    const auto rollbackActor = std::filesystem::path(rollback.string() + ".actors.csv");
    const auto rollbackFrame = std::filesystem::path(rollback.string() + ".actor-frames.csv");
    ActorTraceFiles files(rollback);
    // Unix permits unlinking the open second partial: first rename succeeds,
    // second fails ENOENT, and the first must be rolled back to partial.
    std::filesystem::remove(partial(rollbackFrame));
    rejects([&] { files.finish(); }, "missing second partial must fail publication");
    check(!std::filesystem::exists(rollbackActor) && !std::filesystem::exists(rollbackFrame) && std::filesystem::exists(partial(rollbackActor)), "second rename failure rolls first back");
    check(!std::filesystem::exists(base.string() + ".complete"), "provider cannot create main completion");
}
}
int main() {
    schemasAndCensus(); membershipAndValues(); identities(); invalidSamples(); publication();
    std::printf("actor trace: %d checks passed\n", checks);
}
