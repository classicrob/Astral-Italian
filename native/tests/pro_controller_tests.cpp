// Switch Pro Controller layouts (native/CONTROLS.md "Switch Pro Controller"):
// the label-bound default tables, the automatic Galaxy/Odyssey switch with the
// OdysseyMovement mod, controls.txt sections, gyro pointer, shake as spin, the
// Capture button and rumble's option. Headless: the report model and host API,
// no running game and no real controller.
#include "petari/input.hpp"
#include "petari/input_remap.hpp"
#include "petari/mods.hpp"
#include "petari/point_move.hpp"
#include "../input/remote_model.hpp"
#include <revolution/kpad.h>
#include <revolution/os.h>
#ifdef PETARI_INPUT_TEST_SDL3
#include "petari/input_sdl3.hpp"
#endif

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <unistd.h>
#include <vector>

namespace In = PetariNative::Input;
namespace Mods = PetariNative::Mods;
namespace Remap = In::Remap;
using In::Action;
using In::Binding;
using In::PadButton;
using In::PadLayout;
using In::Detail::RemoteModel;

namespace {
int checks = 0;
int failures = 0;
void check(bool condition, const std::string& description) {
    ++checks;
    if (!condition) {
        ++failures;
        std::fprintf(stderr, "FAIL: %s\n", description.c_str());
    }
}

std::vector<PadButton> pads(const In::Bindings& b, Action action) {
    std::vector<PadButton> result;
    for (const Binding& input : b.inputs(action)) {
        if (input.device == Binding::Device::Pad) result.push_back(static_cast<PadButton>(input.code));
    }
    return result;
}

std::vector<Binding> keys(const In::Bindings& b, Action action) {
    std::vector<Binding> result;
    for (const Binding& input : b.inputs(action)) {
        if (input.device != Binding::Device::Pad) result.push_back(input);
    }
    return result;
}

// The spec table (PRO-CONTROLLER-TASK.md), by Nintendo label -> SDL position.
constexpr PadButton kA = PadButton::East, kB = PadButton::South, kX = PadButton::North, kY = PadButton::West;
constexpr PadButton kL = PadButton::LeftShoulder, kR = PadButton::RightShoulder, kZL = PadButton::LeftTrigger,
                    kZR = PadButton::RightTrigger, kPlus = PadButton::Start, kMinus = PadButton::Back,
                    kHome = PadButton::Guide, kCapture = PadButton::Misc1, kShake = PadButton::Shake,
                    kL3 = PadButton::LeftStick, kGR = PadButton::RightPaddle1, kGL = PadButton::LeftPaddle1,
                    kC = PadButton::Misc2;

std::map<Action, std::vector<PadButton>> expectedPro(bool odyssey) {
    std::map<Action, std::vector<PadButton>> t;
    t[Action::PlayJump] = {kB, kGR};        // B / GR: jump in play, back in menus
    t[Action::Start] = {kA};                // A: jump; A+B at the title prompt
    if (odyssey) {
        t[Action::ModCappyThrow] = {kY, kX, kGL};  // Y / X / GL: throw Cappy
        t[Action::Shake] = {kR, kShake};      // R, shake: spin
    } else {
        t[Action::Shake] = {kY, kX, kR, kShake, kGL};  // Y / X, R, shake, GL: spin
    }
    t[Action::NunchukZ] = {kZL};
    t[Action::B] = {kZR};
    t[Action::NunchukC] = {kL};
    t[Action::DpadUp] = {PadButton::DpadUp};
    t[Action::DpadDown] = {PadButton::DpadDown};
    if (odyssey) t[Action::PhotoMode] = {PadButton::DpadDown};  // down = photo mode
    t[Action::DpadLeft] = {PadButton::DpadLeft};
    t[Action::DpadRight] = {PadButton::DpadRight};
    t[Action::Plus] = {kPlus};
    t[Action::Minus] = {kMinus};
    t[Action::Home] = {kHome};
    t[Action::PlaytestCapture] = {kCapture};
    t[Action::ModCollectStarBits] = {kL3};
    return t;
}

void testTables() {
    const In::Bindings standard = In::Bindings::defaults();
    for (const bool odyssey : {false, true}) {
        const PadLayout layout = odyssey ? PadLayout::ProOdyssey : PadLayout::ProGalaxy;
        const In::Bindings b = In::Bindings::defaults(layout);
        check(b.layout() == layout, "defaults(layout) is seen through that layout");
        const auto expected = expectedPro(odyssey);
        for (int a = 0; a < static_cast<int>(Action::Count); ++a) {
            const Action action = static_cast<Action>(a);
            const auto it = expected.find(action);
            const std::vector<PadButton> want = it == expected.end() ? std::vector<PadButton>{} : it->second;
            check(pads(b, action) == want, std::string(In::padLayoutName(layout)) + " pad buttons of " + In::actionName(action));
            check(keys(b, action) == keys(standard, action),
                  std::string("keyboard and mouse unchanged in ") + In::padLayoutName(layout) + ": " + In::actionName(action));
        }
    }
    // The Standard (Xbox/PlayStation) layout keeps the imported positional defaults.
    check(pads(standard, Action::A) == std::vector<PadButton>{PadButton::South}, "Standard: bottom = A");
    check(pads(standard, Action::Shake) == (std::vector<PadButton>{PadButton::West, PadButton::RightShoulder}), "Standard spin");
    check(pads(standard, Action::PlaytestCapture).empty() && keys(standard, Action::PlaytestCapture).empty(),
          "Standard: the capture action has no default (F9/M stay with the app)");
    check(standard.serialize().find('[') == std::string::npos, "the defaults write no sections (older builds read the file)");
    check(In::padLayoutFor(false, true) == PadLayout::Standard && In::padLayoutFor(true, false) == PadLayout::ProGalaxy &&
              In::padLayoutFor(true, true) == PadLayout::ProOdyssey,
          "padLayoutFor");
    // Labels in the player's view.
    check(In::displayName(Binding::pad(kB), PadLayout::ProGalaxy) == "Pad B" &&
              In::displayName(Binding::pad(kZR), PadLayout::ProOdyssey) == "Pad ZR" &&
              In::displayName(Binding::pad(kB)) == "Pad bottom",
          "Nintendo labels shown in a Pro layout, positions otherwise");
    bool header = false;
    for (const auto& line : In::controlsSummary(In::Bindings::defaults(PadLayout::ProOdyssey))) {
        header |= line.pad == "Pro Controller (Odyssey layout)";
        if (line.action == "Jump / confirm") check(line.pad == "A / B / GR", "summary: A, B and GR jump (" + line.pad + ")");
        if (line.action == "Cap throw (Cappy mod)") check(line.pad == "Y / X / GL", "summary: Y / X / GL throw Cappy");
        if (line.action == "Shoot Star Bits / back") check(line.pad == "ZR", "summary: ZR shoots");
    }
    check(header, "summary names the Pro layout");
    check(In::controlsSummary(In::Bindings::defaults(PadLayout::ProGalaxy)).size() <= 20, "summary fits the page");
}

void testExtraButtonNames() {
    // Earlier codes are persisted in bindings and indexed in the report model.
    const PadButton oldButtons[] = {PadButton::South, PadButton::East, PadButton::West, PadButton::North,
        PadButton::Back, PadButton::Guide, PadButton::Start, PadButton::LeftStick, PadButton::RightStick,
        PadButton::LeftShoulder, PadButton::RightShoulder, PadButton::DpadUp, PadButton::DpadDown,
        PadButton::DpadLeft, PadButton::DpadRight, PadButton::LeftTrigger, PadButton::RightTrigger,
        PadButton::Misc1, PadButton::Shake};
    for (int i = 0; i < 19; ++i) check(static_cast<int>(oldButtons[i]) == i, "stable earlier button " + std::to_string(i));
    const PadButton buttons[] = {kC, kGR, kGL, PadButton::RightPaddle2, PadButton::LeftPaddle2};
    const char* neutral[] = {"Misc2", "RightPaddle1", "LeftPaddle1", "RightPaddle2", "LeftPaddle2"};
    const char* pro[] = {"C", "GR", "GL", "GR2", "GL2"};
    const char* display[] = {"Pad C", "Pad paddle R1", "Pad paddle L1", "Pad paddle R2", "Pad paddle L2"};
    check(static_cast<int>(PadButton::Count) == 24, "24 bindable pad buttons");
    for (int i = 0; i < 5; ++i) {
        check(static_cast<int>(buttons[i]) == 19 + i, "new button appended " + std::string(neutral[i]));
        check(In::displayName(Binding::pad(buttons[i])) == display[i], "neutral display " + std::string(neutral[i]));
        for (const PadLayout layout : {PadLayout::Standard, PadLayout::ProGalaxy, PadLayout::ProOdyssey}) {
            const bool isPro = layout != PadLayout::Standard;
            In::Bindings b = In::Bindings::defaults(layout);
            const std::string label = std::string(In::padLayoutName(layout)) + " " + neutral[i];
            if (isPro) check(In::displayName(Binding::pad(buttons[i]), layout) == std::string("Pad ") + pro[i], label + " display");
            else {
                for (int a = 0; a < static_cast<int>(Action::Count); ++a) {
                    const auto bound = pads(b, static_cast<Action>(a));
                    check(std::find(bound.begin(), bound.end(), buttons[i]) == bound.end(), label + " has no default");
                }
            }
            // Every new name round trips in both naming schemes, including the
            // usually unbound second pair of paddles.
            b.clear(Action::One);
            b.bind(Action::One, Binding::pad(buttons[i]));
            const std::string text = b.serialize();
            check(text.find(std::string("One=Pad:") + (isPro ? pro[i] : neutral[i]) + "\n") != std::string::npos,
                  label + " serialized name");
            In::Bindings back = In::Bindings::defaults(layout);
            std::string error;
            check(back.parse(text, &error) && back.serialize() == text && pads(back, Action::One) == std::vector<PadButton>{buttons[i]},
                  label + " round trip: " + error);
        }
    }
#ifdef PETARI_INPUT_TEST_SDL3
    const Uint8 sdl[] = {SDL_GAMEPAD_BUTTON_MISC2, SDL_GAMEPAD_BUTTON_RIGHT_PADDLE1, SDL_GAMEPAD_BUTTON_LEFT_PADDLE1,
        SDL_GAMEPAD_BUTTON_RIGHT_PADDLE2, SDL_GAMEPAD_BUTTON_LEFT_PADDLE2};
    for (int i = 0; i < 5; ++i) {
        PadButton button = PadButton::Count;
        check(In::SDL3::padButtonOf(sdl[i], &button) && button == buttons[i], std::string("SDL mapping ") + neutral[i]);
    }
    PadButton ignored;
    check(!In::SDL3::padButtonOf(SDL_GAMEPAD_BUTTON_MISC3, &ignored), "unnamed SDL button ignored");
#endif
}

In::Detail::PointerCalibration calibration{0, 0, 1, 0.3f};
void tick(RemoteModel& model, int reports = 6) {
    for (int i = 0; i < reports; ++i) model.nextReport(calibration);
}
std::uint32_t pressAndRead(RemoteModel& model, PadButton button) {
    model.padButtonEvent(button, true);
    tick(model);
    const std::uint32_t bits = model.outputBits();
    model.padButtonEvent(button, false);
    tick(model, 400);  // past every minimum and shake spacing
    return bits;
}

void testButtonsThroughModel() {
    const std::uint32_t bitA = In::Detail::actionBits(Action::A), bitB = In::Detail::actionBits(Action::B);
    const std::uint32_t bitZ = In::Detail::actionBits(Action::NunchukZ), bitC = In::Detail::actionBits(Action::NunchukC);
    const std::uint32_t bitPlus = In::Detail::actionBits(Action::Plus), shake = In::Detail::kBitShake;
    Mods::resetForTesting();
    {
        RemoteModel model;
        model.setBindings(In::Bindings::defaults(PadLayout::ProGalaxy));
        // Menus (no play hint): Nintendo convention, A confirms (Wii A) and B goes back (Wii B).
        const std::uint32_t menuB = pressAndRead(model, kB);
        check((menuB & bitB) != 0 && (menuB & bitA) == 0, "Galaxy menus: B is Wii B (back)");
        const std::uint32_t menuA = pressAndRead(model, kA);
        check((menuA & bitA) != 0 && (menuA & bitB) == 0, "Galaxy menus: A is Wii A (confirm)");
        // In play: B jumps (Wii A).
        model.playControlShown();
        const std::uint32_t playB = pressAndRead(model, kB);
        check((playB & bitA) != 0 && (playB & bitB) == 0, "Galaxy in play: B jumps (Wii A)");
        // A press keeps its meaning while held: a menu opening mid-jump does not turn it into back.
        model.playControlShown();
        model.padButtonEvent(kB, true);
        tick(model, 60);  // the play hint lapses (100 ms) while B is held
        check(!model.playControlActive() && (model.outputBits() & bitA) != 0 && (model.outputBits() & bitB) == 0,
              "held B stays a jump after play ends (flutter / hold is not cut into back)");
        model.padButtonEvent(kB, false);
        tick(model, 400);
        const std::uint32_t after = pressAndRead(model, kB);
        check((after & bitB) != 0 && (after & bitA) == 0, "the next B press out of play is back again");
        const std::uint32_t a = pressAndRead(model, kA);
        check((a & bitA) != 0 && (a & bitB) == 0, "Galaxy: A jumps (Wii A), without B outside the title");
        model.titlePromptShown();
        model.padButtonEvent(kA, true);
        tick(model);
        check((model.outputBits() & (bitA | bitB)) == (bitA | bitB), "Galaxy: A at the title prompt is A and B (starts)");
        model.padButtonEvent(kA, false);
        tick(model, 400);
        check((pressAndRead(model, kY) & shake) != 0, "Galaxy: Y spins");
        check((pressAndRead(model, kX) & shake) != 0, "Galaxy: X spins");
        check((pressAndRead(model, kR) & shake) != 0, "Galaxy: R spins");
        check((pressAndRead(model, kZL) & bitZ) != 0, "Galaxy: ZL is Z");
        check((pressAndRead(model, kZR) & bitB) != 0, "Galaxy: ZR is B (Star Bits)");
        check((pressAndRead(model, kL) & bitC) != 0, "Galaxy: L is C (center camera)");
        check((pressAndRead(model, kPlus) & bitPlus) != 0, "Galaxy: + is Plus");
        model.padButtonEvent(kShake, true);
        model.padButtonEvent(kShake, false);  // a quick shake still counts once
        tick(model, 2);  // within the 4-report minimum pulse
        check((model.outputBits() & shake) != 0, "Galaxy: a controller shake spins");
    }
    {
        // Odyssey layout with Cappy: Y/X throw (not spin); R still spins.
        Mods::setEnabled(Mods::Mod::OdysseyMovement, true);
        Mods::setEnabled(Mods::Mod::Cappy, true);
        RemoteModel model;
        model.setBindings(In::Bindings::defaults(PadLayout::ProOdyssey));
        check((pressAndRead(model, kY) & shake) == 0, "Odyssey + Cappy: Y throws, does not spin");
        check((pressAndRead(model, kR) & shake) != 0, "Odyssey: R spins");
        const std::uint32_t menu = pressAndRead(model, kB);
        check((menu & bitB) != 0 && (menu & bitA) == 0, "Odyssey menus: B is back (Wii B)");
        model.playControlShown();
        check((pressAndRead(model, kB) & bitA) != 0, "Odyssey in play: B jumps");
        // Cappy off: the throw buttons fall back to spin, as RB does in Standard.
        Mods::setEnabled(Mods::Mod::Cappy, false);
        check((pressAndRead(model, kX) & shake) != 0, "Odyssey without Cappy: X spins");
    }
    Mods::resetForTesting();
}

void testGripButtons() {
    const auto bitA = In::Detail::actionBits(Action::A), bitB = In::Detail::actionBits(Action::B);
    const auto shake = In::Detail::kBitShake;
    for (const PadLayout layout : {PadLayout::ProGalaxy, PadLayout::ProOdyssey}) {
        Mods::resetForTesting();
        const bool odyssey = layout == PadLayout::ProOdyssey;
        Mods::setEnabled(Mods::Mod::OdysseyMovement, odyssey);
        RemoteModel model;
        model.setBindings(In::Bindings::defaults(layout));
        auto bits = pressAndRead(model, kGR);
        check((bits & bitB) && !(bits & bitA), "GR goes back in menus");
        model.playControlShown();
        model.padButtonEvent(kGR, true);
        tick(model);
        check((model.outputBits() & bitA) && !(model.outputBits() & bitB), "GR jumps in play");
        tick(model, 60);
        check((model.outputBits() & bitA) && !(model.outputBits() & bitB), "held GR stays jump after play ends");
        model.padButtonEvent(kGR, false);
        tick(model, 400);
        bits = pressAndRead(model, kGR);
        check((bits & bitB) && !(bits & bitA), "next GR press out of play goes back");
        check((pressAndRead(model, kGL) & shake) != 0, "GL spins with Cappy off");
        check(pressAndRead(model, kC) == 0, "C is unbound by default");
        if (odyssey) {
            Mods::setEnabled(Mods::Mod::Cappy, true);
            check((pressAndRead(model, kGL) & shake) == 0, "GL does not spin with Cappy active");
            In::resetForTesting();
            In::padConnected(true);
            In::padButtonEvent(kGL, true);
            check(In::actionHeld(Action::ModCappyThrow) && In::takeActionPresses(Action::ModCappyThrow) == 1,
                  "Odyssey GL produces a Cappy throw press and hold");
            In::padButtonEvent(kGL, false);
            check(!In::actionHeld(Action::ModCappyThrow), "GL throw releases");
        }
    }
    In::resetForTesting();
    Mods::resetForTesting();
}

void testExtraRemapping() {
    for (const PadLayout layout : {PadLayout::ProGalaxy, PadLayout::ProOdyssey}) {
        In::Bindings b = In::Bindings::defaults(layout);
        const Action gripAction = layout == PadLayout::ProGalaxy ? Action::Shake : Action::ModCappyThrow;
        In::Bindings visible = b;
        Binding shown;
        check(Remap::assign(visible, gripAction, 2, Binding::pad(kGL), Remap::Resolve::Check) == Remap::Result::Done &&
              Remap::slotInput(visible, gripAction, 2, &shown) && shown == Binding::pad(kGL),
              "capturing GL on its own action brings it into a visible Controls slot");
        check(Remap::clearSlot(visible, gripAction, 2) == Remap::Result::Done &&
              Remap::boundElsewhere(visible, Action::One, Binding::pad(kGL)).empty(), "visible GL can be cleared");
        // C captures through the same path as every Controls page pad binding.
        Mods::resetForTesting();
        Mods::setEnabled(Mods::Mod::OdysseyMovement, layout == PadLayout::ProOdyssey);
        In::resetForTesting();
        In::padConnected(true);
        In::beginCapture();
        In::padButtonEvent(kC, true);
        Binding captured;
        check(In::captureState(&captured) == In::CaptureState::Captured && captured == Binding::pad(kC), "Controls page captures C");
        In::padButtonEvent(kC, false);
        check(Remap::assign(b, Action::One, 2, captured, Remap::Resolve::Check) == Remap::Result::Done,
              "C can be assigned to an action");
        In::setBindings(b);
        In::padButtonEvent(kC, true);
        check(In::actionHeld(Action::One), "remapped C reaches the action");
        In::padButtonEvent(kC, false);
        check(Remap::resetAction(b, Action::One) == Remap::Result::Done && pads(b, Action::One).empty(), "reset leaves C unbound");
        for (PadButton grip : {kGR, kGL}) {
            In::Bindings original = b;
            const Action owner = grip == kGR ? Action::PlayJump : gripAction;
            check(Remap::assign(b, Action::One, 2, Binding::pad(grip), Remap::Resolve::Check) == Remap::Result::Conflict,
                  "grip assignment asks for conflict resolution");
            check(b.serialize() == original.serialize(), "Cancel/check leaves bindings unchanged");
            check(Remap::conflicts(b, Action::One, Binding::pad(grip)) == std::vector<Action>{owner}, "conflict names grip owner");
            In::Bindings both = b;
            check(Remap::assign(both, Action::One, 2, Binding::pad(grip), Remap::Resolve::KeepBoth) == Remap::Result::Done &&
                  !Remap::boundElsewhere(both, Action::One, Binding::pad(grip)).empty(), "Keep both preserves grip owner");
            b.bind(Action::One, Binding::pad(kC));
            check(Remap::assign(b, Action::One, 2, Binding::pad(grip), Remap::Resolve::Swap) == Remap::Result::Done &&
                  Remap::boundElsewhere(b, Action::One, Binding::pad(grip)).empty(), "Swap removes even a grip beyond the two visible slots");
            const auto ownerPads = pads(b, owner);
            check(std::find(ownerPads.begin(), ownerPads.end(), kC) != ownerPads.end(), "Swap gives old C binding to grip owner");
            b = original;
        }
    }
    In::resetForTesting();
    Mods::resetForTesting();
}

// The stage intro (ScenarioStarter) is not play: the game skips it on Wii A only
// (trySkipTrigger -> testSystemPadTriggerDecide -> testCorePadTriggerA). Both Pro
// layouts must give Wii A on A (so the skip works) and Wii B on B (back, no skip).
void testIntroSkipButtons() {
    const std::uint32_t bitA = In::Detail::actionBits(Action::A), bitB = In::Detail::actionBits(Action::B);
    for (const PadLayout layout : {PadLayout::ProGalaxy, PadLayout::ProOdyssey}) {
        const std::string name = In::padLayoutName(layout);
        Mods::resetForTesting();
        if (layout == PadLayout::ProOdyssey) Mods::setEnabled(Mods::Mod::OdysseyMovement, true);
        RemoteModel model;
        model.setBindings(In::Bindings::defaults(layout));
        // Play just ended (the hint lapses after 100 ms), as when an intro follows play.
        model.playControlShown();
        tick(model, 60);
        check(!model.playControlActive(), name + ": the intro is not play");
        const std::uint32_t a = pressAndRead(model, kA);
        check((a & bitA) != 0 && (a & bitB) == 0, name + " intro: Pro A is Wii A only (skips the intro)");
        const std::uint32_t b = pressAndRead(model, kB);
        check((b & bitB) != 0 && (b & bitA) == 0, name + " intro: Pro B is Wii B only (back; does not skip)");
    }
    Mods::resetForTesting();
}

void testLayoutSwitching() {
    Mods::resetForTesting();
    In::resetForTesting();
    check(In::padLayout() == PadLayout::Standard, "no pad: Standard");
    In::padConnected(true);
    check(In::padLayout() == PadLayout::ProGalaxy, "a Nintendo pad, mods off: Galaxy layout");
    check(In::bindings().layout() == PadLayout::ProGalaxy, "bindings() follow the layout");
    Mods::setEnabled(Mods::Mod::OdysseyMovement, true);
    Mods::setEnabled(Mods::Mod::Cappy, true);
    // The next controller event (or bindings call) switches; no restart.
    (void)In::takeActionPresses(Action::ModCappyThrow);
    In::padButtonEvent(kY, true);
    In::padButtonEvent(kY, false);
    check(In::takeActionPresses(Action::ModCappyThrow) == 1, "OdysseyMovement on: Y throws Cappy at once");
    check(In::padLayout() == PadLayout::ProOdyssey, "OdysseyMovement on: Odyssey layout");
    check(pads(In::bindings(), Action::Shake) == (std::vector<PadButton>{kR, kShake}), "Odyssey: Y/X leave Spin");
    Mods::setEnabled(Mods::Mod::OdysseyMovement, false);
    Mods::setEnabled(Mods::Mod::Cappy, false);
    In::padButtonEvent(kY, true);
    In::padButtonEvent(kY, false);
    check(In::padLayout() == PadLayout::ProGalaxy && In::takeActionPresses(Action::ModCappyThrow) == 0,
          "OdysseyMovement off: back to Galaxy, Y no longer throws");
    In::padConnected(false);
    check(In::padLayout() == PadLayout::Standard, "an Xbox/PlayStation pad: Standard");
    In::padButtonEvent(PadButton::Misc1, true);
    In::padButtonEvent(PadButton::Misc1, false);
    check(In::takeActionPresses(Action::PlaytestCapture) == 0, "Standard: Share does nothing by default");
    In::padConnected(true);
    In::padButtonEvent(kCapture, true);
    In::padButtonEvent(kCapture, false);
    check(In::takeActionPresses(Action::PlaytestCapture) == 1, "Pro: Capture asks for a playtest capture");
    // setBindings from the Controls page keeps the pad's layout, whatever the copy said.
    In::Bindings copy = In::Bindings::defaults(PadLayout::Standard);
    In::setBindings(copy);
    check(In::bindings().layout() == PadLayout::ProGalaxy, "setBindings keeps the pad's layout");
    // Keyboard is the same in every layout.
    check(keys(In::bindings(), Action::Shake) == std::vector<Binding>{Binding::key(In::Key::F)}, "F still spins");
    In::resetForTesting();
    Mods::resetForTesting();
}

void testRemapPerLayout() {
    In::Bindings b = In::Bindings::defaults(PadLayout::ProOdyssey);
    check(Remap::assign(b, Action::Shake, 3, Binding::pad(kL3), Remap::Resolve::KeepBoth) == Remap::Result::Done,
          "assign a controller slot in the Odyssey layout");
    check(pads(b, Action::Shake) == (std::vector<PadButton>{kR, kL3}), "Odyssey: second pad slot replaced");
    b.setLayout(PadLayout::ProGalaxy);
    check(pads(b, Action::Shake) == (std::vector<PadButton>{kY, kX, kR, kShake, kGL}), "Galaxy layout untouched");
    b.setLayout(PadLayout::Standard);
    check(pads(b, Action::Shake) == (std::vector<PadButton>{PadButton::West, PadButton::RightShoulder}), "Standard untouched");
    // A keyboard slot edited in a Pro layout is the shared keyboard binding.
    b.setLayout(PadLayout::ProOdyssey);
    check(Remap::assign(b, Action::Shake, 0, Binding::key(In::Key::G), Remap::Resolve::KeepBoth) == Remap::Result::Done,
          "assign a keyboard slot in the Odyssey layout");
    b.setLayout(PadLayout::Standard);
    check(b.inputs(Action::Shake) ==
              (std::vector<Binding>{Binding::key(In::Key::G), Binding::pad(PadButton::West), Binding::pad(PadButton::RightShoulder)}),
          "the keyboard change shows in Standard, in the defaults' order");
    const std::string text = b.serialize();
    check(text.find("\n[ProOdyssey]\n") != std::string::npos && text.find("[ProGalaxy]") == std::string::npos,
          "only the changed layout is written");
    check(text.find("\nShake=Pad:R,Pad:LStick\n") != std::string::npos, "Pro sections use Nintendo labels");
    check(text.find("\nModCappyThrow=Pad:Y,Pad:X,Pad:GL\n") != std::string::npos, "the whole section is written");
    In::Bindings back = In::Bindings::defaults();
    std::string error;
    check(back.parse(text, &error) && back.serialize() == text, "round trip: " + error);
    // Reset of one action restores that layout's defaults.
    b.setLayout(PadLayout::ProOdyssey);
    check(Remap::resetAction(b, Action::Shake) == Remap::Result::Done && pads(b, Action::Shake) == (std::vector<PadButton>{kR, kShake}),
          "reset action: the Odyssey layout's defaults");
    // Default sharing in a Pro layout is not a conflict (D-pad down: first person and photo mode).
    check(Remap::conflicts(b, Action::PhotoMode, Binding::pad(PadButton::DpadDown)).empty(), "Odyssey's shared D-pad down");
}

void testFile() {
    std::string error;
    In::Bindings b = In::Bindings::defaults();
    check(!b.parse("[ProGalaxy]\nA=Key:Space\n", &error), "a key in a Pro section is refused");
    check(!b.parse("[Bogus]\n", &error), "an unknown section is refused");
    check(!b.parse("A=Pad:ZR\n", &error), "labels only inside the Pro sections");
    In::Bindings legacy = In::Bindings::defaults();
    check(legacy.parse("Shake=Key:F\n[ProGalaxy]\nPlayJump=Pad:B\nShake=Pad:Y,Pad:X,Pad:R,Pad:Shake\n"
                       "[ProOdyssey]\nPlayJump=Pad:B\nModCappyThrow=Pad:Y,Pad:X\n", &error), "pre-Switch-2 saved sections load");
    for (const PadLayout layout : {PadLayout::ProGalaxy, PadLayout::ProOdyssey}) {
        legacy.setLayout(layout);
        check(pads(legacy, Action::PlayJump) == (std::vector<PadButton>{kB, kGR}), "old saved PlayJump gains default GR");
        check(pads(legacy, layout == PadLayout::ProGalaxy ? Action::Shake : Action::ModCappyThrow) ==
                  expectedPro(layout == PadLayout::ProOdyssey).at(layout == PadLayout::ProGalaxy ? Action::Shake : Action::ModCappyThrow),
              "old saved action gains default GL");
        legacy.unbind(Action::PlayJump, Binding::pad(kGR));
        legacy.unbind(layout == PadLayout::ProGalaxy ? Action::Shake : Action::ModCappyThrow, Binding::pad(kGL));
    }
    const std::string noGrips = legacy.serialize();
    In::Bindings noGripsBack = In::Bindings::defaults();
    check(noGripsBack.parse(noGrips, &error) && noGripsBack.serialize() == noGrips,
          "intentional removal of every grip survives save/load");
    check(noGrips.find("Pad:GR") == std::string::npos && noGrips.find("Pad:GL") == std::string::npos,
          "explicit empty extras do not need a sentinel binding");
    check(b.parse("[ProGalaxy]\nA=Pad:South,Pad:A\n", &error) && (b.setLayout(PadLayout::ProGalaxy), true) &&
              pads(b, Action::A) == (std::vector<PadButton>{kB, kA}),
          "Pro sections also take position names: " + error);
    In::Bindings o = In::Bindings::defaults();
    check(o.parse("[Controller]\nGyro=on\nGyroSpeed=1.5\nGyroInvertY=on\nRumble=off\nShakeG=2\n", &error), "options: " + error);
    check(o.options().gyro == In::PadOptions::Gyro::On && o.options().gyroSpeed == 1.5f && o.options().gyroInvertY &&
              !o.options().rumble && o.options().shakeG == 2.0f,
          "options parsed");
    In::Bindings o2 = In::Bindings::defaults();
    check(o2.parse(o.serialize(), &error) && o2.options() == o.options(), "options round trip");
    check(!o2.parse("[Controller]\nGyroSpeed=99\n", &error), "an out-of-range option is refused");
    check(!o2.parse("[Controller]\nGyro=sometimes\n", &error), "an unknown option value is refused");

    // controls.txt through Remap::load/save.
    char dirTemplate[] = "/tmp/petari-pro-XXXXXX";
    const char* dir = mkdtemp(dirTemplate);
    check(dir != nullptr, "temp dir");
    if (dir == nullptr) return;
    const std::filesystem::path file = std::filesystem::path(dir) / "controls.txt";
    {
        std::ofstream out(file);
        out << "Shake=Key:F\n\n[ProOdyssey]\nModCappyThrow=Pad:ZR\nB=Pad:Y\n\n[Controller]\nGyro=off\n";
    }
    In::Bindings loaded;
    std::string message;
    check(Remap::load(file, &loaded, &message), "load a Pro file: " + message);
    loaded.setLayout(PadLayout::ProOdyssey);
    check(pads(loaded, Action::ModCappyThrow) == (std::vector<PadButton>{kZR, kGL}) && pads(loaded, Action::B) == std::vector<PadButton>{kY},
          "the file's Odyssey remaps");
    check(pads(loaded, Action::PlayJump) == (std::vector<PadButton>{kB, kGR}), "actions the section omits keep their defaults");
    check(loaded.options().gyro == In::PadOptions::Gyro::Off, "the file's options");
    check(Remap::save(loaded, &message), "save: " + message);
    In::Bindings again;
    check(Remap::load(file, &again, &message) && again.serialize() == loaded.serialize(), "save/load round trip");
    std::filesystem::remove_all(dir);
}

void testShakeDetector() {
    constexpr float g = 9.80665f;
    In::Detail::PadShakeDetector d;
    double t = 1.0;
    int presses = 0;
    bool was = false;
    auto feed = [&](float x, float y, float z, double seconds) {
        const float a[3] = {x, y, z};
        const double end = t + seconds;
        for (; t < end; t += 0.005) {
            const bool held = d.update(a, t, 1.6f);
            if (held && !was) ++presses;
            was = held;
        }
    };
    feed(0, g, 0, 1.0);
    check(presses == 0, "a controller at rest does not shake");
    // A slow turn onto its side over a second (orientation, not a shake).
    for (int i = 0; i <= 200; ++i) {
        const float angle = 1.5707963f * static_cast<float>(i) / 200.0f;
        feed(g * std::sin(angle), g * std::cos(angle), 0, 0.005);
    }
    feed(g, 0, 0, 0.5);
    check(presses == 0, "turning the controller over a second is not a shake");
    feed(0, g, 0, 1.0);
    presses = 0;
    feed(0, g, 0, 0.2);
    feed(2.5f * g, g, 0, 0.04);  // a flick
    feed(0, g, 0, 0.3);
    check(presses == 1 && !was, "one flick: one press, released");
    presses = 0;
    feed(3.0f * g, g, 0, 0.03);
    feed(-3.0f * g, g, 0, 0.03);
    feed(0, g, 0, 0.3);
    check(presses == 1, "a back-and-forth flick is one press");
    presses = 0;
    for (int i = 0; i < 6; ++i) {  // shaking for 0.6 s
        feed(3.0f * g, g, 0, 0.05);
        feed(-3.0f * g, g, 0, 0.05);
    }
    check(presses >= 1 && presses <= 3, "continuous shaking: a few presses, not one per sample (" + std::to_string(presses) + ")");
    feed(0, g, 0, 0.5);
    check(!was, "released once the controller settles");
}

void testGyro() {
    In::PadOptions options;
    double last = 0.0;
    float dx = 0, dy = 0;
    const float yawLeft[3] = {0.0f, 1.0f, 0.0f};
    check(!In::Detail::gyroPointerDelta(yawLeft, 1.0, &last, options, &dx, &dy), "first gyro sample moves nothing");
    float sumX = 0, sumY = 0;
    for (int i = 1; i <= 20; ++i) {
        if (In::Detail::gyroPointerDelta(yawLeft, 1.0 + 0.005 * i, &last, options, &dx, &dy)) {
            sumX += dx;
            sumY += dy;
        }
    }
    check(std::fabs(sumX + 0.1f * In::Detail::kGyroUnitsPerRadian) < 1e-3f && sumY == 0.0f, "yaw left 0.1 rad: pointer left");
    const float pitchUp[3] = {1.0f, 0.0f, 0.0f};
    In::Detail::gyroPointerDelta(pitchUp, 1.105, &last, options, &dx, &dy);
    check(dy < 0.0f && dx == 0.0f, "pitch up: pointer up (KPAD y down)");
    options.gyroInvertY = true;
    In::Detail::gyroPointerDelta(pitchUp, 1.110, &last, options, &dx, &dy);
    check(dy > 0.0f, "invert Y");
    const float rest[3] = {0.01f, -0.015f, 0.0f};
    check(!In::Detail::gyroPointerDelta(rest, 1.115, &last, options, &dx, &dy), "noise at rest moves nothing");

    // Through the host: the gyro is the pointer in the Odyssey layout (Auto).
    Mods::resetForTesting();
    In::resetForTesting();
    In::padConnected(true);
    In::setClock(In::Clock::Manual);
    KPADInit();
    auto turn = [](float yaw) {
        static double t = 10.0;
        const float data[3] = {0.0f, yaw, 0.0f};
        for (int i = 0; i < 20; ++i) {
            In::padSensorEvent(In::PadSensor::Gyro, data, t += 0.005);
            In::pumpReports(1);
        }
    };
    turn(-1.0f);  // yaw right
    check(!In::boundState().pointerValid || In::boundState().pointerX == 0.0f, "Galaxy layout (Gyro=auto): gyro idle");
    Mods::setEnabled(Mods::Mod::OdysseyMovement, true);
    turn(-1.0f);
    turn(-1.0f);
    const float x = In::boundState().pointerX;
    check(x > 0.1f, "Odyssey layout: yaw right moves the pointer right (" + std::to_string(x) + ")");
    Mods::setEnabled(Mods::Mod::OdysseyMovement, false);
    In::Bindings on = In::bindings();
    on.options().gyro = In::PadOptions::Gyro::On;
    In::setBindings(on);
    turn(1.0f);
    check(In::boundState().pointerX < x, "Gyro=on: the Galaxy layout's pointer too");
    on.options().gyro = In::PadOptions::Gyro::Off;
    on.options().rumble = false;
    In::setBindings(on);
    const float before = In::boundState().pointerX;
    turn(1.0f);
    check(In::boundState().pointerX == before, "Gyro=off");
    check(!In::padRumble(), "Rumble=off: no controller rumble");
    // A shake through the host is Pad:Shake, held while shaking.
    const float still[3] = {0.0f, 9.8f, 0.0f}, flick[3] = {30.0f, 9.8f, 0.0f};
    double t = 50.0;
    for (int i = 0; i < 40; ++i) { In::padSensorEvent(In::PadSensor::Accel, still, t += 0.005); In::pumpReports(1); }
    for (int i = 0; i < 4; ++i) { In::padSensorEvent(In::PadSensor::Accel, flick, t += 0.005); In::pumpReports(1); }
    check(In::actionHeld(Action::Shake), "Pro: a shake holds Spin");
    for (int i = 0; i < 60; ++i) { In::padSensorEvent(In::PadSensor::Accel, still, t += 0.005); In::pumpReports(1); }
    check(!In::actionHeld(Action::Shake), "and lets go");
    In::resetForTesting();
    Mods::resetForTesting();
}
}  // namespace

int main() {
    __OSThreadInit();
    Mods::resetForTesting();
    PetariNative::PointMove::resetForTesting();
    testTables();
    testExtraButtonNames();
    testButtonsThroughModel();
    testGripButtons();
    testExtraRemapping();
    testIntroSkipButtons();
    testLayoutSwitching();
    testRemapPerLayout();
    testFile();
    testShakeDetector();
    testGyro();
    std::printf("Pro Controller: %d checks, %d failed\n", checks, failures);
    return failures == 0 ? 0 : 1;
}
