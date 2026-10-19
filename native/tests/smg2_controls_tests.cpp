// SMG2 additions: preserve the imported defaults and exercise the actual
// report model and controls.txt editor/storage, without a running game.
#include "petari/input.hpp"
#include "petari/input_remap.hpp"
#include "petari/mods.hpp"
#include "petari/point_move.hpp"
#include "../input/remote_model.hpp"

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <string>
#include <unistd.h>

namespace In = PetariNative::Input;
namespace Remap = In::Remap;
using In::Action;
using In::Binding;
using In::Detail::RemoteModel;
using In::Detail::Report;
namespace {
int checks = 0;
void check(bool condition, const char* description) {
    ++checks;
    if (!condition) {
        std::fprintf(stderr, "FAIL: %s\n", description);
        std::exit(1);
    }
}

// Captured from the imported Bindings::defaults(), before this change.
constexpr const char* kImportedDefaults = R"controls(
StickUp=Key:W
StickDown=Key:S
StickLeft=Key:A
StickRight=Key:D
A=Key:Space,Mouse:Right,Pad:South
B=Mouse:Left,Key:B,Key:Backspace,Pad:East,Pad:RightTrigger
Plus=Key:Escape,Pad:Start
Minus=Key:Minus,Pad:Back
Home=Key:F1,Key:Grave,Pad:Guide
One=Key:Num1
Two=Key:Num2
DpadUp=Key:Up,Pad:DpadUp
DpadDown=Key:Down,Pad:DpadDown
DpadLeft=Key:Q,Key:Left,Pad:DpadLeft
DpadRight=Key:E,Key:Right,Pad:DpadRight
NunchukC=Key:C,Pad:LeftShoulder
NunchukZ=Key:LeftShift,Key:RightShift,Pad:LeftTrigger
Shake=Key:F,Pad:West,Pad:RightShoulder
TiltHold=Key:Tab
PostureToggle=Key:T
Walk=Key:LeftAlt
Start=Key:Return,Key:KeypadEnter,Pad:North
ModCollectStarBits=Key:G,Pad:LeftStick
ModShootEnemy=Key:V
CameraOrbitHold=Mouse:Middle,Key:LeftGui
CameraZoomIn=Key:Z
CameraZoomOut=Key:X
CameraOrbitLeft=Key:J
CameraOrbitRight=Key:L
CameraPitchUp=Key:I
CameraPitchDown=Key:K
PhotoMode=Key:P,Pad:RightStick
PhotoShot=Key:O
ModCappyThrow=Key:R,Pad:RightShoulder
LevelEditor=Key:Tab
)controls";

In::Detail::PointerCalibration calibration{0, 0, 1, 0.3f};
Report tick(RemoteModel& model, int reports = 1) {
    Report out;
    for (int i = 0; i < reports; ++i) out = model.nextReport(calibration);
    return out;
}
void key(RemoteModel& model, In::KeyCode code, bool down = true) {
    model.keyEvent(code, down, false);
}

void testDefaultsAndStorage() {
    const auto defaults = In::Bindings::defaults();
    const auto summary = In::controlsSummary(defaults);
    check(summary.size() <= 20, "controls summary still fits its existing page");
    bool nunchukNamed = false;
    for (const auto& line : summary) {
        check(line.action.size() < 40 && line.inputs.size() < 64 && line.pad.size() < 40, "summary text fits existing fields");
        nunchukNamed |= line.action == "Shake Nunchuk" && line.inputs == "H";
    }
    check(nunchukNamed, "summary names the new Nunchuk input");
    const std::string old = kImportedDefaults + 1;  // raw string's initial newline
    check(defaults.serialize().compare(0, old.size(), old) == 0, "every imported key/mouse/pad binding is unchanged");
    const struct { Action action; const char* name; In::KeyCode key; } added[] = {
        {Action::NunchukShake, "NunchukShake", In::Key::H},
        {Action::CoStarToggle, "CoStarToggle", In::Key::F2},
        {Action::CoStarA, "CoStarA", In::Key::U},
        {Action::CoStarB, "CoStarB", In::Key::Y},
        {Action::CoStarShake, "CoStarShake", In::Key::N},
    };
    for (const auto& a : added) {
        check(std::strcmp(In::actionName(a.action), a.name) == 0, "new serialized action name");
        check(defaults.inputs(a.action) == std::vector<Binding>{Binding::key(a.key)}, "new key has no unexpected extra bindings");
        check(Remap::conflicts(defaults, a.action, Binding::key(a.key)).empty(), "new default does not steal an existing key");
    }
    In::Bindings parsed = defaults;
    std::string error;
    check(parsed.parse(old, &error) && parsed.serialize() == defaults.serialize(), "old controls.txt gets new defaults without changing old actions");
    check(Remap::assign(parsed, Action::CoStarA, 0, Binding::key(In::Key::F), Remap::Resolve::Check) == Remap::Result::Conflict,
          "new action detects conflict with old spin key");
    check(Remap::assign(parsed, Action::NunchukShake, 0, Binding::key(In::Key::M), Remap::Resolve::Check) == Remap::Result::Done,
          "Nunchuk shake remaps using the existing editor");
    check(Remap::assign(parsed, Action::CoStarA, 0, Binding::mouse(In::MouseButton::X1), Remap::Resolve::Check) == Remap::Result::Done,
          "co-star remaps to a mouse button");
    check(Remap::assign(parsed, Action::CoStarShake, 2, Binding::pad(In::PadButton::LeftStick), Remap::Resolve::KeepBoth) == Remap::Result::Done,
          "co-star remaps to a controller button");
    In::Bindings roundtrip = defaults;
    check(roundtrip.parse(parsed.serialize(), &error) && roundtrip.serialize() == parsed.serialize(), "new remaps serialize and parse exactly");
    const auto before = parsed.serialize();
    check(!parsed.parse("CoStarA=Key:NoSuchKey\n", &error) && parsed.serialize() == before, "invalid new binding leaves all mappings intact");

    char directory[] = "/tmp/petari-smg2-controls-XXXXXX";
    check(mkdtemp(directory) != nullptr, "create private controls.txt test directory");
    const auto file = std::filesystem::path(directory) / "controls.txt";
    In::Bindings loaded;
    check(Remap::load(file, &loaded, &error) && loaded.serialize() == defaults.serialize(), "missing file retains defaults");
    check(Remap::save(parsed, &error), "new actions save through the existing controls.txt path");
    check(Remap::load(file, &loaded, &error) && loaded.serialize() == parsed.serialize(), "new actions survive controls.txt reload");
    check(Remap::resetAction(loaded, Action::NunchukShake) == Remap::Result::Done &&
              loaded.inputs(Action::NunchukShake) == defaults.inputs(Action::NunchukShake), "reset restores the new default");
    std::filesystem::remove_all(directory);  // only this test's mkdtemp directory
}

void testNunchukFlick() {
    RemoteModel model;
    key(model, In::Key::H);
    const Report first = tick(model);
    check(first.nunchukAcc[0] > 2 && first.nunchukAcc[2] > 2, "H drives Nunchuk acceleration");
    check(model.boundState().spin, "logical input recording includes the Nunchuk spin key");
    check(first.acc[0] == 0 && first.acc[1] == -1 && first.acc[2] == 0 && first.buttons == 0, "H leaves Remote acceleration and all buttons alone");
    tick(model, 100);
    const Report held = tick(model);
    check(held.nunchukAcc[0] == 0 && held.nunchukAcc[2] == 0, "holding H does not repeat");
    key(model, In::Key::F);
    const Report remote = tick(model);
    check(remote.acc[0] == 3 && remote.nunchukAcc[0] == 0, "F retains its original Remote-only flick");

    RemoteModel both;
    key(both, In::Key::H);
    key(both, In::Key::F);
    const auto together = tick(both);
    check(together.acc[0] == 3 && together.nunchukAcc[0] > 2, "independent flicks can start together");

    RemoteModel delayed;
    key(delayed, In::Key::LeftShift);
    tick(delayed);
    key(delayed, In::Key::H);
    check(tick(delayed).nunchukAcc[0] == 0, "Z delays Nunchuk shake for Mario's left-swing lockout");
    key(delayed, In::Key::H, false);
    bool arrived = false;
    for (int i = 0; i < 40; ++i) arrived |= tick(delayed).nunchukAcc[0] > 2;
    check(arrived, "a released tap still plays its pending Nunchuk flick");

    RemoteModel lost;
    key(lost, In::Key::C);
    tick(lost);
    key(lost, In::Key::H);
    tick(lost);
    lost.focusChanged(false);
    lost.focusChanged(true);
    bool leaked = false;
    for (int i = 0; i < 80; ++i) leaked |= tick(lost).nunchukAcc[0] != 0;
    check(!leaked, "focus loss cancels a waiting Nunchuk flick");

    auto remapped = In::Bindings::defaults();
    std::string error;
    check(remapped.parse("NunchukShake=Key:M\n", &error), "parse Nunchuk remap");
    RemoteModel changed;
    changed.setBindings(remapped);
    key(changed, In::Key::H);
    check(tick(changed).nunchukAcc[0] == 0, "old Nunchuk key stops working after remap");
    key(changed, In::Key::M);
    check(tick(changed).nunchukAcc[0] > 2, "remapped Nunchuk key drives the same gesture");
}

void testCoStarIsolation() {
    RemoteModel player;
    RemoteModel coStar(true);
    for (auto* model : {&player, &coStar}) {
        model->setViewport(In::Viewport::letterbox(1280, 720, 16.0f / 9.0f));
        model->mouseMoved(640, 360);
        key(*model, In::Key::U);
    }
    auto p1 = tick(player);
    auto p2 = tick(coStar);
    check(p1.buttons == 0 && p2.buttons == 0x0800, "U is A on co-star only");
    check(p1.dotCount == 2 && p2.dotCount == 2 && p1.dots[0].x == p2.dots[0].x && p1.dots[0].y == p2.dots[0].y,
          "both remotes aim at the shared mouse position");
    for (auto* model : {&player, &coStar}) key(*model, In::Key::Y);
    check(tick(player).buttons == 0 && tick(coStar).buttons == 0x0C00, "Y adds B on co-star only");
    for (auto* model : {&player, &coStar}) key(*model, In::Key::Space);
    check(tick(player).buttons == 0x0800 && tick(coStar).buttons == 0x0C00, "Space remains player 1 A, not another co-star input");
    for (auto* model : {&player, &coStar}) key(*model, In::Key::N);
    check(tick(player).acc[0] == 0, "N cannot shake player 1");
    bool shook = false;
    for (int i = 0; i < 50; ++i) shook |= tick(coStar).acc[0] == 3;
    check(shook, "N shakes the co-star after its A/B delay");

    RemoteModel toggle;
    key(toggle, In::Key::F2);
    tick(toggle);
    check(toggle.takeCoStarToggle() && !toggle.takeCoStarToggle(), "F2 produces exactly one toggle request");
    tick(toggle, 100);
    check(!toggle.takeCoStarToggle(), "holding F2 cannot repeatedly connect/disconnect");
    key(toggle, In::Key::F2, false);
    tick(toggle, 5);
    key(toggle, In::Key::F2);
    tick(toggle);
    check(toggle.takeCoStarToggle(), "a fresh F2 press toggles again");
    coStar.focusChanged(false);
    tick(coStar, 100);
    check(tick(coStar).buttons == 0 && tick(coStar).dotCount == 0, "focus loss releases co-star buttons and hides its pointer");

    auto remapped = In::Bindings::defaults();
    std::string error;
    check(remapped.parse("CoStarA=Mouse:X1\nCoStarB=Key:M\nCoStarShake=Pad:LeftStick\n", &error), "parse independent co-star remaps");
    RemoteModel changed(true);
    changed.setBindings(remapped);
    key(changed, In::Key::U);
    check(tick(changed).buttons == 0, "old co-star A key is removed");
    changed.mouseButtonEvent(In::MouseButton::X1, true);
    key(changed, In::Key::M);
    check(tick(changed).buttons == 0x0C00, "new mouse/key co-star remaps reach A/B");
    RemoteModel triggers(true);
    check(remapped.parse("CoStarA=Pad:LeftTrigger\n", &error), "parse co-star trigger remap");
    triggers.setBindings(remapped);
    triggers.padAxisEvent(In::PadAxis::LeftTrigger, 0.6f);
    check(tick(triggers).buttons == 0x0800, "analog trigger can press a remapped co-star button");
    triggers.padAxisEvent(In::PadAxis::LeftTrigger, 0.5f);
    check(tick(triggers, 5).buttons == 0x0800, "co-star trigger retains existing hysteresis");
    triggers.padAxisEvent(In::PadAxis::LeftTrigger, 0.4f);
    check(tick(triggers, 5).buttons == 0, "co-star trigger releases below existing threshold");
    triggers.padAxisEvent(In::PadAxis::LeftX, 1.0f);
    triggers.padAxisEvent(In::PadAxis::RightX, 1.0f);
    const auto noStick = tick(triggers);
    check(noStick.stickX == 0 && noStick.stickY == 0, "co-star has no gamepad movement stick");
}
}  // namespace
int main() {
    PetariNative::Mods::resetForTesting();
    PetariNative::PointMove::resetForTesting();
    testDefaultsAndStorage();
    testNunchukFlick();
    testCoStarIsolation();
    std::printf("SMG2 controls: %d checks passed\n", checks);
}
