// Remappable bindings, their defaults, and their text form.

#include <algorithm>
#include <cstdlib>
#include <initializer_list>
#include <sstream>
#include <utility>

#include "petari/input.hpp"

namespace PetariNative::Input {
namespace {

constexpr int kActionCount = static_cast<int>(Action::Count);

const char* const kActionNames[kActionCount] = {
    "StickUp", "StickDown", "StickLeft", "StickRight", "A",        "B",        "Plus",     "Minus",     "Home",   "One",
    "Two",     "DpadUp",    "DpadDown",  "DpadLeft",   "DpadRight", "NunchukC", "NunchukZ", "Shake",     "TiltHold",
    "PostureToggle", "Walk",   "Start", "ModCollectStarBits", "ModShootEnemy", "CameraOrbitHold", "CameraZoomIn",
    "CameraZoomOut", "CameraOrbitLeft", "CameraOrbitRight", "CameraPitchUp", "CameraPitchDown", "PhotoMode", "PhotoShot",
    "ModCappyThrow", "LevelEditor", "NunchukShake", "CoStarToggle", "CoStarA", "CoStarB", "CoStarShake",
    "PlaytestCapture", "PlayJump",
};

const char* const kMouseNames[static_cast<int>(MouseButton::Count)] = {"Left", "Middle", "Right", "X1", "X2"};

const char* const kPadNames[static_cast<int>(PadButton::Count)] = {
    "South",        "East",          "West",   "North",    "Back",     "Guide",     "Start",       "LeftStick", "RightStick",
    "LeftShoulder", "RightShoulder", "DpadUp", "DpadDown", "DpadLeft", "DpadRight", "LeftTrigger", "RightTrigger",
    "Misc1",        "Shake", "Misc2", "RightPaddle1", "LeftPaddle1", "RightPaddle2", "LeftPaddle2"};

// The same buttons by Nintendo label, for the Pro layouts' sections of controls.txt
// (SDL3 reports positions: the bottom button is South, labelled B on a Nintendo pad).
const char* const kProPadNames[static_cast<int>(PadButton::Count)] = {
    "B", "A", "Y", "X", "Minus", "Home", "Plus", "LStick", "RStick", "L", "R",
    "DpadUp", "DpadDown", "DpadLeft", "DpadRight", "ZL", "ZR", "Capture", "Shake", "C", "GR", "GL", "GR2", "GL2"};

// Player-facing, layout neutral (South is Xbox A, PlayStation Cross, Nintendo B).
const char* const kPadDisplayNames[static_cast<int>(PadButton::Count)] = {
    "Pad bottom",   "Pad right",    "Pad left",      "Pad top",        "Pad Back",       "Pad Guide",
    "Pad Start",    "Pad L3",       "Pad R3",        "Pad LB",         "Pad RB",         "Pad D-pad up",
    "Pad D-pad down", "Pad D-pad left", "Pad D-pad right", "Pad LT", "Pad RT", "Pad Share", "Pad shake",
    "Pad C", "Pad paddle R1", "Pad paddle L1", "Pad paddle R2", "Pad paddle L2"};

const char* const kProPadDisplayNames[static_cast<int>(PadButton::Count)] = {
    "Pad B",        "Pad A",          "Pad Y",           "Pad X",           "Pad -",   "Pad Home", "Pad +",
    "Pad L-stick press", "Pad R-stick press", "Pad L", "Pad R", "Pad D-pad up", "Pad D-pad down", "Pad D-pad left",
    "Pad D-pad right", "Pad ZL", "Pad ZR", "Pad Capture", "Pad shake", "Pad C", "Pad GR", "Pad GL", "Pad GR2", "Pad GL2"};

struct NamedKey {
    const char* name;
    KeyCode code;
};

const NamedKey kKeyNames[] = {
    {"A", Key::A}, {"B", Key::B}, {"C", Key::C}, {"D", Key::D}, {"E", Key::E}, {"F", Key::F}, {"G", Key::G},
    {"H", Key::H}, {"I", Key::I}, {"J", Key::J}, {"K", Key::K}, {"L", Key::L}, {"M", Key::M}, {"N", Key::N},
    {"O", Key::O}, {"P", Key::P}, {"Q", Key::Q}, {"R", Key::R}, {"S", Key::S}, {"T", Key::T}, {"U", Key::U},
    {"V", Key::V}, {"W", Key::W}, {"X", Key::X}, {"Y", Key::Y}, {"Z", Key::Z},
    {"Num1", Key::Num1}, {"Num2", Key::Num2}, {"Num3", Key::Num3}, {"Num4", Key::Num4}, {"Num5", Key::Num5},
    {"Num6", Key::Num6}, {"Num7", Key::Num7}, {"Num8", Key::Num8}, {"Num9", Key::Num9}, {"Num0", Key::Num0},
    {"Return", Key::Return}, {"Escape", Key::Escape}, {"Backspace", Key::Backspace}, {"Tab", Key::Tab},
    {"Space", Key::Space}, {"Minus", Key::Minus}, {"Equals", Key::Equals}, {"LeftBracket", Key::LeftBracket},
    {"RightBracket", Key::RightBracket}, {"Backslash", Key::Backslash}, {"Semicolon", Key::Semicolon},
    {"Apostrophe", Key::Apostrophe}, {"Grave", Key::Grave}, {"Comma", Key::Comma}, {"Period", Key::Period},
    {"Slash", Key::Slash}, {"CapsLock", Key::CapsLock},
    {"F1", Key::F1}, {"F2", Key::F2}, {"F3", Key::F3}, {"F4", Key::F4}, {"F5", Key::F5}, {"F6", Key::F6},
    {"F7", Key::F7}, {"F8", Key::F8}, {"F9", Key::F9}, {"F10", Key::F10}, {"F11", Key::F11}, {"F12", Key::F12},
    {"KeypadEnter", Key::KeypadEnter},
    {"Right", Key::Right}, {"Left", Key::Left}, {"Down", Key::Down}, {"Up", Key::Up},
    {"LeftCtrl", Key::LeftCtrl}, {"LeftShift", Key::LeftShift}, {"LeftAlt", Key::LeftAlt}, {"LeftGui", Key::LeftGui},
    {"RightCtrl", Key::RightCtrl}, {"RightShift", Key::RightShift}, {"RightAlt", Key::RightAlt},
    {"RightGui", Key::RightGui},
};

std::string trim(const std::string& text) {
    const auto begin = text.find_first_not_of(" \t\r");
    if (begin == std::string::npos) {
        return {};
    }
    const auto end = text.find_last_not_of(" \t\r");
    return text.substr(begin, end - begin + 1);
}

bool parseAction(const std::string& name, Action* action) {
    for (int i = 0; i < kActionCount; ++i) {
        if (name == kActionNames[i]) {
            *action = static_cast<Action>(i);
            return true;
        }
    }
    return false;
}

// labels: also accept the Nintendo label names (the Pro layouts' sections).
bool parseBinding(const std::string& text, Binding* binding, bool labels = false) {
    const auto colon = text.find(':');
    if (colon == std::string::npos) {
        return false;
    }
    const std::string device = text.substr(0, colon);
    const std::string name = text.substr(colon + 1);
    if (device == "Key") {
        KeyCode code;
        if (!parseKeyName(name, &code)) {
            return false;
        }
        *binding = Binding::key(code);
        return true;
    }
    if (device == "Mouse") {
        for (int i = 0; i < static_cast<int>(MouseButton::Count); ++i) {
            if (name == kMouseNames[i]) {
                *binding = Binding::mouse(static_cast<MouseButton>(i));
                return true;
            }
        }
    }
    if (device == "Pad") {
        for (int i = 0; labels && i < static_cast<int>(PadButton::Count); ++i) {
            if (name == kProPadNames[i]) {
                *binding = Binding::pad(static_cast<PadButton>(i));
                return true;
            }
        }
        for (int i = 0; i < static_cast<int>(PadButton::Count); ++i) {
            if (name == kPadNames[i]) {
                *binding = Binding::pad(static_cast<PadButton>(i));
                return true;
            }
        }
    }
    return false;
}

// Player-facing names where keyName's differ.
const NamedKey kDisplayNames[] = {
    {"Return", Key::Return},         {"Keypad Enter", Key::KeypadEnter}, {"Left Shift", Key::LeftShift},
    {"Right Shift", Key::RightShift}, {"Left Ctrl", Key::LeftCtrl},       {"Right Ctrl", Key::RightCtrl},
    {"Left Alt", Key::LeftAlt},       {"Right Alt", Key::RightAlt},       {"Left Cmd", Key::LeftGui},
    {"Right Cmd", Key::RightGui},     {"Up arrow", Key::Up},              {"Down arrow", Key::Down},
    {"Left arrow", Key::Left},        {"Right arrow", Key::Right},        {"-", Key::Minus},
    {"=", Key::Equals},               {"Caps Lock", Key::CapsLock},       {"`", Key::Grave},
};

const char* const kMouseDisplayNames[static_cast<int>(MouseButton::Count)] = {
    "Left mouse", "Middle mouse", "Right mouse", "Mouse button 4", "Mouse button 5"};

// The inputs bound to any of the actions, in order, without repeats.
// Controller buttons in the Controls page's own column: no "Pad " prefix.
const char* const kPadShortNames[static_cast<int>(PadButton::Count)] = {
    "bottom", "right", "left", "top", "Back", "Guide", "Start", "L3", "R3", "LB", "RB",
    "D-pad up", "D-pad down", "D-pad left", "D-pad right", "LT", "RT", "Share", "shake",
    "C", "paddle R1", "paddle L1", "paddle R2", "paddle L2"};
const char* const kProPadShortNames[static_cast<int>(PadButton::Count)] = {
    "B", "A", "Y", "X", "-", "Home", "+", "L-stick press", "R-stick press", "L", "R",
    "D-pad up", "D-pad down", "D-pad left", "D-pad right", "ZL", "ZR", "Capture", "shake", "C", "GR", "GL", "GR2", "GL2"};

bool proLayout(PadLayout layout) {
    return layout == PadLayout::ProGalaxy || layout == PadLayout::ProOdyssey;
}

enum class Devices { KeyboardMouse, Pad };

bool isPad(const Binding& input) {
    return input.device == Binding::Device::Pad;
}

std::vector<Binding> inputsOf(const Bindings& bindings, std::initializer_list<Action> actions,
                              Devices devices = Devices::KeyboardMouse) {
    std::vector<Binding> result;
    for (const Action action : actions) {
        for (const Binding& input : bindings.inputs(action)) {
            if (isPad(input) != (devices == Devices::Pad)) {
                continue;
            }
            if (std::find(result.begin(), result.end(), input) == result.end()) {
                result.push_back(input);
            }
        }
    }
    return result;
}

std::string joined(const std::vector<Binding>& inputs, const char* separator, PadLayout layout = PadLayout::Standard) {
    if (inputs.empty()) {
        return "(not bound)";
    }
    std::string text;
    for (size_t i = 0; i < inputs.size(); ++i) {
        if (i > 0) {
            text += separator;
        }
        // Left and right modifiers bound together read as one key.
        const Binding& input = inputs[i];
        if (input.device == Binding::Device::Key && i + 1 < inputs.size()) {
            const Binding& next = inputs[i + 1];
            const std::string name = displayName(input);
            const std::string nextName = displayName(next);
            if (name.rfind("Left ", 0) == 0 && nextName == "Right " + name.substr(5)) {
                text += name.substr(5);
                ++i;
                continue;
            }
        }
        text += isPad(input) && input.code < static_cast<int>(PadButton::Count)
                    ? (proLayout(layout) ? kProPadShortNames : kPadShortNames)[input.code]
                    : displayName(input);
    }
    return text;
}

// Controller buttons of the actions, or "" when none are bound.
std::string padText(const Bindings& b, std::initializer_list<Action> actions, const std::string& prefix = "",
                    const std::string& suffix = "") {
    const std::vector<Binding> inputs = inputsOf(b, actions, Devices::Pad);
    return inputs.empty() ? std::string() : prefix + joined(inputs, " / ", b.layout()) + suffix;
}

// The first keyboard or mouse input of each action, e.g. "W A S D". Actions
// without one show "-".
std::string firstOfEach(const Bindings& bindings, std::initializer_list<Action> actions, const char* separator) {
    std::string text;
    bool any = false;
    for (const Action action : actions) {
        if (!text.empty()) {
            text += separator;
        }
        const std::vector<Binding> inputs = inputsOf(bindings, {action});
        text += inputs.empty() ? "-" : displayName(inputs.front());
        any = any || !inputs.empty();
    }
    return any ? text : "(not bound)";
}

}  // namespace

std::string displayName(Binding input, PadLayout layout) {
    if (input.device == Binding::Device::Pad) {
        if (input.code >= static_cast<int>(PadButton::Count)) return "Pad";
        return (proLayout(layout) ? kProPadDisplayNames : kPadDisplayNames)[input.code];
    }
    if (input.device == Binding::Device::Mouse) {
        return input.code < static_cast<int>(MouseButton::Count) ? kMouseDisplayNames[input.code] : "Mouse";
    }
    for (const NamedKey& key : kDisplayNames) {
        if (key.code == input.code) {
            return key.name;
        }
    }
    std::string name = keyName(input.code);
    if (name.size() == 4 && name.rfind("Num", 0) == 0) {
        return name.substr(3);  // Num1 -> 1
    }
    return name;
}

std::vector<ControlsLine> controlsSummary(const Bindings& b) {
    const std::string move = firstOfEach(b, {Action::StickUp, Action::StickLeft, Action::StickDown, Action::StickRight}, " ");
    // "Hold <inputs><suffix>", or "(not bound)".
    auto hold = [&b](std::initializer_list<Action> actions, const std::string& suffix) {
        const std::vector<Binding> inputs = inputsOf(b, actions);
        return inputs.empty() ? std::string("(not bound)") : "Hold " + joined(inputs, " / ") + suffix;
    };
    const std::string padLeave = padText(b, {Action::DpadDown});
    const std::string padRotate = padText(b, {Action::DpadLeft}) + (padText(b, {Action::DpadRight}).empty() ? "" : " / ") +
                                  padText(b, {Action::DpadRight});
    return {
        {"", "Keyboard and mouse", b.layout() == PadLayout::ProOdyssey ? "Pro Controller (Odyssey layout)"
                                   : b.layout() == PadLayout::ProGalaxy ? "Pro Controller (Galaxy layout)"
                                                                         : "Controller"},
        {"Move", move, "left stick"},
        {"Jump / confirm", joined(inputsOf(b, {Action::A}), " / "), padText(b, {Action::A, Action::Start, Action::PlayJump})},
        {"Start (title: A and B)", joined(inputsOf(b, {Action::Start}), " / "), padText(b, {Action::Start})},
        {"Spin", joined(inputsOf(b, {Action::Shake}), " / "), padText(b, {Action::Shake})},
        {"Shake Nunchuk", joined(inputsOf(b, {Action::NunchukShake}), " / "), padText(b, {Action::NunchukShake})},
        {"Co-star (toggle / A / B / shake)",
         firstOfEach(b, {Action::CoStarToggle, Action::CoStarA, Action::CoStarB, Action::CoStarShake}, " / "),
         padText(b, {Action::CoStarToggle, Action::CoStarA, Action::CoStarB, Action::CoStarShake})},
        {"Cap throw (Cappy mod)", joined(inputsOf(b, {Action::ModCappyThrow}), " / "), padText(b, {Action::ModCappyThrow})},
        {"Crouch / ground pound", joined(inputsOf(b, {Action::NunchukZ}), " / "), padText(b, {Action::NunchukZ})},
        {"Star Pointer", "Mouse", b.layout() == PadLayout::ProOdyssey ? "gyro (R3: center)"
                                  : proLayout(b.layout())            ? "right stick or gyro (R3: center)"
                                                                     : "right stick (R3: center)"},
        {"Shoot Star Bits / back", joined(inputsOf(b, {Action::B}), " / "), padText(b, {Action::B})},
        {"Grab (Pull Stars)", hold({Action::A}, " on the target"), padText(b, {Action::A}, "hold ")},
        {"Rotate camera", firstOfEach(b, {Action::DpadLeft, Action::DpadRight}, " / "), padRotate},
        {"Recenter camera", joined(inputsOf(b, {Action::NunchukC}), " / "), padText(b, {Action::NunchukC})},
        {"First-person view",
         joined(inputsOf(b, {Action::DpadUp}), " / ") + " (leave: " + joined(inputsOf(b, {Action::DpadDown}), " / ") + ")",
         padText(b, {Action::DpadUp}, "", padLeave.empty() ? "" : " (leave: " + padLeave + ")")},
        {"Walk slowly", hold({Action::Walk}, ""), "push the stick part way"},
        {"Pause", joined(inputsOf(b, {Action::Plus, Action::Minus}), " / "), padText(b, {Action::Plus, Action::Minus})},
        {"Star Ball", move + " tilt while riding", "left stick tilts while riding"},
        {"Tilt the remote by hand", hold({Action::TiltHold}, " + " + move), ""},
        {"This menu", joined(inputsOf(b, {Action::Home}), " / "), padText(b, {Action::Home})},
    };
}

std::string titleHint(const Bindings& b) {
    // The keyboard input of an action if it has one, else its first input.
    auto preferKey = [&b](Action action) {
        const std::vector<Binding>& inputs = b.inputs(action);
        for (const Binding& input : inputs) {
            if (input.device == Binding::Device::Key) {
                return displayName(input);
            }
        }
        return displayName(inputs.front());
    };
    std::string start;
    if (!b.inputs(Action::Start).empty()) {
        start = preferKey(Action::Start) + " starts";
    } else if (!b.inputs(Action::A).empty() && !b.inputs(Action::B).empty()) {
        start = "hold " + preferKey(Action::A) + ", then press " + preferKey(Action::B) + " to start";
    } else {
        start = "bind Start in controls.txt to start";
    }
    std::string text = "Keyboard: " + start;
    if (!b.inputs(Action::Home).empty()) {
        text += "   |   " + preferKey(Action::Home) + ": all controls";
    }
    return text;
}

Bindings Bindings::defaults(PadLayout layout) {
    Bindings b;
    // Approved (native/CONTROLS.md).
    b.bind(Action::StickUp, Binding::key(Key::W));
    b.bind(Action::StickDown, Binding::key(Key::S));
    b.bind(Action::StickLeft, Binding::key(Key::A));
    b.bind(Action::StickRight, Binding::key(Key::D));
    b.bind(Action::A, Binding::key(Key::Space));
    b.bind(Action::A, Binding::mouse(MouseButton::Right));  // Pull Stars (GCapture) and pointer grabs use A
    b.bind(Action::B, Binding::mouse(MouseButton::Left));   // Star Bits
    b.bind(Action::B, Binding::key(Key::B));  // Star Bits too; with Point to move, left click steers (native/MODS.md)
    b.bind(Action::Shake, Binding::key(Key::F));
    b.bind(Action::NunchukZ, Binding::key(Key::LeftShift));
    b.bind(Action::NunchukZ, Binding::key(Key::RightShift));
    b.bind(Action::DpadLeft, Binding::key(Key::Q));   // camera rotation (CameraLocalUtil)
    b.bind(Action::DpadRight, Binding::key(Key::E));
    b.bind(Action::NunchukC, Binding::key(Key::C));   // camera recenter
    b.bind(Action::Plus, Binding::key(Key::Escape));  // pause
    b.bind(Action::B, Binding::key(Key::Backspace));  // back in menus
    // Provisional, pending gameplay review.
    b.bind(Action::DpadUp, Binding::key(Key::Up));  // first-person view
    b.bind(Action::DpadDown, Binding::key(Key::Down));
    b.bind(Action::DpadLeft, Binding::key(Key::Left));
    b.bind(Action::DpadRight, Binding::key(Key::Right));
    b.bind(Action::Minus, Binding::key(Key::Minus));
    b.bind(Action::One, Binding::key(Key::Num1));
    b.bind(Action::Two, Binding::key(Key::Num2));
    b.bind(Action::Home, Binding::key(Key::F1));
    b.bind(Action::Home, Binding::key(Key::Grave));  // ` (under Escape), for keyboards without a usable F1
    b.bind(Action::TiltHold, Binding::key(Key::Tab));
    b.bind(Action::PostureToggle, Binding::key(Key::T));
    b.bind(Action::Walk, Binding::key(Key::LeftAlt));
    // Return confirms (A). On the title's "press A and B" prompt it is A and
    // B together; it never sends B elsewhere, where B backs out of menus.
    b.bind(Action::Start, Binding::key(Key::Return));
    b.bind(Action::Start, Binding::key(Key::KeypadEnter));
    // Mods (off by default, native/MODS.md): keys the game does not use.
    b.bind(Action::ModCollectStarBits, Binding::key(Key::G));
    b.bind(Action::ModShootEnemy, Binding::key(Key::V));
    b.bind(Action::ModCappyThrow, Binding::key(Key::R));
    // Odyssey camera mod (off by default): hold-to-orbit for mice without a
    // middle button (Command), and zoom keys.
    b.bind(Action::CameraOrbitHold, Binding::mouse(MouseButton::Middle));
    b.bind(Action::CameraOrbitHold, Binding::key(Key::LeftGui));
    b.bind(Action::CameraZoomIn, Binding::key(Key::Z));
    b.bind(Action::CameraZoomOut, Binding::key(Key::X));
    b.bind(Action::CameraOrbitLeft, Binding::key(Key::J));
    b.bind(Action::CameraOrbitRight, Binding::key(Key::L));
    b.bind(Action::CameraPitchUp, Binding::key(Key::I));
    b.bind(Action::CameraPitchDown, Binding::key(Key::K));
    b.bind(Action::PhotoMode, Binding::key(Key::P));
    b.bind(Action::PhotoShot, Binding::key(Key::O));
    b.bind(Action::LevelEditor, Binding::key(Key::Tab));
    // Game controllers: the usual console layout for this game. The left
    // stick is the Nunchuk stick and the right stick the Star Pointer (not
    // bindings; see RemoteModel). Start and Back would block pausing if they
    // also sent A or B, so the title's A and B is the top button (Start).
    b.bind(Action::A, Binding::pad(PadButton::South));
    b.bind(Action::B, Binding::pad(PadButton::East));
    b.bind(Action::B, Binding::pad(PadButton::RightTrigger));  // Star Bits
    b.bind(Action::Shake, Binding::pad(PadButton::West));
    b.bind(Action::Shake, Binding::pad(PadButton::RightShoulder));
    b.bind(Action::Start, Binding::pad(PadButton::North));
    b.bind(Action::NunchukZ, Binding::pad(PadButton::LeftTrigger));
    b.bind(Action::NunchukC, Binding::pad(PadButton::LeftShoulder));
    b.bind(Action::DpadUp, Binding::pad(PadButton::DpadUp));
    b.bind(Action::DpadDown, Binding::pad(PadButton::DpadDown));
    b.bind(Action::DpadLeft, Binding::pad(PadButton::DpadLeft));
    b.bind(Action::DpadRight, Binding::pad(PadButton::DpadRight));
    b.bind(Action::Plus, Binding::pad(PadButton::Start));
    b.bind(Action::Minus, Binding::pad(PadButton::Back));
    b.bind(Action::Home, Binding::pad(PadButton::Guide));
    // The only controller button left free; ModShootEnemy has no controller default.
    b.bind(Action::ModCollectStarBits, Binding::pad(PadButton::LeftStick));
    b.bind(Action::PhotoMode, Binding::pad(PadButton::RightStick));
    // Cappy mod: RB, Spin's second button; while the mod is active RB throws and
    // does not spin (setCappyClaimsShake). The left button still spins.
    b.bind(Action::ModCappyThrow, Binding::pad(PadButton::RightShoulder));
    // SMG2 additions. Append actions/defaults so every imported binding and
    // serialized action name keeps its meaning. These keys were unassigned.
    b.bind(Action::NunchukShake, Binding::key(Key::H));
    b.bind(Action::CoStarToggle, Binding::key(Key::F2));
    b.bind(Action::CoStarA, Binding::key(Key::U));
    b.bind(Action::CoStarB, Binding::key(Key::Y));
    b.bind(Action::CoStarShake, Binding::key(Key::N));
    // Pads with Nintendo labels (native/CONTROLS.md "Switch Pro Controller"): the
    // Galaxy layout, and the Odyssey layout while OdysseyMovement is on. Codes are
    // positions; the comments give the Pro Controller's labels.
    for (const PadLayout pro : {PadLayout::ProGalaxy, PadLayout::ProOdyssey}) {
        const bool odyssey = pro == PadLayout::ProOdyssey;
        b.setLayout(pro);
        b.bind(Action::PlayJump, Binding::pad(PadButton::South));  // B: jump in play, back in menus
        b.bind(Action::PlayJump, Binding::pad(PadButton::RightPaddle1));  // GR: keep the thumb on the pointer stick
        b.bind(Action::Start, Binding::pad(PadButton::East));  // A: jump (A then B together at the title prompt)
        // Y / X: spin, or in the Odyssey layout throw Cappy (which spins while Cappy is off).
        const Action face = odyssey ? Action::ModCappyThrow : Action::Shake;
        b.bind(face, Binding::pad(PadButton::West));
        b.bind(face, Binding::pad(PadButton::North));
        b.bind(Action::Shake, Binding::pad(PadButton::RightShoulder));  // R
        b.bind(Action::Shake, Binding::pad(PadButton::Shake));          // shake the controller
        b.bind(face, Binding::pad(PadButton::LeftPaddle1));  // GL: spin, or throw Cappy in Odyssey
        b.bind(Action::NunchukZ, Binding::pad(PadButton::LeftTrigger));  // ZL
        b.bind(Action::B, Binding::pad(PadButton::RightTrigger));        // ZR: Star Bits, back in menus
        b.bind(Action::NunchukC, Binding::pad(PadButton::LeftShoulder));  // L: center the camera
        b.bind(Action::DpadUp, Binding::pad(PadButton::DpadUp));
        b.bind(Action::DpadDown, Binding::pad(PadButton::DpadDown));
        if (odyssey) b.bind(Action::PhotoMode, Binding::pad(PadButton::DpadDown));
        b.bind(Action::DpadLeft, Binding::pad(PadButton::DpadLeft));
        b.bind(Action::DpadRight, Binding::pad(PadButton::DpadRight));
        b.bind(Action::Plus, Binding::pad(PadButton::Start));  // +
        b.bind(Action::Minus, Binding::pad(PadButton::Back));  // -
        b.bind(Action::Home, Binding::pad(PadButton::Guide));
        b.bind(Action::PlaytestCapture, Binding::pad(PadButton::Misc1));  // Capture
        b.bind(Action::ModCollectStarBits, Binding::pad(PadButton::LeftStick));
        // R3 recenters the pointer (RemoteModel), with no action.
    }
    b.setLayout(layout);
    return b;
}

std::vector<Binding>& Bindings::padSet(int layout, int action) {
    return layout == 0 ? mInputs[action] : mPro[layout - 1][action];
}

void Bindings::rebuild(int action) {
    if (!proLayout(mLayout)) return;
    std::vector<Binding>& view = mView[action];
    view.clear();
    for (const Binding& input : mInputs[action]) {
        if (!isPad(input)) view.push_back(input);
    }
    const auto& pads = mPro[static_cast<int>(mLayout) - 1][action];
    view.insert(view.end(), pads.begin(), pads.end());
}

void Bindings::setLayout(PadLayout layout) {
    if (static_cast<int>(layout) >= static_cast<int>(PadLayout::Count)) layout = PadLayout::Standard;
    mLayout = layout;
    for (int i = 0; i < kActions; ++i) rebuild(i);
}

void Bindings::bind(Action action, Binding input) {
    const int a = static_cast<int>(action);
    auto& inputs = isPad(input) ? padSet(static_cast<int>(mLayout), a) : mInputs[a];
    if (std::find(inputs.begin(), inputs.end(), input) == inputs.end()) {
        // In a Pro layout keyboard and mouse inputs go before the Standard pad's,
        // where the defaults keep them.
        auto at = inputs.end();
        if (proLayout(mLayout) && !isPad(input)) at = std::find_if(inputs.begin(), inputs.end(), isPad);
        inputs.insert(at, input);
    }
    rebuild(a);
}

void Bindings::unbind(Action action, Binding input) {
    const int a = static_cast<int>(action);
    auto& inputs = isPad(input) ? padSet(static_cast<int>(mLayout), a) : mInputs[a];
    inputs.erase(std::remove(inputs.begin(), inputs.end(), input), inputs.end());
    rebuild(a);
}

void Bindings::clear(Action action) {
    const int a = static_cast<int>(action);
    if (proLayout(mLayout)) {
        // The keyboard and mouse inputs and this layout's buttons; the Standard pad's stay.
        mInputs[a].erase(std::remove_if(mInputs[a].begin(), mInputs[a].end(), [](const Binding& b) { return !isPad(b); }),
                         mInputs[a].end());
        mPro[static_cast<int>(mLayout) - 1][a].clear();
    } else {
        mInputs[a].clear();
    }
    rebuild(a);
}

const std::vector<Binding>& Bindings::inputs(Action action) const {
    const int a = static_cast<int>(action);
    return proLayout(mLayout) ? mView[a] : mInputs[a];
}

const char* padLayoutName(PadLayout layout) {
    switch (layout) {
    case PadLayout::Standard: return "Standard";
    case PadLayout::ProGalaxy: return "ProGalaxy";
    case PadLayout::ProOdyssey: return "ProOdyssey";
    default: return "?";
    }
}

PadLayout padLayoutFor(bool nintendoLabels, bool odysseyMovement) {
    if (!nintendoLabels) return PadLayout::Standard;
    return odysseyMovement ? PadLayout::ProOdyssey : PadLayout::ProGalaxy;
}

std::string Bindings::serialize() const {
    std::ostringstream out;
    for (int i = 0; i < kActionCount; ++i) {
        out << kActionNames[i] << '=';
        bool first = true;
        for (const Binding& input : mInputs[i]) {
            if (!first) {
                out << ',';
            }
            first = false;
            if (input.device == Binding::Device::Key) {
                out << "Key:" << keyName(input.code);
            } else if (input.device == Binding::Device::Mouse) {
                out << "Mouse:" << kMouseNames[input.code];
            } else {
                out << "Pad:" << kPadNames[input.code];
            }
        }
        out << '\n';
    }
    // The Pro layouts and options only where they differ from the defaults, so a
    // file with keyboard remaps alone still reads in builds without them.
    const Bindings d = defaults();
    for (int l = 0; l < kProLayouts; ++l) {
        bool differs = false;
        for (int i = 0; i < kActions && !differs; ++i) differs = mPro[l][i] != d.mPro[l][i];
        if (!differs) continue;
        out << "\n[" << padLayoutName(static_cast<PadLayout>(l + 1)) << "]\n";
        // Distinguish an intentional unbinding of every new button from a
        // saved pre-Switch-2 section, which lists the old defaults in full.
        out << "# PadExtras=explicit\n";
        for (int i = 0; i < kActions; ++i) {
            out << kActionNames[i] << '=';
            for (size_t k = 0; k < mPro[l][i].size(); ++k) {
                out << (k > 0 ? "," : "") << "Pad:" << kProPadNames[mPro[l][i][k].code];
            }
            out << '\n';
        }
    }
    if (!(mOptions == d.mOptions)) {
        static const char* const kGyro[] = {"auto", "on", "off"};
        out << "\n[Controller]\n";
        out << "Gyro=" << kGyro[static_cast<int>(mOptions.gyro)] << '\n';
        out << "GyroSpeed=" << mOptions.gyroSpeed << '\n';
        out << "GyroInvertX=" << (mOptions.gyroInvertX ? "on" : "off") << '\n';
        out << "GyroInvertY=" << (mOptions.gyroInvertY ? "on" : "off") << '\n';
        out << "Rumble=" << (mOptions.rumble ? "on" : "off") << '\n';
        out << "ShakeG=" << mOptions.shakeG << '\n';
    }
    return out.str();
}

namespace {
bool parseSwitch(const std::string& value, bool* out) {
    if (value == "on") return *out = true, true;
    if (value == "off") return *out = false, true;
    return false;
}

bool parseNumber(const std::string& value, float low, float high, float* out) {
    char* end = nullptr;
    const float number = std::strtof(value.c_str(), &end);
    if (value.empty() || *end != '\0' || !(number >= low && number <= high)) return false;
    *out = number;
    return true;
}

bool parseOption(PadOptions& options, const std::string& key, const std::string& value) {
    if (key == "Gyro") {
        if (value == "auto") options.gyro = PadOptions::Gyro::Auto;
        else if (value == "on") options.gyro = PadOptions::Gyro::On;
        else if (value == "off") options.gyro = PadOptions::Gyro::Off;
        else return false;
        return true;
    }
    if (key == "GyroSpeed") return parseNumber(value, 0.05f, 10.0f, &options.gyroSpeed);
    if (key == "GyroInvertX") return parseSwitch(value, &options.gyroInvertX);
    if (key == "GyroInvertY") return parseSwitch(value, &options.gyroInvertY);
    if (key == "Rumble") return parseSwitch(value, &options.rumble);
    if (key == "ShakeG") return parseNumber(value, 0.5f, 8.0f, &options.shakeG);
    return false;
}
}  // namespace

bool Bindings::parse(const std::string& text, std::string* error) {
    Bindings result = *this;
    result.setLayout(PadLayout::Standard);
    enum { Main, Pro, Options } section = Main;
    bool proSeen[kProLayouts]{};
    bool explicitExtras[kProLayouts]{};
    std::istringstream in(text);
    std::string line;
    int lineNumber = 0;
    auto fail = [&](const std::string& message) {
        if (error != nullptr) {
            *error = "line " + std::to_string(lineNumber) + ": " + message;
        }
        return false;
    };
    while (std::getline(in, line)) {
        ++lineNumber;
        line = trim(line);
        if (section == Pro && line == "# PadExtras=explicit") {
            explicitExtras[static_cast<int>(result.mLayout) - 1] = true;
        }
        if (line.empty() || line[0] == '#') {
            continue;
        }
        if (line.front() == '[' && line.back() == ']') {
            const std::string name = trim(line.substr(1, line.size() - 2));
            if (name == "ProGalaxy" || name == "ProOdyssey") {
                section = Pro;
                result.setLayout(name == "ProGalaxy" ? PadLayout::ProGalaxy : PadLayout::ProOdyssey);
                proSeen[static_cast<int>(result.mLayout) - 1] = true;
            } else if (name == "Controller") {
                section = Options;
            } else {
                return fail("unknown section \"" + name + "\" (expected ProGalaxy, ProOdyssey or Controller)");
            }
            continue;
        }
        const auto equals = line.find('=');
        if (equals == std::string::npos) {
            return fail(section == Options ? "expected Option=value" : "expected Action=Input,...");
        }
        if (section == Options) {
            const std::string key = trim(line.substr(0, equals));
            if (!parseOption(result.mOptions, key, trim(line.substr(equals + 1)))) {
                return fail("bad controller option \"" + line + "\"");
            }
            continue;
        }
        Action action;
        if (!parseAction(trim(line.substr(0, equals)), &action)) {
            return fail("unknown action \"" + trim(line.substr(0, equals)) + "\"");
        }
        if (section == Pro) {
            // Only this layout's buttons: keyboard and mouse inputs are in the main section.
            result.mPro[static_cast<int>(result.mLayout) - 1][static_cast<int>(action)].clear();
        } else {
            result.clear(action);
        }
        std::istringstream list(line.substr(equals + 1));
        std::string item;
        while (std::getline(list, item, ',')) {
            item = trim(item);
            if (item.empty()) {
                continue;
            }
            Binding binding;
            if (!parseBinding(item, &binding, section == Pro)) {
                return fail("unknown input \"" + item + "\"");
            }
            if (section == Pro && binding.device != Binding::Device::Pad) {
                return fail("only controller buttons in a Pro layout section: \"" + item + "\"");
            }
            if (section == Pro && binding.code >= static_cast<int>(PadButton::Misc2) &&
                binding.code < static_cast<int>(PadButton::Count)) {
                explicitExtras[static_cast<int>(result.mLayout) - 1] = true;
            }
            result.bind(action, binding);
        }
    }
    // Older saves serialize every action of a changed Pro layout. Preserve
    // their remaps and add the new grip defaults when that section has never
    // mentioned an extra button. New saves mark explicit empty bindings too.
    for (int l = 0; l < kProLayouts; ++l) {
        if (!proSeen[l] || explicitExtras[l]) continue;
        result.setLayout(static_cast<PadLayout>(l + 1));
        result.bind(Action::PlayJump, Binding::pad(PadButton::RightPaddle1));
        result.bind(l == 0 ? Action::Shake : Action::ModCappyThrow, Binding::pad(PadButton::LeftPaddle1));
    }
    result.setLayout(mLayout);
    *this = std::move(result);
    return true;
}

const char* actionName(Action action) {
    const int index = static_cast<int>(action);
    return index >= 0 && index < kActionCount ? kActionNames[index] : "?";
}

std::string keyName(KeyCode code) {
    for (const NamedKey& key : kKeyNames) {
        if (key.code == code) {
            return key.name;
        }
    }
    return "Usage" + std::to_string(code);
}

bool parseKeyName(const std::string& name, KeyCode* code) {
    for (const NamedKey& key : kKeyNames) {
        if (name == key.name) {
            *code = key.code;
            return true;
        }
    }
    if (name.rfind("Usage", 0) == 0 && name.size() > 5) {
        char* end = nullptr;
        const long value = std::strtol(name.c_str() + 5, &end, 10);
        if (*end == '\0' && value > 0 && value < Key::Max) {
            *code = static_cast<KeyCode>(value);
            return true;
        }
    }
    return false;
}

Viewport Viewport::letterbox(float windowWidth, float windowHeight, float imageAspect) {
    Viewport v;
    v.windowWidth = windowWidth;
    v.windowHeight = windowHeight;
    if (windowWidth <= 0.0f || windowHeight <= 0.0f || imageAspect <= 0.0f) {
        return v;
    }
    if (windowWidth / windowHeight > imageAspect) {
        v.imageHeight = windowHeight;
        v.imageWidth = windowHeight * imageAspect;
        v.imageX = (windowWidth - v.imageWidth) * 0.5f;
    } else {
        v.imageWidth = windowWidth;
        v.imageHeight = windowWidth / imageAspect;
        v.imageY = (windowHeight - v.imageHeight) * 0.5f;
    }
    return v;
}

}  // namespace PetariNative::Input
