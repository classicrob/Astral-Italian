#include "probe_core.h"
#include <stdio.h>
#include <string.h>

#define CHECK(x) do { if (!(x)) { fprintf(stderr, "FAIL line %d: %s\n", __LINE__, #x); return 1; } } while (0)
int main(void)
{
    unsigned char bytes[] = {0, 0x0f, 0x80, 0xff};
    char hex[12], tiny[3], sentinel = 'x';
    CHECK(probe_hex(hex, sizeof(hex), bytes, 4) == 4);
    CHECK(strcmp(hex, "00 0f 80 ff") == 0);
    CHECK(probe_hex(tiny, sizeof(tiny), bytes, 4) == 1 && !strcmp(tiny, "00"));
    CHECK(probe_hex(&sentinel, 0, bytes, 4) == 0 && sentinel == 'x');
    CHECK(probe_hex(hex, sizeof(hex), bytes, 0) == 0 && !*hex);
    CHECK(probe_control_count == 29);
    const int expected[] = {SDL_GAMEPAD_BUTTON_EAST, SDL_GAMEPAD_BUTTON_SOUTH,
        SDL_GAMEPAD_BUTTON_NORTH, SDL_GAMEPAD_BUTTON_WEST};
    int extras = 0, axes = 0;
    for (size_t i = 0; i < probe_control_count; ++i) {
        const ProbeControl *c = &probe_controls[i];
        if (i < 4) CHECK(c->button == expected[i]);
        if (c->extra) {
            ++extras;
            CHECK(!strcmp(probe_button_name(c->button),
                extras == 1 ? "MISC1" : extras == 2 ? "MISC2" : extras == 3 ? "LEFT_PADDLE1" : "RIGHT_PADDLE1"));
        }
        if (c->button < 0) {
            ++axes;
            CHECK(probe_control_matches(c, -1, c->axis, c->direction * 16384));
            CHECK(!probe_control_matches(c, -1, c->axis, c->direction * 16383));
            CHECK(!probe_control_matches(c, -1, c->axis, -c->direction * 20000));
        } else {
            CHECK(probe_control_matches(c, c->button, -1, 0));
            CHECK(!probe_control_matches(c, (c->button + 1) % SDL_GAMEPAD_BUTTON_COUNT, -1, 0));
        }
    }
    CHECK(extras == 4 && axes == 10);
    ProbeResult r[L_COUNT] = {0};
    for (int i = 0; i < L_COUNT; ++i) r[i].status = PROBE_PASS;
    CHECK(!strncmp(probe_verdict(r, 0, 0, 0), "NO_PAD:", 7));
    CHECK(!strncmp(probe_verdict(r, -1, 0, 0), "USB_ENUMERATION_FAILED:", 23));
    r[L_LOAD].status = PROBE_FAIL;
    CHECK(!strncmp(probe_verdict(r, 0, 0, 0), "LIBUSB_LOAD_FAILED:", 19));
    r[L_LOAD].status = PROBE_PASS;
    const struct { int layer; const char *prefix; } cases[] = {
        {L_USB_ENUM, "USB_ENUMERATION_FAILED:"}, {L_USB_OPEN, "USB_OPEN_FAILED:"},
        {L_BULK, "BULK_CLAIM_FAILED:"}, {L_HID, "IOHID_OPEN_FAILED:"},
        {L_DRIVER, "SDL_DRIVER_FAILED:"}
    };
    for (size_t i = 0; i < sizeof(cases)/sizeof(cases[0]); ++i) {
        r[cases[i].layer].status = PROBE_FAIL;
        CHECK(!strncmp(probe_verdict(r, 1, 0, 1), cases[i].prefix, strlen(cases[i].prefix)));
        r[cases[i].layer].status = PROBE_PASS;
    }
    CHECK(!strncmp(probe_verdict(r, 1, 0, 1), "SDL_MAPPING_FAILED:", 19));
    r[L_EVENTS].status = PROBE_FAIL;
    CHECK(!strncmp(probe_verdict(r, 1, 1, 1), "EVENTS_FAILED:", 14));
    r[L_EVENTS].status = PROBE_PASS;
    CHECK(!strncmp(probe_verdict(r, 1, 1, 0), "OTHER_GAMEPAD:", 14));
    r[L_EXTRAS].status = PROBE_FAIL;
    CHECK(!strncmp(probe_verdict(r, 1, 1, 1), "PARTIAL:", 8));
    r[L_EXTRAS].status = PROBE_SKIP;
    CHECK(!strncmp(probe_verdict(r, 1, 1, 1), "INCOMPLETE:", 11));
    r[L_EXTRAS].status = PROBE_PASS;
    CHECK(!strncmp(probe_verdict(r, 1, 1, 1), "PASS:", 5));
    puts("PASS hex formatting/bounds; Nintendo labels/extras/directions; verdict layer precedence");
    return 0;
}
