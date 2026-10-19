#include "trace.hpp"
#include "replay.hpp"
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
    if (!std::isfinite(value)) throw ReplayError("nonfinite trace field: " + key);
    std::uint32_t bits;
    std::memcpy(&bits, &value, sizeof(bits));
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
