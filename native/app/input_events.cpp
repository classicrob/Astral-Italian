// Window events and remapped controls for the native input layer.

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_gamepad.h>
#include <SDL3/SDL_scancode.h>

#include <cstdio>
#include <fstream>
#include <sstream>

#include "host.hpp"
#include "playtest_capture.hpp"
#include "smoke_background.hpp"
#include "petari/input.hpp"
#include "petari/input_remap.hpp"
#include "petari/input_sdl3.hpp"

namespace PetariNative::App::Events {

bool loadControls(const std::filesystem::path& file, std::string* error) {
    // A missing file is the defaults; an invalid one is the defaults too, with a
    // message (the Home menu's Controls page shows it until the player saves).
    Input::Bindings bindings;
    const bool ok = Input::Remap::load(file, &bindings, error);
    Input::setBindings(bindings);
    return ok;
}

namespace {

Smoke::PhysicalInputs gPhysical;

// The controller whose labels choose the layout, sensors feed the pointer and
// shake, and rumble follows the game's Wii Remote motor (native/CONTROLS.md
// "Switch Pro Controller"). Aurora opens every controller; all act as one.
SDL_JoystickID gPadId = 0;
bool gRumbling = false;

void usePad(SDL_JoystickID id) {
    SDL_Gamepad* pad = SDL_GetGamepadFromID(id);
    if (pad == nullptr) return;
    gPadId = id;
    const bool nintendo = SDL_GetGamepadButtonLabel(pad, SDL_GAMEPAD_BUTTON_SOUTH) == SDL_GAMEPAD_BUTTON_LABEL_B;
    bool gyro = false;
    bool accel = false;
    if (nintendo) {
        // Only the Nintendo layouts use them: no sensor reports from other pads.
        gyro = SDL_GamepadHasSensor(pad, SDL_SENSOR_GYRO) && SDL_SetGamepadSensorEnabled(pad, SDL_SENSOR_GYRO, true);
        accel = SDL_GamepadHasSensor(pad, SDL_SENSOR_ACCEL) && SDL_SetGamepadSensorEnabled(pad, SDL_SENSOR_ACCEL, true);
    }
    const bool rumble = SDL_GetBooleanProperty(SDL_GetGamepadProperties(pad), SDL_PROP_GAMEPAD_CAP_RUMBLE_BOOLEAN, false);
    Input::padConnected(nintendo);
    const char* name = SDL_GetGamepadName(pad);
    std::fprintf(stderr, "[pad] using '%s' (type %d): %s layout; gyro %s, accelerometer %s, rumble %s\n",
                 name != nullptr ? name : "?", static_cast<int>(SDL_GetGamepadType(pad)),
                 Input::padLayoutName(Input::padLayout()), gyro ? "on" : "no", accel ? "on" : "no", rumble ? "yes" : "no");
}

void padRemoved(SDL_JoystickID id) {
    if (id != gPadId) return;
    gPadId = 0;
    gRumbling = false;
    int count = 0;
    SDL_JoystickID* pads = SDL_GetGamepads(&count);
    for (int i = 0; pads != nullptr && i < count; ++i) {
        if (pads[i] != id && SDL_GetGamepadFromID(pads[i]) != nullptr) {
            usePad(pads[i]);
            break;
        }
    }
    SDL_free(pads);
    if (gPadId == 0) Input::padConnected(false);
}

// The first game action the input is bound to, or null.
const char* boundAction(const Input::Binding& input) {
    const Input::Bindings bindings = Input::bindings();
    for (int a = 0; a < static_cast<int>(Input::Action::Count); ++a) {
        const Input::Action action = static_cast<Input::Action>(a);
        for (const Input::Binding& bound : bindings.inputs(action)) {
            if (bound == input) {
                return Input::actionName(action);
            }
        }
    }
    return nullptr;
}

void countGameplay(const char* device, const char* name, const char* action, bool down) {
    ++gPhysical.gameplay;
    std::snprintf(gPhysical.last, sizeof(gPhysical.last), "%s %s (%s) %s", device, name, action,
                  down ? "down" : "up");
}

void countPhysical(const SDL_Event& event) {
    switch (event.type) {
    case SDL_EVENT_KEY_DOWN:
    case SDL_EVENT_KEY_UP: {
        if (event.key.repeat) {
            return;
        }
        const char* action = boundAction(Input::Binding::key(static_cast<Input::KeyCode>(event.key.scancode)));
        if (action != nullptr) {
            const char* name = SDL_GetScancodeName(event.key.scancode);
            countGameplay("key", name != nullptr && name[0] != '\0' ? name : "?", action, event.key.down);
        }
        return;
    }
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
    case SDL_EVENT_MOUSE_BUTTON_UP: {
        // The buttons Input::SDL3::handleEvent maps.
        static const struct {
            Uint8 sdl;
            Input::MouseButton button;
            const char* name;
        } kButtons[] = {{SDL_BUTTON_LEFT, Input::MouseButton::Left, "Left"},
                        {SDL_BUTTON_MIDDLE, Input::MouseButton::Middle, "Middle"},
                        {SDL_BUTTON_RIGHT, Input::MouseButton::Right, "Right"},
                        {SDL_BUTTON_X1, Input::MouseButton::X1, "X1"},
                        {SDL_BUTTON_X2, Input::MouseButton::X2, "X2"}};
        for (const auto& b : kButtons) {
            if (b.sdl == event.button.button) {
                const char* action = boundAction(Input::Binding::mouse(b.button));
                if (action != nullptr) {
                    countGameplay("mouse", b.name, action, event.button.down);
                }
                return;
            }
        }
        return;
    }
    case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
    case SDL_EVENT_GAMEPAD_BUTTON_UP: {
        Input::PadButton button;
        if (Input::SDL3::padButtonOf(event.gbutton.button, &button)) {
            const char* action = boundAction(Input::Binding::pad(button));
            if (action != nullptr) {
                countGameplay("pad", Input::displayName(Input::Binding::pad(button)).c_str(), action, event.gbutton.down);
            }
        }
        return;
    }
    case SDL_EVENT_GAMEPAD_AXIS_MOTION: {
        // A deliberate push (past half travel) is gameplay input; a resting
        // stick's drift is not. The right stick moves the pointer.
        Input::PadAxis axis;
        if (Input::SDL3::padAxisOf(event.gaxis.axis, &axis) && (event.gaxis.value > 16384 || event.gaxis.value < -16384)) {
            if (axis == Input::PadAxis::RightX || axis == Input::PadAxis::RightY) {
                ++gPhysical.pointer;
            } else {
                countGameplay("pad", "stick or trigger", "axis", true);
            }
        }
        return;
    }
    case SDL_EVENT_MOUSE_MOTION:
        ++gPhysical.pointer;
        return;
    case SDL_EVENT_WINDOW_FOCUS_GAINED:
    case SDL_EVENT_WINDOW_FOCUS_LOST:
        ++gPhysical.focus;
        return;
    default:
        return;
    }
}

}  // namespace

bool input(const SDL_Event& event) {
    if (SmokeBackground::enabled && (SmokeBackground::physicalEvent(event.type) ||
        event.type == SDL_EVENT_WINDOW_FOCUS_LOST || event.type == SDL_EVENT_WINDOW_FOCUS_GAINED)) {
        return true;
    }
    // Foreground run ignoring physical input: focus events still reach the input layer and are counted.
    if (SmokeBackground::ignorePhysical && SmokeBackground::physicalEvent(event.type)) {
        return true;
    }
    countPhysical(event);
    // F9 or M ("mark this moment", for keyboards without usable function keys): playtest capture
    // (playtest_capture.hpp; inert unless PETARI_CAPTURE_DIR is set). Both keys also reach the input layer, where they
    // are unbound by default.
    if (event.type == SDL_EVENT_KEY_DOWN && !event.key.repeat &&
        (event.key.scancode == SDL_SCANCODE_F9 || event.key.scancode == SDL_SCANCODE_M)) {
        PlaytestCapture::request();
    }
    if (event.type == SDL_EVENT_GAMEPAD_ADDED) usePad(event.gdevice.which);
    if (event.type == SDL_EVENT_GAMEPAD_REMOVED) padRemoved(event.gdevice.which);
    return Input::SDL3::handleEvent(event);
}

void frame() {
    // The PlaytestCapture action (the Pro Controller's Capture by default; F9/M above).
    if (Input::takeActionPresses(Input::Action::PlaytestCapture) > 0) PlaytestCapture::request();
    // Rumble: renewed each frame while the game's motor runs, stopped when it stops.
    SDL_Gamepad* pad = gPadId != 0 ? SDL_GetGamepadFromID(gPadId) : nullptr;
    if (pad == nullptr) return;
    const bool want = Input::padRumble();
    if (want) {
        SDL_RumbleGamepad(pad, 0x5000, 0xB000, 100);
    } else if (gRumbling) {
        SDL_RumbleGamepad(pad, 0, 0, 0);
    }
    gRumbling = want;
}

Smoke::PhysicalInputs physicalInputs() {
    return gPhysical;
}

void setImage(const Rect& image, float windowWidth, float windowHeight) {
    Input::Viewport viewport;
    viewport.windowWidth = windowWidth;
    viewport.windowHeight = windowHeight;
    viewport.imageX = image.x;
    viewport.imageY = image.y;
    viewport.imageWidth = image.width;
    viewport.imageHeight = image.height;
    Input::setViewport(viewport);
}

void pressButton(int button, bool down) {
    static const Input::Action kActions[] = {Input::Action::A,         Input::Action::B,     Input::Action::StickUp,
                                             Input::Action::StickDown, Input::Action::Plus,  Input::Action::Minus,
                                             Input::Action::StickLeft, Input::Action::StickRight,
                                             Input::Action::Shake,     Input::Action::DpadLeft, Input::Action::DpadRight,
                                             Input::Action::NunchukZ,  Input::Action::ModCappyThrow,
                                             Input::Action::Home};
    if (button < 0 || button >= static_cast<int>(sizeof(kActions) / sizeof(kActions[0]))) {
        return;
    }
    const Input::Action action = kActions[button];
    const Input::Bindings bindings = Input::bindings();
    const auto& inputs = bindings.inputs(action);
    if (inputs.empty()) {
        std::fprintf(stderr, "PETARI SMOKE: no input is bound to %s\n", Input::actionName(action));
        return;
    }
    const Input::Binding& input = inputs.front();
    if (input.device == Input::Binding::Device::Key) {
        Input::keyEvent(input.code, down, false);
    } else {
        Input::mouseButtonEvent(static_cast<Input::MouseButton>(input.code), down);
    }
}

void assertFocus() {
    Input::focusChanged(true);
}

void movePointer(float x, float y) {
    Input::mouseMoved(x, y);
}

void menuWheel(float y) {
    Input::menuWheel(y);
}

}  // namespace PetariNative::App::Events
