#include "probe_core.h"
#include <stdio.h>

#define B(label, id, extra, only) {label, SDL_GAMEPAD_BUTTON_##id, -1, 0, extra, only}
#define A(label, id, dir) {label, -1, SDL_GAMEPAD_AXIS_##id, dir, 0, 0}
const ProbeControl probe_controls[] = {
    B("A (Nintendo right)", EAST, 0, 0), B("B (Nintendo bottom)", SOUTH, 0, 0),
    B("X (Nintendo top)", NORTH, 0, 0), B("Y (Nintendo left)", WEST, 0, 0),
    B("L", LEFT_SHOULDER, 0, 0), B("R", RIGHT_SHOULDER, 0, 0),
    A("ZL", LEFT_TRIGGER, 1), A("ZR", RIGHT_TRIGGER, 1),
    B("+", START, 0, 0), B("-", BACK, 0, 0), B("Home", GUIDE, 0, 0),
    B("Capture", MISC1, 1, 0), B("C", MISC2, 1, 1),
    B("GL (left grip)", LEFT_PADDLE1, 1, 1),
    B("GR (right grip)", RIGHT_PADDLE1, 1, 1),
    B("L3", LEFT_STICK, 0, 0), B("R3", RIGHT_STICK, 0, 0),
    B("D-pad up", DPAD_UP, 0, 0), B("D-pad down", DPAD_DOWN, 0, 0),
    B("D-pad left", DPAD_LEFT, 0, 0), B("D-pad right", DPAD_RIGHT, 0, 0),
    A("left stick up >50%", LEFTY, -1), A("left stick down >50%", LEFTY, 1),
    A("left stick left >50%", LEFTX, -1), A("left stick right >50%", LEFTX, 1),
    A("right stick up >50%", RIGHTY, -1), A("right stick down >50%", RIGHTY, 1),
    A("right stick left >50%", RIGHTX, -1), A("right stick right >50%", RIGHTX, 1)
};
const size_t probe_control_count = sizeof(probe_controls) / sizeof(probe_controls[0]);
const char *const probe_layer_names[L_COUNT] = {
    "libusb load", "USB enumerate", "USB open", "bulk interface claim", "IOHID open",
    "SDL driver attached", "SDL mapping", "gamepad event flow", "mapping of extras", "sensors", "rumble"
};
const char *probe_status_name(ProbeStatus s)
{
    return s == PROBE_PASS ? "PASS" : s == PROBE_FAIL ? "FAIL" : "SKIP";
}
const char *probe_button_name(int b)
{
    static const char *const names[] = {
        "SOUTH", "EAST", "WEST", "NORTH", "BACK", "GUIDE", "START", "LEFT_STICK",
        "RIGHT_STICK", "LEFT_SHOULDER", "RIGHT_SHOULDER", "DPAD_UP", "DPAD_DOWN",
        "DPAD_LEFT", "DPAD_RIGHT", "MISC1", "RIGHT_PADDLE1", "LEFT_PADDLE1",
        "RIGHT_PADDLE2", "LEFT_PADDLE2", "TOUCHPAD", "MISC2", "MISC3", "MISC4",
        "MISC5", "MISC6"
    };
    return b >= 0 && (size_t)b < sizeof(names)/sizeof(names[0]) ? names[b] : "UNKNOWN";
}
const char *probe_axis_name(int a)
{
    static const char *const names[] = {"LEFTX", "LEFTY", "RIGHTX", "RIGHTY", "LEFT_TRIGGER", "RIGHT_TRIGGER"};
    return a >= 0 && a < 6 ? names[a] : "UNKNOWN";
}
size_t probe_hex(char *out, size_t cap, const unsigned char *bytes, size_t count)
{
    size_t used = 0, n = 0;
    if (!cap) return 0;
    out[0] = '\0';
    while (n < count) {
        size_t need = n ? 3 : 2;
        if (cap - used <= need) break;
        int wrote = snprintf(out + used, cap - used, n ? " %02x" : "%02x", bytes[n]);
        used += (size_t)wrote;
        ++n;
    }
    return n;
}
int probe_control_matches(const ProbeControl *c, int button, int axis, int value)
{
    return c->button >= 0 ? button == c->button :
        axis == c->axis && (c->direction > 0 ? value > 16383 : value < -16383);
}
const char *probe_verdict(const ProbeResult r[L_COUNT], int seen, int gamepad, int switch2)
{
    if (r[L_LOAD].status == PROBE_FAIL) return "LIBUSB_LOAD_FAILED: inspect dlopen paths/signatures";
    if (seen < 0) return "USB_ENUMERATION_FAILED: libusb could not establish the device inventory";
    if (!seen) return "NO_PAD: no Nintendo USB/HID device or SDL joystick seen before deadline";
    if (!gamepad) {
        if (r[L_USB_ENUM].status == PROBE_FAIL) return "USB_ENUMERATION_FAILED: inspect USB cable/device enumeration";
        if (r[L_USB_OPEN].status == PROBE_FAIL) return "USB_OPEN_FAILED: inspect libusb error/other owners";
        if (r[L_BULK].status == PROBE_FAIL) return "BULK_CLAIM_FAILED: handshake interface unavailable";
        if (r[L_HID].status == PROBE_FAIL) return "IOHID_OPEN_FAILED: inspect IOKit error/other owners";
        if (r[L_DRIVER].status == PROBE_FAIL) return "SDL_DRIVER_FAILED: inspect INPUT driver/open logs";
        return "SDL_MAPPING_FAILED: joystick exists but no SDL gamepad could be opened";
    }
    if (r[L_EVENTS].status != PROBE_PASS) return "EVENTS_FAILED: gamepad opened but no guided input arrived";
    if (!switch2) return "OTHER_GAMEPAD: event flow works; Switch 2 USB/extras qualification skipped";
    for (int i = L_USB_ENUM; i < L_COUNT; ++i)
        if (r[i].status == PROBE_FAIL) return "PARTIAL: inspect FAIL layers and unexpected control IDs";
    for (int i = 0; i < L_COUNT; ++i)
        if (r[i].status == PROBE_SKIP) return "INCOMPLETE: increase -t and repeat skipped tests";
    return "PASS: all measured layers passed; confirm physical rumble and game behavior separately";
}
