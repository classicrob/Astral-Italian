#pragma once
// Pure formatting: SDL is queried by input_events.cpp, never by this header.
#include <array>
#include <cstdint>
#include <iomanip>
#include <sstream>
#include <string>

namespace PetariNative::App::PadLog {

struct Pad {
    std::string name;
    std::uint16_t vendor = 0;
    std::uint16_t product = 0;
    int type = 0;
    enum class Connection { Unknown, Wired, Wireless } connection = Connection::Unknown;
    std::string path;
    std::string serial;
    // C, GR, GL, GR2, GL2 (SDL MISC2 and four paddles).
    std::array<bool, 5> extras{};
};

// Device strings can contain line breaks or quotes; keep each event on one line.
inline std::string quoted(const std::string& value) {
    std::string out = "'";
    for (unsigned char c : value.empty() ? std::string("?") : value) {
        if (c < 0x20 || c == 0x7f) out += '?';
        else {
            if (c == '\'' || c == '\\') out += '\\';
            out += static_cast<char>(c);
        }
    }
    return out + "'";
}

inline std::string identity(const Pad& pad) {
    std::ostringstream out;
    out << quoted(pad.name) << " vid=0x" << std::hex << std::setfill('0') << std::setw(4) << pad.vendor
        << " pid=0x" << std::setw(4) << pad.product;
    return out.str();
}

inline std::string usingPad(const Pad& pad, const std::string& layout, bool gyro, bool accel, bool rumble) {
    std::ostringstream out;
    out << "[pad] using " << identity(pad) << " type=" << pad.type << " connection=";
    switch (pad.connection) {
    case Pad::Connection::Wired: out << "wired/USB"; break;
    case Pad::Connection::Wireless: out << "wireless/Bluetooth-or-other"; break;
    default: out << "unknown"; break;
    }
    if (!pad.path.empty()) out << " path=" << quoted(pad.path);
    if (!pad.serial.empty()) out << " serial=" << quoted(pad.serial);
    out << " layout=" << layout << " extras=";
    constexpr const char* names[] = {"C", "GR", "GL", "GR2", "GL2"};
    bool first = true;
    for (std::size_t i = 0; i < pad.extras.size(); ++i) {
        if (!pad.extras[i]) continue;
        out << (first ? "" : ",") << names[i];
        first = false;
    }
    if (first) out << "none";
    out << "; gyro " << (gyro ? "on" : "no") << ", accelerometer " << (accel ? "on" : "no")
        << ", rumble " << (rumble ? "yes" : "no");
    return out.str();
}

inline std::string removed(const Pad& pad, const Pad* fallback) {
    return "[pad] removed " + identity(pad) + "; fallback=" + (fallback ? identity(*fallback) : "none");
}

inline std::string unmapped(const Pad& pad) {
    return "[pad] joystick present but unmapped " + identity(pad) +
           (pad.path.empty() ? "" : " path=" + quoted(pad.path));
}

inline std::string switch2Policy(bool enabled) {
    return std::string("[pad] SDL HIDAPI Nintendo Switch 2 driver hint=") + (enabled ? "enabled" : "disabled") +
           " (effective hint policy; backend availability depends on SDL build/device)";
}

}  // namespace PetariNative::App::PadLog
