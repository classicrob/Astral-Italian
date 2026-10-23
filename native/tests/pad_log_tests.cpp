#include "../app/pad_log.hpp"
#include <cstdio>
#include <string>

namespace Log = PetariNative::App::PadLog;
int main() {
    int checks = 0, failures = 0;
    auto check = [&](bool ok, const char* message) {
        ++checks;
        if (!ok) { ++failures; std::fprintf(stderr, "FAIL: %s\n", message); }
    };
    Log::Pad pad;
    pad.name = "SWITCH 2 Pro Controller";
    pad.vendor = 0x057e;
    pad.product = 0x2069;
    pad.type = 13;
    pad.connection = Log::Pad::Connection::Wired;
    pad.path = "IOService:/USB/controller";
    pad.serial = "abc123";
    pad.extras = {true, true, true, false, false};
    const auto line = Log::usingPad(pad, "ProGalaxy", true, true, true);
    check(line == "[pad] using 'SWITCH 2 Pro Controller' vid=0x057e pid=0x2069 type=13 connection=wired/USB"
                  " path='IOService:/USB/controller' serial='abc123' layout=ProGalaxy extras=C,GR,GL; gyro on, accelerometer on, rumble yes",
          "full Switch 2 add line");
    pad.connection = Log::Pad::Connection::Wireless;
    pad.extras = {false, true, true, true, true};
    auto wireless = Log::usingPad(pad, "ProOdyssey", false, false, false);
    check(wireless.find("connection=wireless/Bluetooth-or-other") != std::string::npos, "wireless state does not invent a transport");
    check(wireless.find("extras=GR,GL,GR2,GL2") != std::string::npos, "four paddle availability");
    check(wireless.find("layout=ProOdyssey") != std::string::npos, "chosen layout");
    check(wireless.find("gyro no, accelerometer no, rumble no") != std::string::npos, "sensor capabilities");
    Log::Pad fallback;
    fallback.name = "Other pad";
    fallback.vendor = 0x1234;
    fallback.product = 0xabcd;
    check(Log::removed(pad, &fallback) == "[pad] removed 'SWITCH 2 Pro Controller' vid=0x057e pid=0x2069; fallback='Other pad' vid=0x1234 pid=0xabcd",
          "removed identity is cached data, fallback identified");
    check(Log::removed(pad, nullptr).find("fallback=none") != std::string::npos, "last pad removed");
    check(Log::unmapped(pad) == "[pad] joystick present but unmapped 'SWITCH 2 Pro Controller' vid=0x057e pid=0x2069 path='IOService:/USB/controller'",
          "unmapped joystick diagnosis");
    const auto unknown = Log::usingPad({}, "Standard", false, false, false);
    check(unknown.find("'?' vid=0x0000 pid=0x0000 type=0 connection=unknown") != std::string::npos, "missing identity and unknown connection");
    check(unknown.find("path=") == std::string::npos && unknown.find("serial=") == std::string::npos, "absent optional fields omitted");
    check(unknown.find("extras=none") != std::string::npos, "no extras");
    check(Log::quoted("a\n'b\\c") == "'a?\\'b\\\\c'", "device strings stay quoted on one log line");
    check(Log::switch2Policy(true).find("hint=enabled") != std::string::npos, "enabled hint policy");
    check(Log::switch2Policy(false).find("hint=disabled") != std::string::npos, "disabled hint policy");
    std::printf("Pad log: %d checks, %d failed\n", checks, failures);
    return failures ? 1 : 0;
}
