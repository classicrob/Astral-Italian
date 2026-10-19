#include "trace.hpp"
#include "replay.hpp"
#include "fp_mode.hpp"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <locale>
#include <sstream>
#include <iomanip>
namespace PetariNative::Headless {
const std::vector<std::string>& traceColumns() {
    static const std::vector<std::string> columns = {
#define TEXT(name) #name,
#define FLOAT(name) #name, #name "_bits",
#include "trace_fields.inc"
#undef TEXT
#undef FLOAT
    };
    return columns;
}
void State::text(const std::string& key, std::string value) {
    const auto& columns = traceColumns();
    if (std::find(columns.begin(), columns.end(), key) == columns.end()) throw ReplayError("unknown trace column: " + key);
    fields[key] = std::move(value);
}
void State::integer(const std::string& key, std::uint64_t value) { text(key, std::to_string(value)); }
void State::real(const std::string& key, float value) {
    std::uint32_t bits;
    std::memcpy(&bits, &value, sizeof(bits));
    if ((bits & 0x7f800000) == 0x7f800000) throw ReplayError("nonfinite trace field: " + key);
#if defined(__aarch64__)
    FloatingPointMode observeBits(false);
#endif
    char hex[16];
    std::ostringstream decimal;
    decimal.imbue(std::locale::classic());
    decimal << std::setprecision(9) << value;
    std::snprintf(hex, sizeof(hex), "0x%08x", bits);
    text(key, decimal.str());
    text(key + "_bits", hex);
}
namespace {
void cell(std::ostream& out, const std::string& value) {
    if (value.find_first_of(",\"\r\n") == std::string::npos) { out << value; return; }
    out << '"';
    for (char ch : value) { if (ch == '"') out << '"'; out << ch; }
    out << '"';
}
}
Trace::Trace(std::ostream& out) : mOut(out) {
    mOut.imbue(std::locale::classic());
    bool first = true;
    for (const auto& name : traceColumns()) { if (!first) mOut << ','; first = false; mOut << name; }
    mOut << '\n';
    if (!mOut) throw ReplayError("cannot write trace header");
}
void Trace::write(std::uint64_t frame, const State& state) {
    if (frame != mNext) throw ReplayError("trace frames must be contiguous and zero-based");
    for (const char* validity : {"scene_valid", "player_valid", "actor_count_valid", "rng_valid", "animation_valid"}) {
        const auto it=state.fields.find(validity);
        if (it==state.fields.end() || (it->second!="0" && it->second!="1")) throw ReplayError(std::string("missing/invalid validity flag: ")+validity);
    }
    const auto require=[&](const char* name) {
        if (!state.fields.count(name)) throw ReplayError(std::string("missing valid trace field: ")+name);
    };
    require("scene_epoch");
    for (const char* name : {"ticks", "vi_count"})
        if (const auto it=state.fields.find(name); it!=state.fields.end() && !it->second.empty())
            throw ReplayError("schema v1 clock columns are reserved; use the timing sidecar");
    if (state.fields.at("scene_valid")=="1") for (const char* name : {"scene", "stage", "scenario", "init_state"}) require(name);
    if (state.fields.at("rng_valid")=="1") require("rng_seed");
    if (state.fields.at("animation_valid")=="1") require("anim_name");
    if (state.fields.at("actor_count_valid")=="1") for (const char* name : {"actor_registered", "actor_living"}) require(name);
    if (state.fields.at("player_valid")=="1") {
#define TEXT(name)
#define FLOAT(name) require(#name); require(#name "_bits");
#include "trace_fields.inc"
#undef TEXT
#undef FLOAT
        for (const char* name : {"movement_lo", "movement_hi", "flags14_lo", "flags14_hi", "draw", "flags20", "flags24_lo", "flags24_hi", "prev_draw", "state_flags3", "state_ids", "mode", "hp", "dead"}) require(name);
    }
    bool first = true;
    for (const auto& name : traceColumns()) {
        if (!first) mOut << ',';
        first = false;
        if (name == "frame") mOut << frame;
        else if (const auto it = state.fields.find(name); it != state.fields.end()) cell(mOut, it->second);
    }
    mOut << '\n';
    mOut.flush();
    if (!mOut) throw ReplayError("cannot write trace row");
    ++mNext;
}
void Trace::finish() {
    mOut.flush();
    if (!mOut) throw ReplayError("cannot finish trace");
}
}
