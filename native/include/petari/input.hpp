#pragma once
// Native keyboard and mouse input behind the SDK's WPAD/KPAD API.
//
// Game code keeps calling <revolution/wpad.h> and <revolution/kpad.h>. The
// input library emulates WPAD for a virtual Wii Remote with a Nunchuk on
// channel 0, and compiles the SDK's own KPAD.c on top of it, so KPAD's pointer,
// accelerometer, and button-repeat processing are the original code.
//
// The virtual remote produces WPAD reports at 200 Hz, the Wii rate:
// - buttons and the Nunchuk stick from remappable bindings;
// - two sensor-bar dots in the IR camera image, placed so that KPAD reports
//   the Star Pointer where the mouse is over the game image;
// - accelerometer data from a remote orientation (tilt) and motion bursts
//   (a shake, which the game reads as a spin).
//
// Host events come in through the functions below, from any host thread. They
// identify keys by USB HID keyboard usage, which equals SDL3's SDL_Scancode.
// See input_sdl3.hpp in native/input for an SDL3 event adapter.

#include <cstdint>
#include <string>
#include <vector>

namespace PetariNative::Input {

// USB HID keyboard page (0x07) usages. Identical to SDL_Scancode values.
using KeyCode = std::uint16_t;
namespace Key {
constexpr KeyCode A = 4, B = 5, C = 6, D = 7, E = 8, F = 9, G = 10, H = 11, I = 12, J = 13, K = 14, L = 15, M = 16, N = 17,
                  O = 18, P = 19, Q = 20, R = 21, S = 22, T = 23, U = 24, V = 25, W = 26, X = 27, Y = 28, Z = 29;
constexpr KeyCode Num1 = 30, Num2 = 31, Num3 = 32, Num4 = 33, Num5 = 34, Num6 = 35, Num7 = 36, Num8 = 37, Num9 = 38, Num0 = 39;
constexpr KeyCode Return = 40, Escape = 41, Backspace = 42, Tab = 43, Space = 44, Minus = 45, Equals = 46, LeftBracket = 47,
                  RightBracket = 48, Backslash = 49, Semicolon = 51, Apostrophe = 52, Grave = 53, Comma = 54, Period = 55, Slash = 56,
                  CapsLock = 57;
constexpr KeyCode F1 = 58, F2 = 59, F3 = 60, F4 = 61, F5 = 62, F6 = 63, F7 = 64, F8 = 65, F9 = 66, F10 = 67, F11 = 68, F12 = 69;
constexpr KeyCode Right = 79, Left = 80, Down = 81, Up = 82;
constexpr KeyCode KeypadEnter = 88;
constexpr KeyCode LeftCtrl = 224, LeftShift = 225, LeftAlt = 226, LeftGui = 227, RightCtrl = 228, RightShift = 229, RightAlt = 230,
                  RightGui = 231;
constexpr KeyCode Max = 512;  // exclusive bound accepted by the API (SDL_SCANCODE_COUNT)
}  // namespace Key

enum class MouseButton : std::uint8_t { Left, Middle, Right, X1, X2, Count };

// Game controller buttons, by position (SDL3's standard gamepad layout:
// South is Xbox A, PlayStation Cross, Nintendo B). The triggers are analog
// axes that act as buttons past half travel.
enum class PadButton : std::uint8_t {
    South,
    East,
    West,
    North,
    Back,
    Guide,
    Start,
    LeftStick,   // L3
    RightStick,  // R3: also recenters the controller's Star Pointer
    LeftShoulder,
    RightShoulder,
    DpadUp,
    DpadDown,
    DpadLeft,
    DpadRight,
    LeftTrigger,
    RightTrigger,
    // Appended (2026-10-21, Pro Controller layouts); earlier names keep their codes.
    Misc1,  // Switch Pro Capture, Xbox Share (SDL_GAMEPAD_BUTTON_MISC1)
    Shake,  // a shake of a controller with an accelerometer (padSensorEvent), as a button
    Count
};
enum class PadAxis : std::uint8_t { LeftX, LeftY, RightX, RightY, LeftTrigger, RightTrigger, Count };

// What a binding does. Buttons are the Wii Remote's and the Nunchuk's.
enum class Action : std::uint8_t {
    StickUp,
    StickDown,
    StickLeft,
    StickRight,
    A,
    B,
    Plus,
    Minus,
    Home,
    One,
    Two,
    DpadUp,
    DpadDown,
    DpadLeft,
    DpadRight,
    NunchukC,
    NunchukZ,
    Shake,           // flick the remote: the game reads a swing (Mario spins)
    TiltHold,        // while held, the stick keys tilt the remote instead of the stick
    PostureToggle,   // switch the remote between pointing at the screen and upright
    Walk,            // while held, the stick moves at Settings::walkStickScale
    Start,           // A; A and B together while the title asks for both (titlePromptShown)
    // Native mods (native/MODS.md). Not remote buttons: each press is counted
    // for the game side (takeActionPresses) and does nothing unless its mod is on.
    ModCollectStarBits,
    ModShootEnemy,
    // Odyssey camera mod (docs/dev/ODYSSEY_CAMERA.md), only while it is on:
    CameraOrbitHold,  // while held, mouse motion orbits the camera (the pointer stays)
    CameraZoomIn,
    CameraZoomOut,
    CameraOrbitLeft,  // held: orbit (J/L, I/K by default)
    CameraOrbitRight,
    CameraPitchUp,
    CameraPitchDown,
    // Photo mode (docs/dev/ODYSSEY_CAMERA.md "Photo mode"), only while it is on:
    PhotoMode,  // freeze the game and fly the camera; again to return
    PhotoShot,  // in photo mode: save a screenshot
    // Cappy mod (docs/dev/CAPPY.md): throw the cap. While the mod is active, an
    // input bound to it and to Shake throws only (RB by default; X still spins).
    ModCappyThrow,
    // The level editor (docs/dev/level-editor/UX.md section 3), only while it is on (Mods page):
    // a tap enters Edit mode or leaves it. Tab by default, which also tilts the remote while
    // held (TiltHold): only a short tap with nothing else pressed toggles.
    LevelEditor,
    // SMG2: separate Nunchuk motion and optional second Remote. Append to
    // preserve all imported action identities and controls.txt names.
    NunchukShake,
    CoStarToggle,
    CoStarA,
    CoStarB,
    CoStarShake,
    // Playtest capture (F9/M on the keyboard, handled by the app; the Pro Controller's
    // Capture button by default). Not a remote button: counted like the mod actions.
    PlaytestCapture,
    // The Pro layouts' B: Wii A (jump) while Mario is in play (playControlShown),
    // else Wii B, so menus keep A = confirm, B = back. Decided at the press.
    PlayJump,
    Count
};

// Controller layouts. Standard is positional (Xbox, PlayStation, and any pad
// without Nintendo labels) and is the imported default set. A pad with Nintendo
// labels (Switch Pro, Joy-Con pair) uses ProGalaxy, or ProOdyssey while the
// OdysseyMovement mod is on (native/CONTROLS.md "Switch Pro Controller").
// Keyboard and mouse bindings are shared by all layouts.
enum class PadLayout : std::uint8_t { Standard, ProGalaxy, ProOdyssey, Count };
const char* padLayoutName(PadLayout layout);
// The layout for a pad: Standard unless it has Nintendo labels.
PadLayout padLayoutFor(bool nintendoLabels, bool odysseyMovement);

// Controller options in controls.txt's [Controller] section.
struct PadOptions {
    enum class Gyro : std::uint8_t { Auto, On, Off };
    // The gyro moves the Star Pointer: Auto in ProOdyssey (where the right stick
    // is the camera), On in every Nintendo layout, Off never.
    Gyro gyro = Gyro::Auto;
    float gyroSpeed = 1.0f;    // pointer speed multiplier
    bool gyroInvertX = false;
    bool gyroInvertY = false;
    bool rumble = true;        // the game's Wii Remote rumble on the controller
    float shakeG = 1.6f;       // accelerometer change (g) that counts as a shake (Pad:Shake)
    bool operator==(const PadOptions& other) const {
        return gyro == other.gyro && gyroSpeed == other.gyroSpeed && gyroInvertX == other.gyroInvertX &&
               gyroInvertY == other.gyroInvertY && rumble == other.rumble && shakeG == other.shakeG;
    }
};

struct Binding {
    enum class Device : std::uint8_t { Key, Mouse, Pad };
    Device device = Device::Key;
    std::uint16_t code = 0;  // KeyCode, MouseButton, or PadButton

    static Binding key(KeyCode code) { return {Device::Key, code}; }
    static Binding mouse(MouseButton button) { return {Device::Mouse, static_cast<std::uint16_t>(button)}; }
    static Binding pad(PadButton button) { return {Device::Pad, static_cast<std::uint16_t>(button)}; }
    bool operator==(const Binding& other) const { return device == other.device && code == other.code; }
};

// Remappable bindings. Each action may have several inputs, and an input may
// drive several actions.
// Each controller layout has its own controller buttons; layout() selects the
// one inputs(), bind(), unbind() and clear() see, next to the shared keyboard
// and mouse inputs. The host keeps it on the connected pad's layout.
class Bindings {
public:
    // The approved defaults from native/CONTROLS.md, plus provisional
    // bindings for the other buttons and tilt (see native/input/README.md),
    // seen through the given controller layout.
    static Bindings defaults(PadLayout layout = PadLayout::Standard);

    void bind(Action action, Binding input);    // adds; no effect if present
    void unbind(Action action, Binding input);
    void clear(Action action);
    // Keyboard and mouse inputs, then the current layout's controller buttons.
    const std::vector<Binding>& inputs(Action action) const;

    PadLayout layout() const { return mLayout; }
    void setLayout(PadLayout layout);
    PadOptions& options() { return mOptions; }
    const PadOptions& options() const { return mOptions; }

    // Text form, one "Action=Input,Input" line per action, for saving remaps.
    // Inputs are "Key:<name>" (see keyName), "Mouse:<Left|Middle|Right|X1|X2>"
    // or "Pad:<South|East|West|North|Back|Guide|Start|LeftStick|RightStick|
    // LeftShoulder|RightShoulder|DpadUp|DpadDown|DpadLeft|DpadRight|
    // LeftTrigger|RightTrigger|Misc1|Shake>".
    // Then, only where they differ from the defaults, a "[ProGalaxy]" and a
    // "[ProOdyssey]" section of controller buttons by Nintendo label
    // ("Pad:<A|B|X|Y|L|R|ZL|ZR|Plus|Minus|Home|Capture|LStick|RStick|
    // DpadUp|DpadDown|DpadLeft|DpadRight|Shake>"), and a "[Controller]"
    // section of PadOptions ("Gyro=auto|on|off", "GyroSpeed=1", "GyroInvertX=off",
    // "GyroInvertY=off", "Rumble=on", "ShakeG=1.6").
    std::string serialize() const;
    // Replaces bindings for the actions present in the text. Actions not
    // mentioned keep their current bindings. On error nothing changes.
    bool parse(const std::string& text, std::string* error);

private:
    static constexpr int kActions = static_cast<int>(Action::Count);
    static constexpr int kProLayouts = 2;
    std::vector<Binding>& padSet(int layout, int action);
    void rebuild(int action);

    // Keyboard, mouse and the Standard layout's controller buttons.
    std::vector<Binding> mInputs[kActions];
    // The controller buttons of ProGalaxy and ProOdyssey.
    std::vector<Binding> mPro[kProLayouts][kActions];
    // inputs() in a Pro layout: mInputs' keyboard and mouse inputs, then mPro's.
    std::vector<Binding> mView[kActions];
    PadLayout mLayout = PadLayout::Standard;
    PadOptions mOptions;
};

const char* actionName(Action action);

// The player-facing controls list for a set of bindings (the Home menu's
// Controls page): one line per thing the player does, with the inputs bound
// to it as they appear on the keyboard ("Space / Right mouse"), or
// "(not bound)". Follows remaps. Allocates: on a game thread, call it inside
// a HostAllocationScope.
struct ControlsLine {
    std::string action;
    std::string inputs;  // keyboard and mouse
    std::string pad;     // game controller ("" if it has none)
};
std::vector<ControlsLine> controlsSummary(const Bindings& bindings);
// One line for the title screen, which asks for A and B together:
// "Keyboard: Return starts   |   F1: all controls", from the bindings (without
// a Start key: "hold Space, then press Backspace"). Allocates, like
// controlsSummary.
std::string titleHint(const Bindings& bindings);
// How an input appears to the player: "Space", "Left Shift", "Left mouse".
// Controller buttons by position ("Pad bottom"), or in a Pro layout by their
// Nintendo label ("Pad B", "Pad ZR").
std::string displayName(Binding input, PadLayout layout = PadLayout::Standard);
// The constant names in Key ("W", "Num1", "LeftShift", ...), or "Usage<n>".
std::string keyName(KeyCode code);
bool parseKeyName(const std::string& name, KeyCode* code);

// Where the game image is drawn in the window, in the units of mouse events
// (SDL3: window coordinates). The Star Pointer spans the image; the rest of
// the window is letterboxing.
struct Viewport {
    float windowWidth = 0.0f;
    float windowHeight = 0.0f;
    float imageX = 0.0f;
    float imageY = 0.0f;
    float imageWidth = 0.0f;
    float imageHeight = 0.0f;

    // The largest image of the given aspect ratio (width / height) centered in
    // the window.
    static Viewport letterbox(float windowWidth, float windowHeight, float imageAspect);
};

// Posture of the remote at rest. Pointing: level and aimed at the screen (the
// pointer works). Upright: tip up, tilted 10 degrees toward the screen, as the
// Star Ball and Ray surfing instructions ask (the IR camera sees no sensor
// bar, as on the Wii).
enum class Posture : std::uint8_t { Pointing, Upright };

struct Settings {
    // Remote distance from the sensor bar in metres, reported as KPAD dist.
    float pointerDistance = 2.0f;
    // Largest tilt from the posture, in degrees, for the stick keys with
    // TiltHold. Ray surfing turns need a twist of at least 40.5 degrees
    // (SurfRayTutorial: 0.65 g across the remote).
    float maxTiltDegrees = 45.0f;
    // How fast the remote turns toward a new tilt, in degrees per second.
    float tiltRateDegreesPerSecond = 360.0f;
    // A button press or release lasts at least this many 5 ms reports, so a
    // quick tap spans a KPADRead at 60 frames per second (KPAD compares only
    // the newest report with the previous read).
    int minimumPulseReports = 4;
    // Stick length while Walk is held (the keys otherwise give a full stick).
    float walkStickScale = 0.5f;
    // Mario ignores a swing that starts within 10 frames of pressing A or B
    // (MarioActor::updateControllerSwing). A keyboard shake that soon after a
    // jump is delayed until that window has passed, so jump-then-spin works.
    // Reports from the A or B press; 0 disables the delay.
    // Shake presses also start at least 250 ms apart (a flick must return
    // before the next, or the game sees one long swing). A press while the
    // previous flick is returning waits; at most one waits, and losing focus
    // cancels it.
    int shakeDelayAfterButtonReports = 36;
    // Plus and Minus presses last at least this many reports. The game pauses
    // only once Plus or Minus has been held for 12 frames
    // (PauseButtonCheckerInGame), so a tap of Escape would do nothing; 50
    // reports (250 ms) span 14 frames. The pause menu closes on a new press,
    // and its hold counter is not updated while paused, so the long press
    // cannot reopen it.
    int pauseTapReports = 50;
    // Game controller sticks: a radial dead zone (0..1 of full travel); past
    // it the left stick is the Nunchuk stick, analog.
    float padStickDeadZone = 0.2f;
    // The right stick moves the Star Pointer at up to this many image widths
    // per second (the image is 2 KPAD units wide).
    float padPointerWidthsPerSecond = 0.9f;
};

// --- Host events (any thread) ---

void setBindings(const Bindings& bindings);
Bindings bindings();
void setSettings(const Settings& settings);
Settings settings();

// The device of the latest press (a deliberate stick push counts as the pad): Key by default.
// For hints that name the input the player is using. Any thread.
Binding::Device lastDevice();

// repeat: the OS auto-repeat flag. Repeats are ignored; the game's KPAD
// button repeat applies instead.
void keyEvent(KeyCode code, bool down, bool repeat);
void mouseButtonEvent(MouseButton button, bool down);
// Mouse position in window coordinates.
void mouseMoved(float x, float y);
// Scroll, as SDL3 reports it (x right, y away from the user). Only the Odyssey
// camera mod uses it: precise (fractional, trackpad or Magic Mouse) scrolling
// orbits, whole wheel clicks zoom (camera.txt ScrollMode overrides).
void mouseWheel(float x, float y);
// The native menus' scrolling (the Home menu's Controls list). menuWheel takes
// the wheel as the system reports it (SDL3's event.wheel.y without the FLIPPED
// correction: + scrolls toward the top of a list), so macOS natural scrolling
// is honoured; the SDL3 adapter calls it for every wheel event.
void menuWheel(float y);
// Rows to scroll since the last call (+ toward the top): a whole wheel notch is
// 3 rows, precise (trackpad, Magic Mouse) scrolling 1 row per 2 units, with the
// fraction carried over. And Page Up / Page Down presses (key repeats count)
// since the last call (+ for Page Down). Any thread.
struct MenuScroll {
    int rows = 0;
    int pages = 0;
};
MenuScroll takeMenuScroll();
// Trackpad pinch: scale change since the last update (> 1 = zoom in).
void pinch(float scale);
// The mouse left the window: the remote points away from the screen.
void mouseLeft();
void setViewport(const Viewport& viewport);
// Losing focus releases every held input and hides the pointer, so nothing
// stays pressed while the window is in the background.
void focusChanged(bool focused);
// Game controllers (all of them act as one). Axis values are -1..1 for the
// sticks, x right and y DOWN as SDL reports them, and 0..1 for the triggers.
void padButtonEvent(PadButton button, bool down);
void padAxisEvent(PadAxis axis, float value);
// The last controller was removed: everything it held is released.
void padDisconnected();
// A controller was connected; nintendoLabels: its bottom face button is labelled
// B (Switch Pro, Joy-Con). Selects the controller layout (padLayout).
void padConnected(bool nintendoLabels);
// The controller layout in effect: from the connected pad and the OdysseyMovement
// mod, followed as either changes. Any thread.
PadLayout padLayout();
// Controller motion sensors, as SDL3 reports them: the gyro in radians per second
// (x pitch up, y yaw left, z roll), the accelerometer in m/s^2. In a Nintendo
// layout the gyro moves the Star Pointer (PadOptions::gyro) and a shake presses
// Pad:Shake. seconds: the sample's time (any monotonic clock).
enum class PadSensor : std::uint8_t { Gyro, Accel };
void padSensorEvent(PadSensor sensor, const float data[3], double seconds);
// Whether controller rumble should run now: the game's motor (rumbleActive(0))
// and PadOptions::rumble. Any thread.
bool padRumble();
void setPosture(Posture posture);
Posture posture();

// Game side (native builds): the title screen is asking for A and B held
// together this frame (TitleSequenceProduct::exeLogoDisplay calls it every
// frame the prompt is up). For the next 100 ms the Start action (Return)
// presses B with A, so one key starts the game. Elsewhere Start is A alone,
// and never sends the B that backs out of menus. Any thread.
void titlePromptShown();
// Host side: the title prompt was reported within the last 100 ms (the
// native title hint shows while it is). Any thread.
bool titlePromptActive();
// Game side (native builds): the game steers by the remote's tilt this frame.
// For the next 100 ms the stick keys tilt the remote instead of moving the
// stick, as Tab would, so WASD steer with no extra keys. Any thread.
enum class Steering : std::uint8_t {
    Ball,  // Star Ball (SphereAccelSensorController): upright, WASD tilt
    Ray,   // Ray surfing (SurfRay::updateRide): level, A/D twist, W/S ignored
};
void motionControlShown(Steering steering);
// Game side (native builds): Mario is in play this frame (a stage, no menu,
// demo, pause or system talk). For the next 100 ms PlayJump presses are Wii A;
// otherwise, in menus, the file select, the world map and the Home menu, Wii B.
void playControlShown();
// Host side: playControlShown was reported within the last 100 ms. Any thread.
bool playControlActive();

// Presses (down edges, OS repeats ignored) of an input bound to a mod action
// since the last call, then zero. Any thread. Other actions always return 0.
int takeActionPresses(Action action);
// Adds one press of a mod action, as if its key went down (tests and drivers).
void injectActionPress(Action action);
// Whether an input bound to a mod action is down now. Any thread.
bool actionHeld(Action action);

// Read-only bound output for route recording, including controller axes.
struct BoundState {
    float moveX = 0, moveY = 0;
    bool jump = false, spin = false, crouch = false;
    bool cameraLeft = false, cameraRight = false, cameraCenter = false;
    bool cameraUp = false, cameraDown = false;
    bool shoot = false, pointerValid = false;  // bound B and visible host pointer intent, before KPAD filtering
    // KPAD screen coordinates: x right, y down, screen -1..1; mouse intent can extend outside the image.
    float pointerX = 0, pointerY = 0;
};
BoundState boundState();

// --- Device state ---

// Connects or disconnects the virtual remote on a channel. Channel 0 is
// connected by WPADInit (after the Bluetooth reconnection delay) and
// reconnects when a bound input is pressed after WPADDisconnect.
void setConnected(int channel, bool connected);
void setNunchukAttached(int channel, bool attached);
// Rumble motor state last set by WPADControlMotor, for host haptics.
bool rumbleActive(int channel);

// Remote speaker. The game streams 4-bit ADPCM (WENC) packets. Without a sink
// they are consumed at the device rate and not played.
using SpeakerSink = void (*)(int channel, const std::uint8_t* data, std::uint16_t length, void* user);
void setSpeakerSink(SpeakerSink sink, void* user);

// --- Report clock ---

enum class Clock : std::uint8_t {
    Alarm,   // default: a 5 ms periodic OSAlarm, like WPAD's manage handler
    Manual,  // reports are produced only by pumpReports (tests, frame-locked hosts)
};
// Takes effect at WPADInit, or immediately if WPAD is running.
void setClock(Clock clock);
// Produces count reports, as count * 5 ms of device time. Runs callbacks with
// interrupts disabled, as the alarm does. Any thread.
void pumpReports(int count);

// Clears all state, bindings, settings, and the SDK layer's initialization.
// Tests only; WPAD must not be in use.
void resetForTesting();

}  // namespace PetariNative::Input
