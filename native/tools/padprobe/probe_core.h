#ifndef PADPROBE_CORE_H
#define PADPROBE_CORE_H

#include <SDL3/SDL.h>
#include <stddef.h>

typedef enum { PROBE_SKIP, PROBE_PASS, PROBE_FAIL } ProbeStatus;
typedef enum {
    L_LOAD, L_USB_ENUM, L_USB_OPEN, L_BULK, L_HID, L_DRIVER, L_MAPPING,
    L_EVENTS, L_EXTRAS, L_SENSORS, L_RUMBLE, L_COUNT
} ProbeLayer;
typedef struct { ProbeStatus status; char reason[192]; } ProbeResult;
typedef struct {
    const char *label;
    int button;                 /* -1 for an axis */
    int axis;
    int direction;
    int extra;                 /* capture, C, GL, GR */
    int switch2_only;
} ProbeControl;

extern const ProbeControl probe_controls[];
extern const size_t probe_control_count;
extern const char *const probe_layer_names[L_COUNT];
const char *probe_status_name(ProbeStatus status);
const char *probe_button_name(int button);
const char *probe_axis_name(int axis);
/* Number of bytes consumed. A byte is emitted only when its full token fits. */
size_t probe_hex(char *out, size_t capacity, const unsigned char *bytes, size_t count);
int probe_control_matches(const ProbeControl *control, int button, int axis, int value);
/* device_seen: -1 if USB enumeration itself failed, 0 absent, 1 present. */
const char *probe_verdict(const ProbeResult results[L_COUNT], int device_seen,
                          int gamepad, int switch2);
#endif
