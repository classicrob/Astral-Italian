// Tests for the app's physical-input count (native/app/input_events.cpp,
// Events::physicalInputs), which the smoke uses to tell an unattended run from
// an assisted one: which SDL events count as gameplay input, that pointer
// motion and focus are counted apart, that every event still reaches the input
// layer unchanged, and that the smoke's own presses (Events::pressButton) are
// never counted.
//
// Built with input_events.cpp and native/input/bindings.cpp only: the input
// layer's sinks and SDL_GetScancodeName are recording fakes below, so no
// window, SDL library or game is needed.

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_gamepad.h>
#include <SDL3/SDL_hints.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <map>
#include <unistd.h>
#include <vector>

#include "../app/host.hpp"
#include "../app/smoke_background.hpp"
#include "petari/input.hpp"

namespace Input = PetariNative::Input;
namespace Events = PetariNative::App::Events;
namespace Smoke = PetariNative::App::Smoke;

namespace {

int checks = 0;
void check(bool condition, const std::string& label) {
    ++checks;
    if (!condition) {
        std::fprintf(stderr, "FAIL: %s\n", label.c_str());
        std::exit(1);
    }
}

// What reached the input layer, in order.
struct Delivered {
    enum Kind { Key, MouseButton, MouseMoved, MouseLeft, Focus, PadButton, PadAxis, PadRemoved } kind;
    unsigned code;
    bool down;
    bool repeat;
};
std::vector<Delivered> gDelivered;
Input::Bindings gBindings;
struct FakePad {
    std::string name;
    Uint16 vendor, product;
    bool nintendo;
};
std::map<SDL_JoystickID, FakePad> gPads;
bool gNintendo = false;
int gPolicyQueries = 0;
int gPadLookups = 0;

}  // namespace

// Recording fakes of the input layer's sinks (native/input/wpad_host.cpp).
namespace PetariNative::Input {
void setBindings(const Bindings& value) {
    gBindings = value;
}
Bindings bindings() {
    return gBindings;
}
void keyEvent(KeyCode code, bool down, bool repeat) {
    gDelivered.push_back({Delivered::Key, code, down, repeat});
}
void mouseButtonEvent(MouseButton button, bool down) {
    gDelivered.push_back({Delivered::MouseButton, static_cast<unsigned>(button), down, false});
}
void mouseMoved(float, float) {
    gDelivered.push_back({Delivered::MouseMoved, 0, false, false});
}
void mouseWheel(float, float) {}  // the Odyssey camera mod's scroll (not used by these tests)
void menuWheel(float) {}  // the native menus' scroll (not used by these tests)
void pinch(float) {}
void mouseLeft() {
    gDelivered.push_back({Delivered::MouseLeft, 0, false, false});
}
void padButtonEvent(PadButton button, bool down) {
    gDelivered.push_back({Delivered::PadButton, static_cast<unsigned>(button), down, false});
}
void padAxisEvent(PadAxis axis, float value) {
    gDelivered.push_back({Delivered::PadAxis, static_cast<unsigned>(axis), value > 0.0f, false});
}
void padDisconnected() {
    gDelivered.push_back({Delivered::PadRemoved, 0, false, false});
}
void padConnected(bool nintendo) { gNintendo = nintendo; }
PadLayout padLayout() { return gNintendo ? PadLayout::ProGalaxy : PadLayout::Standard; }
void padSensorEvent(PadSensor, const float[3], double) {}
bool padRumble() { return false; }
int takeActionPresses(Action) { return 0; }
void setViewport(const Viewport&) {}
void focusChanged(bool focused) {
    gDelivered.push_back({Delivered::Focus, 0, focused, false});
}
}  // namespace PetariNative::Input

// Device lifecycle fakes: removal deliberately erases the SDL object before
// delivering the event, so the app must retain its identity itself.
extern "C" {
SDL_Gamepad* SDL_GetGamepadFromID(SDL_JoystickID id) {
    ++gPadLookups;
    const auto found = gPads.find(id);
    return found == gPads.end() ? nullptr : reinterpret_cast<SDL_Gamepad*>(&found->second);
}
const char* SDL_GetGamepadName(SDL_Gamepad* pad) { return reinterpret_cast<FakePad*>(pad)->name.c_str(); }
Uint16 SDL_GetGamepadVendor(SDL_Gamepad* pad) { return reinterpret_cast<FakePad*>(pad)->vendor; }
Uint16 SDL_GetGamepadProduct(SDL_Gamepad* pad) { return reinterpret_cast<FakePad*>(pad)->product; }
SDL_GamepadType SDL_GetGamepadType(SDL_Gamepad*) { return SDL_GAMEPAD_TYPE_NINTENDO_SWITCH_PRO; }
SDL_GamepadButtonLabel SDL_GetGamepadButtonLabel(SDL_Gamepad* pad, SDL_GamepadButton) {
    return reinterpret_cast<FakePad*>(pad)->nintendo ? SDL_GAMEPAD_BUTTON_LABEL_B : SDL_GAMEPAD_BUTTON_LABEL_A;
}
SDL_JoystickConnectionState SDL_GetGamepadConnectionState(SDL_Gamepad*) { return SDL_JOYSTICK_CONNECTION_WIRED; }
const char* SDL_GetGamepadPath(SDL_Gamepad*) { return "fake USB path"; }
const char* SDL_GetGamepadSerial(SDL_Gamepad*) { return "serial"; }
bool SDL_GamepadHasButton(SDL_Gamepad*, SDL_GamepadButton button) {
    return button == SDL_GAMEPAD_BUTTON_MISC2 || button == SDL_GAMEPAD_BUTTON_RIGHT_PADDLE1 || button == SDL_GAMEPAD_BUTTON_LEFT_PADDLE1;
}
bool SDL_GamepadHasSensor(SDL_Gamepad*, SDL_SensorType) { return true; }
bool SDL_SetGamepadSensorEnabled(SDL_Gamepad*, SDL_SensorType, bool) { return true; }
SDL_PropertiesID SDL_GetGamepadProperties(SDL_Gamepad*) { return 1; }
bool SDL_GetBooleanProperty(SDL_PropertiesID, const char*, bool) { return true; }
bool SDL_RumbleGamepad(SDL_Gamepad*, Uint16, Uint16, Uint32) { return true; }
bool SDL_GetHintBoolean(const char*, bool defaultValue) { ++gPolicyQueries; return defaultValue; }
SDL_JoystickID* SDL_GetGamepads(int* count) {
    *count = static_cast<int>(gPads.size());
    auto* ids = static_cast<SDL_JoystickID*>(std::malloc((gPads.size() + 1) * sizeof(SDL_JoystickID)));
    int i = 0;
    for (const auto& pad : gPads) ids[i++] = pad.first;
    ids[i] = 0;
    return ids;
}
void SDL_free(void* memory) { std::free(memory); }
bool SDL_IsGamepad(SDL_JoystickID id) { return gPads.count(id) != 0; }
const char* SDL_GetJoystickNameForID(SDL_JoystickID) { return "Unmapped Switch"; }
Uint16 SDL_GetJoystickVendorForID(SDL_JoystickID) { return 0x057e; }
Uint16 SDL_GetJoystickProductForID(SDL_JoystickID) { return 0x2069; }
const char* SDL_GetJoystickPathForID(SDL_JoystickID) { return "unmapped path"; }
}

extern "C" const char* SDL_GetScancodeName(SDL_Scancode scancode) {
    switch (scancode) {
    case SDL_SCANCODE_W:
        return "W";
    case SDL_SCANCODE_SPACE:
        return "Space";
    case SDL_SCANCODE_F9:
        return "F9";
    default:
        return "";
    }
}

namespace {

SDL_Event key(SDL_Scancode scancode, bool down, bool repeat = false) {
    SDL_Event event{};
    event.type = down ? SDL_EVENT_KEY_DOWN : SDL_EVENT_KEY_UP;
    event.key.scancode = scancode;
    event.key.down = down;
    event.key.repeat = repeat;
    return event;
}

SDL_Event mouseButton(Uint8 button, bool down) {
    SDL_Event event{};
    event.type = down ? SDL_EVENT_MOUSE_BUTTON_DOWN : SDL_EVENT_MOUSE_BUTTON_UP;
    event.button.button = button;
    event.button.down = down;
    return event;
}

SDL_Event padButton(Uint8 button, bool down) {
    SDL_Event event{};
    event.type = down ? SDL_EVENT_GAMEPAD_BUTTON_DOWN : SDL_EVENT_GAMEPAD_BUTTON_UP;
    event.gbutton.button = button;
    event.gbutton.down = down;
    return event;
}

SDL_Event padAxis(Uint8 axis, Sint16 value) {
    SDL_Event event{};
    event.type = SDL_EVENT_GAMEPAD_AXIS_MOTION;
    event.gaxis.axis = axis;
    event.gaxis.value = value;
    return event;
}

SDL_Event ofType(Uint32 type) {
    SDL_Event event{};
    event.type = type;
    return event;
}

std::string last() {
    const Smoke::PhysicalInputs now = Events::physicalInputs();
    return std::string(now.last, strnlen(now.last, sizeof(now.last)));
}

}  // namespace

int main() {
    // Known bindings: W is the stick, Space is A, the left mouse button is B.
    Input::Bindings bindings;
    bindings.bind(Input::Action::StickUp, Input::Binding::key(Input::Key::W));
    bindings.bind(Input::Action::A, Input::Binding::key(Input::Key::Space));
    bindings.bind(Input::Action::B, Input::Binding::mouse(Input::MouseButton::Left));
    Input::setBindings(bindings);

    namespace Background = PetariNative::App::SmokeBackground;
    check(!Background::requested(nullptr, false, nullptr), "normal launch stays interactive");
    check(Background::requested("stage", false, nullptr), "smoke defaults to background");
    check(Background::requested(nullptr, true, nullptr), "fixture defaults to background");
    check(!Background::requested("stage", true, "0"), "explicit opt-out stays interactive");
    Background::enabled = true;
    for (SDL_Event event : {key(SDL_SCANCODE_W, true), mouseButton(SDL_BUTTON_LEFT, true),
                           padButton(SDL_GAMEPAD_BUTTON_SOUTH, true), padAxis(SDL_GAMEPAD_AXIS_LEFTX, 32767),
                           ofType(SDL_EVENT_MOUSE_MOTION), ofType(SDL_EVENT_FINGER_DOWN)}) {
        check(!Background::eventFilter(nullptr, &event), "hardware event filtered before Aurora");
        Events::input(event);
    }
    Events::input(ofType(SDL_EVENT_WINDOW_FOCUS_LOST));
    check(gDelivered.empty(), "background hardware/focus events cannot change driver input");
    check(Events::physicalInputs().gameplay == 0 && Events::physicalInputs().pointer == 0 &&
          Events::physicalInputs().focus == 0, "isolated run still has zero assistance counters");
    Events::assertFocus();
    Events::pressButton(0, true);
    Events::pressButton(0, false);
    Events::movePointer(20, 30);
    check(gDelivered.size() == 4, "driver controls still delivered in background mode");
    SDL_Event quit = ofType(SDL_EVENT_QUIT);
    SDL_Event audio = ofType(SDL_EVENT_AUDIO_DEVICE_ADDED);
    check(Background::eventFilter(nullptr, &quit) && Background::eventFilter(nullptr, &audio),
          "quit and audio device lifecycle events remain observable");
    gDelivered.clear();
    Background::enabled = false;

    Smoke::PhysicalInputs now = Events::physicalInputs();
    check(now.gameplay == 0 && now.pointer == 0 && now.focus == 0 && last().empty(), "nothing counted at start");

    // A bound key: the press and the release count, the repeat does not.
    Events::input(key(SDL_SCANCODE_W, true));
    check(Events::physicalInputs().gameplay == 1 && last() == "key W (StickUp) down", "bound key press: " + last());
    Events::input(key(SDL_SCANCODE_W, true, true));
    check(Events::physicalInputs().gameplay == 1, "key repeat not counted");
    Events::input(key(SDL_SCANCODE_W, false));
    check(Events::physicalInputs().gameplay == 2 && last() == "key W (StickUp) up", "bound key release: " + last());

    // An unbound key is not gameplay input.
    Events::input(key(SDL_SCANCODE_F9, true));
    Events::input(key(SDL_SCANCODE_F9, false));
    check(Events::physicalInputs().gameplay == 2 && last() == "key W (StickUp) up", "unbound key not counted");

    // A bound mouse button counts both ways; an unbound one does not.
    Events::input(mouseButton(SDL_BUTTON_LEFT, true));
    check(Events::physicalInputs().gameplay == 3 && last() == "mouse Left (B) down", "bound mouse button: " + last());
    Events::input(mouseButton(SDL_BUTTON_LEFT, false));
    check(Events::physicalInputs().gameplay == 4 && last() == "mouse Left (B) up", "bound mouse release: " + last());
    Events::input(mouseButton(SDL_BUTTON_RIGHT, true));
    Events::input(mouseButton(SDL_BUTTON_RIGHT, false));
    check(Events::physicalInputs().gameplay == 4, "unbound mouse button not counted");

    // Pointer motion and focus changes have their own counts.
    Events::input(ofType(SDL_EVENT_MOUSE_MOTION));
    Events::input(ofType(SDL_EVENT_MOUSE_MOTION));
    Events::input(ofType(SDL_EVENT_WINDOW_FOCUS_LOST));
    Events::input(ofType(SDL_EVENT_WINDOW_FOCUS_GAINED));
    now = Events::physicalInputs();
    check(now.gameplay == 4 && now.pointer == 2 && now.focus == 2, "motion and focus counted apart from gameplay");

    // Every event above reached the input layer unchanged, counted or not.
    const std::vector<Delivered> expected = {
        {Delivered::Key, Input::Key::W, true, false},
        {Delivered::Key, Input::Key::W, true, true},
        {Delivered::Key, Input::Key::W, false, false},
        {Delivered::Key, Input::Key::F9, true, false},
        {Delivered::Key, Input::Key::F9, false, false},
        {Delivered::MouseMoved, 0, false, false},
        {Delivered::MouseButton, static_cast<unsigned>(Input::MouseButton::Left), true, false},
        {Delivered::MouseMoved, 0, false, false},
        {Delivered::MouseButton, static_cast<unsigned>(Input::MouseButton::Left), false, false},
        {Delivered::MouseMoved, 0, false, false},
        {Delivered::MouseButton, static_cast<unsigned>(Input::MouseButton::Right), true, false},
        {Delivered::MouseMoved, 0, false, false},
        {Delivered::MouseButton, static_cast<unsigned>(Input::MouseButton::Right), false, false},
        {Delivered::MouseMoved, 0, false, false},
        {Delivered::MouseMoved, 0, false, false},
        {Delivered::Focus, 0, false, false},
        {Delivered::Focus, 0, true, false},
    };
    bool same = gDelivered.size() == expected.size();
    for (size_t i = 0; same && i < expected.size(); i++) {
        same = gDelivered[i].kind == expected[i].kind && gDelivered[i].code == expected[i].code &&
               gDelivered[i].down == expected[i].down && gDelivered[i].repeat == expected[i].repeat;
    }
    check(same, "every event delivered unchanged (" + std::to_string(gDelivered.size()) + " calls)");

    // The smoke's own presses go to the input layer directly and never count.
    gDelivered.clear();
    const Smoke::PhysicalInputs before = Events::physicalInputs();
    Events::assertFocus();
    Events::pressButton(0, true);   // A
    Events::pressButton(0, false);
    Events::pressButton(2, true);   // stick up
    Events::pressButton(2, false);
    Events::movePointer(10.0f, 20.0f);
    now = Events::physicalInputs();
    check(gDelivered.size() == 6 && gDelivered[1].kind == Delivered::Key && gDelivered[1].code == Input::Key::Space &&
              gDelivered[3].code == Input::Key::W,
          "smoke presses reach the input layer through the bindings");
    check(now.gameplay == before.gameplay && now.pointer == before.pointer && now.focus == before.focus &&
              std::strcmp(now.last, before.last) == 0,
          "smoke presses, pointer and focus are not physical input");

    // Game controllers: a bound button counts both ways, an unbound one does
    // not; a stick counts only past half travel (drift is not a player), and
    // the right stick is pointer motion.
    bindings.bind(Input::Action::A, Input::Binding::pad(Input::PadButton::South));
    Input::setBindings(bindings);
    gDelivered.clear();
    const Smoke::PhysicalInputs padBefore = Events::physicalInputs();
    Events::input(padButton(SDL_GAMEPAD_BUTTON_SOUTH, true));
    check(Events::physicalInputs().gameplay == padBefore.gameplay + 1 && last() == "pad Pad bottom (A) down",
          "bound pad button: " + last());
    Events::input(padButton(SDL_GAMEPAD_BUTTON_SOUTH, false));
    Events::input(padButton(SDL_GAMEPAD_BUTTON_GUIDE, true));
    Events::input(padButton(SDL_GAMEPAD_BUTTON_GUIDE, false));
    check(Events::physicalInputs().gameplay == padBefore.gameplay + 2, "unbound pad button not counted");
    Events::input(padAxis(SDL_GAMEPAD_AXIS_LEFTX, 3000));
    check(Events::physicalInputs().gameplay == padBefore.gameplay + 2, "stick drift not counted");
    Events::input(padAxis(SDL_GAMEPAD_AXIS_LEFTX, 30000));
    check(Events::physicalInputs().gameplay == padBefore.gameplay + 3, "a pushed stick counts");
    Events::input(padAxis(SDL_GAMEPAD_AXIS_RIGHTY, -30000));
    check(Events::physicalInputs().pointer == padBefore.pointer + 1 &&
              Events::physicalInputs().gameplay == padBefore.gameplay + 3,
          "the right stick is pointer motion");
    Events::input(ofType(SDL_EVENT_GAMEPAD_REMOVED));
    check(gDelivered.size() == 8 && gDelivered[0].kind == Delivered::PadButton &&
              gDelivered[0].code == static_cast<unsigned>(Input::PadButton::South) && gDelivered[0].down &&
              gDelivered[5].kind == Delivered::PadAxis && gDelivered[5].down &&
              gDelivered[6].code == static_cast<unsigned>(Input::PadAxis::RightY) && !gDelivered[6].down &&
              gDelivered[7].kind == Delivered::PadRemoved,
          "pad events delivered unchanged (" + std::to_string(gDelivered.size()) + " calls)");

    // PETARI_SMOKE_IGNORE_PHYSICAL: a foreground automated run drops physical input but keeps focus events.
    check(!Background::ignorePhysicalRequested(nullptr, false, "1"), "normal launch never ignores physical input");
    check(Background::ignorePhysicalRequested("stage", false, "1") && Background::ignorePhysicalRequested(nullptr, true, "1"),
          "smoke or fixture with the flag ignores physical input");
    check(!Background::ignorePhysicalRequested("stage", true, "0") && !Background::ignorePhysicalRequested("stage", true, nullptr),
          "flag off keeps physical input");
    SDL_Event press = key(SDL_SCANCODE_W, true);
    check(Background::eventFilter(nullptr, &press), "flag off: a physical key passes the SDL filter");
    Background::ignorePhysical = true;
    gDelivered.clear();
    const Smoke::PhysicalInputs ignoredBefore = Events::physicalInputs();
    check(!Background::eventFilter(nullptr, &press), "flag on: a physical key is dropped by the SDL filter");
    Events::input(press);
    Events::input(padButton(SDL_GAMEPAD_BUTTON_SOUTH, true));
    check(gDelivered.empty() && Events::physicalInputs().gameplay == ignoredBefore.gameplay,
          "flag on: physical key/pad input neither delivered nor counted");
    SDL_Event focusLost = ofType(SDL_EVENT_WINDOW_FOCUS_LOST);
    check(Background::eventFilter(nullptr, &focusLost), "flag on: focus events pass the SDL filter");
    Events::input(focusLost);
    check(gDelivered.size() == 1 && gDelivered[0].kind == Delivered::Focus &&
              Events::physicalInputs().focus == ignoredBefore.focus + 1,
          "flag on: focus events still reach the input layer and are counted");
    Background::ignorePhysical = false;
    gDelivered.clear();
    Events::input(press);
    check(gDelivered.size() == 1 && gDelivered[0].kind == Delivered::Key &&
              Events::physicalInputs().gameplay == ignoredBefore.gameplay + 1,
          "flag off: the same key is delivered and counted");

    // Capture only lifecycle output; all queries above still use real app code.
    std::FILE* log = std::tmpfile();
    check(log != nullptr, "temporary lifecycle log");
    std::fflush(stderr);
    const int savedStderr = dup(fileno(stderr));
    check(savedStderr >= 0 && dup2(fileno(log), fileno(stderr)) >= 0, "capture lifecycle stderr");
    auto lifecycle = [](Uint32 type, SDL_JoystickID id) {
        SDL_Event event{};
        event.type = type;
        event.gdevice.which = id;
        Events::input(event);
    };
    gPads.emplace(1, FakePad{"First pad", 0x1111, 0x2222, false});
    lifecycle(SDL_EVENT_GAMEPAD_ADDED, 1);
    check(!gNintendo, "Standard pad selects positional layout");
    gPads.emplace(2, FakePad{"Switch 2", 0x057e, 0x2069, true});
    lifecycle(SDL_EVENT_GAMEPAD_ADDED, 2);
    check(gNintendo, "Switch 2 labels choose Pro layout");
    gPads.erase(1);  // Remove a non-selected pad: still log it, keep Switch 2.
    const int lookups = gPadLookups;
    lifecycle(SDL_EVENT_GAMEPAD_REMOVED, 1);
    check(gNintendo && gPadLookups == lookups, "non-selected removal uses cached identity without querying a vanished pad");
    gPads.emplace(3, FakePad{"Fallback pad", 0x3333, 0x4444, false});
    lifecycle(SDL_EVENT_GAMEPAD_ADDED, 3);
    gPads.erase(3);
    lifecycle(SDL_EVENT_GAMEPAD_REMOVED, 3);
    check(gNintendo, "selected removal falls back to remaining Nintendo pad");
    gPads.erase(2);
    lifecycle(SDL_EVENT_GAMEPAD_REMOVED, 2);
    check(!gNintendo, "last removal selects Standard");
    lifecycle(SDL_EVENT_JOYSTICK_ADDED, 4);
    lifecycle(SDL_EVENT_JOYSTICK_REMOVED, 4);
    Events::frame();
    check(gPolicyQueries == 2, "Switch 2/HIDAPI hint policy queried once per app, across events and frames");
    std::fflush(stderr);
    check(dup2(savedStderr, fileno(stderr)) >= 0, "restore stderr");
    close(savedStderr);
    std::rewind(log);
    char logBuffer[8192]{};
    const size_t size = std::fread(logBuffer, 1, sizeof(logBuffer) - 1, log);
    check(size < sizeof(logBuffer) - 1, "bounded lifecycle log");
    std::fclose(log);
    const std::string lines(logBuffer, size);
    check(lines.find("[pad] using 'Switch 2' vid=0x057e pid=0x2069") != std::string::npos &&
          lines.find("layout=ProGalaxy extras=C,GR,GL") != std::string::npos, "add logs identity, layout and extra buttons");
    check(lines.find("[pad] removed 'First pad' vid=0x1111 pid=0x2222; fallback='Switch 2'") != std::string::npos,
          "non-selected removal is logged with continuing pad");
    check(lines.find("[pad] removed 'Fallback pad' vid=0x3333 pid=0x4444; fallback='Switch 2'") != std::string::npos,
          "selected removal logs cached identity and fallback");
    check(lines.find("[pad] removed 'Switch 2' vid=0x057e pid=0x2069; fallback=none") != std::string::npos,
          "last removal logs no fallback");
    check(lines.find("joystick present but unmapped 'Unmapped Switch' vid=0x057e pid=0x2069") != std::string::npos,
          "unmapped joystick add is visible");
    check(lines.find("[pad] removed 'Unmapped Switch' vid=0x057e pid=0x2069; fallback=none") != std::string::npos,
          "unmapped joystick removal retains identity");

    std::printf("native app input events tests passed (%d checks, 0 failures)\n", checks);
    return 0;
}
