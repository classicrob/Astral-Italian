/* macOS controller layer probe. Never detaches a driver or sends USB transfers. */
#include "probe_core.h"
#include <libusb.h>
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/hid/IOHIDManager.h>
#include <IOKit/hid/IOHIDKeys.h>
#include <dlfcn.h>
#include <mach-o/dyld.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sysctl.h>
#include <sys/utsname.h>
#include <time.h>
#include <unistd.h>

#ifndef PADPROBE_GIT_ID
#define PADPROBE_GIT_ID "unspecified"
#endif

static FILE *report;
static pthread_mutex_t report_lock = PTHREAD_MUTEX_INITIALIZER;
static size_t report_bytes;
static size_t detail_limit = 8000, log_bytes;
static int log_capped;
static int summary_mode, report_error, detail_capped;
static ProbeResult layers[L_COUNT];
static Uint64 deadline;
static int usb_seen, hid_seen, joystick_seen, switch2_usb, switch2_hid, usb_enumeration_error;
static SDL_Gamepad *gamepad;
static SDL_JoystickID selected;
static int selected_switch2, any_input;

/* Both streams get identical, bounded content. Reserve 5 KB for the summary. */
static void emit(const char *format, ...)
{
    char line[1536];
    va_list args;
    va_start(args, format);
    vsnprintf(line, sizeof(line), format, args);
    va_end(args);
    pthread_mutex_lock(&report_lock);
    size_t n = strlen(line);
    if (report_bytes + n <= (summary_mode ? 29000u : detail_limit)) {
        fputs(line, stdout);
        if (fwrite(line, 1, n, report) != n || fflush(report)) report_error = 1;
        report_bytes += n;
    } else if (!summary_mode && !detail_capped) {
        const char *notice = "SKIP report detail: phase budget reached; later phases/summary space reserved\n";
        fputs(notice, stdout);
        if (fputs(notice, report) < 0) report_error = 1;
        report_bytes += strlen(notice);
        detail_capped = 1;
    }
    pthread_mutex_unlock(&report_lock);
}
static void result(ProbeLayer layer, ProbeStatus status, const char *format, ...)
{
    layers[layer].status = status;
    va_list args;
    va_start(args, format);
    vsnprintf(layers[layer].reason, sizeof(layers[layer].reason), format, args);
    va_end(args);
    emit("%s %s: %s\n", probe_status_name(status), probe_layer_names[layer], layers[layer].reason);
}
static void SDLCALL capture_log(void *unused, int category, SDL_LogPriority priority, const char *message)
{
    (void)unused;
    pthread_mutex_lock(&report_lock);
    size_t n = strlen(message);
    int keep = log_bytes + n < 4500;
    int notice = !keep && !log_capped;
    if (notice) log_capped = 1;
    log_bytes += n;
    pthread_mutex_unlock(&report_lock);
    if (notice) emit("SKIP SDL logs: 4.5 KB allowance reached; repeat with one controller for complete logs\n");
    if (!keep) return;
    emit("SDL log category=%d priority=%d: %.1400s\n", category, priority, message);
}
static const char *safe(const char *s) { return s ? s : "(none)"; }
static const char *usb_image(void)
{
    for (uint32_t i = 0; i < _dyld_image_count(); ++i) {
        const char *path = _dyld_get_image_name(i);
        const char *name = path ? strrchr(path, '/') : NULL;
        name = name ? name + 1 : path;
        if (name && !strncmp(name, "libusb-1.0", 10) && strlen(name) >= 6 &&
            !strcmp(name + strlen(name) - 6, ".dylib")) return path;
    }
    return "not loaded";
}

/* Derive function types from the real libusb header; no private ABI copies. */
#define USB_FUNCTIONS(X) \
    X(init) X(exit) X(get_device_list) X(free_device_list) X(get_device_descriptor) \
    X(get_bus_number) X(get_port_numbers) X(get_device_address) \
    X(get_config_descriptor) X(free_config_descriptor) X(open) X(close) \
    X(get_string_descriptor_ascii) X(kernel_driver_active) X(claim_interface) \
    X(release_interface) X(error_name) X(get_version)
typedef struct {
    void *image;
#define FIELD(name) __typeof__(&libusb_##name) name;
    USB_FUNCTIONS(FIELD)
#undef FIELD
} UsbApi;

static int load_usb(UsbApi *u, int verbose)
{
    const char *paths[] = {
        "@executable_path/Frameworks/libusb-1.0.0.dylib",
        "@executable_path/../Frameworks/libusb-1.0.0.dylib",
        "libusb-1.0.dylib", "libusb-1.0.0.dylib"
    };
    memset(u, 0, sizeof(*u));
    for (size_t i = 0; i < sizeof(paths)/sizeof(paths[0]); ++i) {
        u->image = dlopen(paths[i], RTLD_NOW | RTLD_LOCAL);
        if (verbose) emit("%s libusb dlopen %s: %s\n", u->image ? "PASS" : "FAIL", paths[i], u->image ? "loaded" : safe(dlerror()));
        if (u->image) break;
    }
    if (!u->image) {
        if (verbose) result(L_LOAD, PROBE_FAIL, "all own dlopen paths failed (independent of SDL)");
        return 0;
    }
#define RESOLVE(name) do { \
    u->name = (__typeof__(u->name))dlsym(u->image, "libusb_" #name); \
    if (!u->name) { \
        if (verbose) result(L_LOAD, PROBE_FAIL, "missing symbol libusb_" #name); \
        dlclose(u->image); u->image = NULL; return 0; \
    } \
} while (0);
    USB_FUNCTIONS(RESOLVE)
#undef RESOLVE
    if (verbose) {
        const struct libusb_version *v = u->get_version();
        result(L_LOAD, PROBE_PASS, "own dlopen: %s; libusb %u.%u.%u", usb_image(), v->major, v->minor, v->micro);
    }
    return 1;
}
static unsigned long mix(unsigned long hash, unsigned long value) { return hash * 33u + value + 1u; }

/* All configurations/alternate settings are described; claim each interface once,
 * without changing configuration/alternate setting. Other vendors are never opened. */
static unsigned long usb_snapshot(int verbose, int all)
{
    UsbApi u;
    if (!load_usb(&u, verbose)) return 0;
    libusb_context *ctx = NULL;
    int rc = u.init(&ctx);
    if (rc < 0) {
        if (verbose) {
            usb_enumeration_error = 1;
            result(L_USB_ENUM, PROBE_FAIL, "libusb_init %s (%d)", u.error_name(rc), rc);
        }
        dlclose(u.image);
        return 0;
    }
    libusb_device **devices = NULL;
    ssize_t count = u.get_device_list(ctx, &devices);
    unsigned long signature = 0;
    int wanted_pid = 0, found = 0, opened = 0, bulk_found = 0, bulk_ok = 0;
    for (ssize_t i = 0; i < count; ++i) {
        struct libusb_device_descriptor d;
        if (!u.get_device_descriptor(devices[i], &d) && d.idVendor == 0x057e && d.idProduct == 0x2069) wanted_pid = 0x2069;
    }
    for (ssize_t i = 0; i < count; ++i) {
        struct libusb_device_descriptor d;
        rc = u.get_device_descriptor(devices[i], &d);
        if (rc < 0) { if (verbose) emit("FAIL USB descriptor: %s (%d)\n", u.error_name(rc), rc); continue; }
        int nintendo = d.idVendor == 0x057e;
        int target = nintendo && (!wanted_pid || d.idProduct == wanted_pid);
        if (nintendo) {
            ++found;
            signature += mix(mix(d.idProduct, u.get_bus_number(devices[i])), u.get_device_address(devices[i]));
        }
        if (!verbose || (!all && !nintendo)) continue;
        uint8_t ports[8];
        int nports = u.get_port_numbers(devices[i], ports, sizeof(ports));
        char porttext[32] = "", *p = porttext;
        for (int j = 0; j < nports && j < 8; ++j) p += snprintf(p, (size_t)(porttext + sizeof(porttext) - p), "%s%u", j ? "." : "", ports[j]);
        libusb_device_handle *handle = NULL;
        int open_rc = nintendo ? u.open(devices[i], &handle) : LIBUSB_ERROR_NOT_SUPPORTED;
        unsigned char serial[128] = "unavailable";
        if (handle && d.iSerialNumber) {
            int len = u.get_string_descriptor_ascii(handle, d.iSerialNumber, serial, sizeof(serial)-1);
            if (len >= 0) serial[len] = 0;
            else snprintf((char *)serial, sizeof(serial), "%s (%d)", u.error_name(len), len);
        }
        emit("USB %04x:%04x serial=%s bus=%u port=%s address=%u configs=%u\n", d.idVendor, d.idProduct,
             nintendo ? (char *)serial : "SKIP (other vendor never opened)", u.get_bus_number(devices[i]),
             *porttext ? porttext : "unknown", u.get_device_address(devices[i]), d.bNumConfigurations);
        if (nintendo) emit("%s USB open %04x:%04x: %s (%d)\n", open_rc == 0 ? "PASS" : "FAIL", d.idVendor, d.idProduct, u.error_name(open_rc), open_rc);
        if (target && handle) ++opened;
        unsigned char checked[256] = {0};
        for (int config = 0; config < d.bNumConfigurations; ++config) {
            struct libusb_config_descriptor *c = NULL;
            rc = u.get_config_descriptor(devices[i], (uint8_t)config, &c);
            if (rc < 0) { emit("FAIL USB configuration %d: %s (%d)\n", config, u.error_name(rc), rc); continue; }
            for (int iface = 0; iface < c->bNumInterfaces; ++iface) {
                const struct libusb_interface *it = &c->interface[iface];
                for (int alt = 0; alt < it->num_altsetting; ++alt) {
                    const struct libusb_interface_descriptor *a = &it->altsetting[alt];
                    int bulk_in = 0, bulk_out = 0;
                    emit("  interface=%u config=%u alt=%u class=%02x subclass=%02x protocol=%02x endpoints=%u\n",
                         a->bInterfaceNumber, c->bConfigurationValue, a->bAlternateSetting, a->bInterfaceClass,
                         a->bInterfaceSubClass, a->bInterfaceProtocol, a->bNumEndpoints);
                    for (int ep = 0; ep < a->bNumEndpoints; ++ep) {
                        const struct libusb_endpoint_descriptor *e = &a->endpoint[ep];
                        emit("    endpoint=%02x type=%u maxPacket=%u interval=%u\n", e->bEndpointAddress,
                             e->bmAttributes & LIBUSB_TRANSFER_TYPE_MASK, e->wMaxPacketSize, e->bInterval);
                        if ((e->bmAttributes & LIBUSB_TRANSFER_TYPE_MASK) == LIBUSB_TRANSFER_TYPE_BULK) {
                            if (e->bEndpointAddress & LIBUSB_ENDPOINT_IN) bulk_in = 1; else bulk_out = 1;
                        }
                    }
                    int is_bulk = a->bInterfaceClass == LIBUSB_CLASS_VENDOR_SPEC && bulk_in && bulk_out;
                    if (target && is_bulk) ++bulk_found;
                    if (!nintendo || checked[a->bInterfaceNumber]) continue;
                    checked[a->bInterfaceNumber] = 1;
                    if (!handle) { emit("SKIP USB interface %u claim/kernel-active: USB open failed\n", a->bInterfaceNumber); continue; }
                    int active = u.kernel_driver_active(handle, a->bInterfaceNumber);
                    emit("%s USB interface %u kernel_driver_active=%d (%s; %s)\n", active >= 0 ? "PASS" : "FAIL",
                         a->bInterfaceNumber, active, u.error_name(active < 0 ? active : 0),
                         active >= 0 ? (active ? "kernel owns interface" : "no kernel owner") : "query failed");
                    int claim = u.claim_interface(handle, a->bInterfaceNumber);
                    emit("%s USB interface %u claim=%s (%d)%s\n", claim == 0 ? "PASS" : "FAIL", a->bInterfaceNumber,
                         u.error_name(claim), claim, a->bInterfaceClass == 3 ? " (HID claim failure is expected on macOS)" : "");
                    if (!claim) {
                        int release = u.release_interface(handle, a->bInterfaceNumber);
                        emit("%s USB interface %u immediate release=%s (%d)\n", release == 0 ? "PASS" : "FAIL", a->bInterfaceNumber, u.error_name(release), release);
                        if (target && is_bulk && !release) ++bulk_ok;
                    }
                }
            }
            u.free_config_descriptor(c);
        }
        if (handle) u.close(handle);
    }
    if (verbose) {
        usb_seen = found;
        usb_enumeration_error = count < 0;
        switch2_usb = wanted_pid != 0;
        result(L_USB_ENUM, count < 0 || !found ? PROBE_FAIL : PROBE_PASS,
               "libusb_get_device_list=%ld (%s); Nintendo devices=%d; summary target=%s", (long)count,
               count < 0 ? u.error_name((int)count) : "success", found, wanted_pid ? "057e:2069" : "any 057e");
        result(L_USB_OPEN, !found ? PROBE_SKIP : opened ? PROBE_PASS : PROBE_FAIL,
               "%d target devices opened; handles closed; no detach/reset/transfers", opened);
        result(L_BULK, !wanted_pid ? PROBE_SKIP : bulk_ok ? PROBE_PASS : PROBE_FAIL,
               "%d vendor bulk IN+OUT settings; %d claimed and released (Switch 2 only)", bulk_found, bulk_ok);
    }
    if (devices) u.free_device_list(devices, 1);
    u.exit(ctx);
    dlclose(u.image);
    return signature;
}

static long hid_number(IOHIDDeviceRef d, CFStringRef key)
{
    CFTypeRef value = IOHIDDeviceGetProperty(d, key);
    long number = 0;
    if (value && CFGetTypeID(value) == CFNumberGetTypeID()) CFNumberGetValue((CFNumberRef)value, kCFNumberLongType, &number);
    return number;
}
static const char *hid_string(IOHIDDeviceRef d, CFStringRef key, char *buffer, size_t size)
{
    CFTypeRef value = IOHIDDeviceGetProperty(d, key);
    if (!value || CFGetTypeID(value) != CFStringGetTypeID() ||
        !CFStringGetCString((CFStringRef)value, buffer, (CFIndex)size, kCFStringEncodingUTF8)) snprintf(buffer, size, "(unknown)");
    return buffer;
}
static unsigned long hid_snapshot(int verbose)
{
    IOHIDManagerRef manager = IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDOptionsTypeNone);
    if (!manager) { if (verbose) result(L_HID, PROBE_FAIL, "IOHIDManagerCreate returned NULL"); return 0; }
    int vendor = 0x057e;
    CFNumberRef number = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &vendor);
    const void *key = CFSTR(kIOHIDVendorIDKey), *value = number;
    CFDictionaryRef matching = CFDictionaryCreate(kCFAllocatorDefault, &key, &value, 1,
                                                 &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    IOHIDManagerSetDeviceMatching(manager, matching);
    CFSetRef set = IOHIDManagerCopyDevices(manager);
    CFIndex count = set ? CFSetGetCount(set) : 0;
    const void **devices = count ? calloc((size_t)count, sizeof(*devices)) : NULL;
    unsigned long signature = 0;
    int opened = 0, target_pid = 0;
    if (count && !devices) {
        if (verbose) result(L_HID, PROBE_FAIL, "allocation failed for HID enumeration");
        count = 0;
    }
    if (devices) CFSetGetValues(set, devices);
    for (CFIndex i = 0; i < count; ++i)
        if (hid_number((IOHIDDeviceRef)devices[i], CFSTR(kIOHIDProductIDKey)) == 0x2069) target_pid = 0x2069;
    for (CFIndex i = 0; i < count; ++i) {
        IOHIDDeviceRef d = (IOHIDDeviceRef)devices[i];
        long pid = hid_number(d, CFSTR(kIOHIDProductIDKey));
        long location = hid_number(d, CFSTR(kIOHIDLocationIDKey));
        signature += mix((unsigned long)pid, (unsigned long)location);
        if (!verbose) continue;
        char product[180], transport[80];
        CFTypeRef desc = IOHIDDeviceGetProperty(d, CFSTR(kIOHIDReportDescriptorKey));
        CFIndex len = desc && CFGetTypeID(desc) == CFDataGetTypeID() ? CFDataGetLength((CFDataRef)desc) : 0;
        emit("IOHID 057e:%04lx product=%s usagePage=0x%lx usage=0x%lx location=0x%lx transport=%s descriptor=%ld bytes maxInput=%ld maxOutput=%ld\n",
             pid, hid_string(d, CFSTR(kIOHIDProductKey), product, sizeof(product)),
             hid_number(d, CFSTR(kIOHIDPrimaryUsagePageKey)), hid_number(d, CFSTR(kIOHIDPrimaryUsageKey)), location,
             hid_string(d, CFSTR(kIOHIDTransportKey), transport, sizeof(transport)), (long)len,
             hid_number(d, CFSTR(kIOHIDMaxInputReportSizeKey)), hid_number(d, CFSTR(kIOHIDMaxOutputReportSizeKey)));
        if (len) {
            const UInt8 *bytes = CFDataGetBytePtr((CFDataRef)desc);
            for (CFIndex offset = 0; offset < len && !detail_capped; offset += 16) {
                char hex[48];
                probe_hex(hex, sizeof(hex), bytes + offset, (size_t)(len-offset > 16 ? 16 : len-offset));
                emit("  descriptor %04lx: %s\n", (long)offset, hex);
            }
        } else emit("FAIL IOHID report descriptor: missing/empty property\n");
        IOReturn rc = IOHIDDeviceOpen(d, kIOHIDOptionsTypeNone);
        emit("%s IOHIDDeviceOpen 057e:%04lx = 0x%08x\n", rc == kIOReturnSuccess ? "PASS" : "FAIL", pid, (unsigned)rc);
        if (rc == kIOReturnSuccess) {
            IOReturn close_rc = IOHIDDeviceClose(d, kIOHIDOptionsTypeNone);
            emit("%s IOHIDDeviceClose = 0x%08x\n", close_rc == kIOReturnSuccess ? "PASS" : "FAIL", (unsigned)close_rc);
            if ((!target_pid || pid == target_pid) && close_rc == kIOReturnSuccess) ++opened;
        }
    }
    if (verbose) {
        hid_seen = (int)count;
        switch2_hid = target_pid != 0;
        result(L_HID, !count ? (usb_seen ? PROBE_FAIL : PROBE_SKIP) : opened ? PROBE_PASS : PROBE_FAIL,
               "%ld Nintendo HID devices; %d target opens and closes succeeded", (long)count, opened);
    }
    free(devices);
    if (set) CFRelease(set);
    CFRelease(matching);
    CFRelease(number);
    CFRelease(manager);
    return signature;
}

static void print_hints(void)
{
    SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1");
    const char *names[] = {SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, SDL_HINT_JOYSTICK_HIDAPI,
        SDL_HINT_JOYSTICK_HIDAPI_SWITCH2, SDL_HINT_JOYSTICK_HIDAPI_SWITCH, SDL_HINT_JOYSTICK_MFI,
        "SDL_GAMECONTROLLER_USE_BUTTON_LABELS", "SDL_GAMECONTROLLERCONFIG",
        "SDL_JOYSTICK_HIDAPI_LIBUSB", "SDL_HIDAPI_LIBUSB"};
    emit("Hints: native/app has no joystick-specific SDL_SetHint calls; foreground game mode mirrored.\n");
    for (size_t i = 0; i < sizeof(names)/sizeof(names[0]); ++i)
        emit("hint %s=%s\n", names[i], SDL_GetHint(names[i]) ? SDL_GetHint(names[i]) : "(unset; SDL default)");
    emit("DYLD_FALLBACK_LIBRARY_PATH=%s\n", safe(getenv("DYLD_FALLBACK_LIBRARY_PATH")));
}
static int init_sdl(void)
{
    int ok = SDL_Init(SDL_INIT_GAMEPAD | SDL_INIT_EVENTS);
    emit("%s SDL init GAMEPAD|EVENTS: %s\n", ok ? "PASS" : "FAIL", ok ? "initialized" : SDL_GetError());
    emit("SDL phase dyld libusb image (own dlopen already closed): %s\n", usb_image());
    if (!ok) result(L_DRIVER, PROBE_FAIL, "SDL_Init: %s", SDL_GetError());
    return ok;
}
static void describe_joystick(SDL_JoystickID id, const char *source)
{
    char guid[33];
    SDL_GUIDToString(SDL_GetJoystickGUIDForID(id), guid, sizeof(guid));
    emit("%s id=%u name=%s VID/PID=%04x:%04x guid=%s joystickType=%d gamepadType=%d isGamepad=%d path=%s\n",
         source, id, safe(SDL_GetJoystickNameForID(id)), SDL_GetJoystickVendorForID(id), SDL_GetJoystickProductForID(id),
         guid, SDL_GetJoystickTypeForID(id), SDL_GetGamepadTypeForID(id), SDL_IsGamepad(id), safe(SDL_GetJoystickPathForID(id)));
    SDL_Joystick *joy = SDL_OpenJoystick(id);
    if (joy) {
        emit("PASS SDL joystick open id=%u buttons=%d axes=%d hats=%d\n", id,
             SDL_GetNumJoystickButtons(joy), SDL_GetNumJoystickAxes(joy), SDL_GetNumJoystickHats(joy));
        SDL_CloseJoystick(joy);
    } else emit("FAIL SDL joystick open id=%u: %s\n", id, SDL_GetError());
}
static void select_gamepad(void)
{
    int count = 0;
    SDL_JoystickID *ids = SDL_GetJoysticks(&count);
    SDL_JoystickID candidate = 0;
    for (int i = 0; ids && i < count; ++i) {
        joystick_seen = 1;
        if (!SDL_IsGamepad(ids[i])) continue;
        if (!candidate) candidate = ids[i];
        if (SDL_GetJoystickVendorForID(ids[i]) == 0x057e && SDL_GetJoystickProductForID(ids[i]) == 0x2069) { candidate = ids[i]; break; }
    }
    if (!candidate) { SDL_free(ids); return; }
    /* Do not use another controller to qualify a visible Switch 2. */
    if ((switch2_usb || switch2_hid) && !(SDL_GetJoystickVendorForID(candidate) == 0x057e && SDL_GetJoystickProductForID(candidate) == 0x2069)) { SDL_free(ids); return; }
    gamepad = SDL_OpenGamepad(candidate);
    if (!gamepad) { result(L_MAPPING, PROBE_FAIL, "SDL mapping/open id=%u: %s", candidate, SDL_GetError()); SDL_free(ids); return; }
    selected = candidate;
    selected_switch2 = SDL_GetGamepadVendor(gamepad) == 0x057e && SDL_GetGamepadProduct(gamepad) == 0x2069;
    describe_joystick(candidate, "Selected SDL gamepad");
    SDL_Joystick *joy = SDL_GetGamepadJoystick(gamepad);
    emit("PASS SDL mapping/open: connectionState=%d buttons=%d axes=%d hats=%d rumbleCapable=%d\n",
         SDL_GetGamepadConnectionState(gamepad), SDL_GetNumJoystickButtons(joy), SDL_GetNumJoystickAxes(joy),
         SDL_GetNumJoystickHats(joy), SDL_GetBooleanProperty(SDL_GetGamepadProperties(gamepad), SDL_PROP_GAMEPAD_CAP_RUMBLE_BOOLEAN, false));
    char *mapping = SDL_GetGamepadMapping(gamepad);
    emit("SDL mapping: %.1400s\n", safe(mapping));
    SDL_free(mapping);
    for (int sensor = SDL_SENSOR_ACCEL; sensor <= SDL_SENSOR_GYRO_R; ++sensor)
        emit("%s SDL sensor type=%d available=%d\n", SDL_GamepadHasSensor(gamepad, (SDL_SensorType)sensor) ? "PASS" : "SKIP", sensor,
             SDL_GamepadHasSensor(gamepad, (SDL_SensorType)sensor));
    result(L_DRIVER, PROBE_PASS, "SDL joystick/gamepad %04x:%04x opened; see INPUT logs for HIDAPI driver name", SDL_GetGamepadVendor(gamepad), SDL_GetGamepadProduct(gamepad));
    result(L_MAPPING, PROBE_PASS, "SDL gamepad mapping available; guided physical controls pending");
    result(L_EVENTS, PROBE_FAIL, "opened, awaiting physical button/axis events");
    SDL_free(ids);
}
static void discovery_events(void)
{
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_EVENT_JOYSTICK_ADDED) {
            joystick_seen = 1;
            describe_joystick(e.jdevice.which, "SDL_EVENT_JOYSTICK_ADDED");
            if (!(switch2_usb || switch2_hid) || (SDL_GetJoystickVendorForID(e.jdevice.which) == 0x057e &&
                SDL_GetJoystickProductForID(e.jdevice.which) == 0x2069))
                result(L_DRIVER, PROBE_PASS, "target joystick added; mapping/open pending");
        } else if (e.type == SDL_EVENT_GAMEPAD_ADDED) {
            emit("PASS SDL_EVENT_GAMEPAD_ADDED id=%u\n", e.gdevice.which);
        }
    }
}
static int connected(void) { return gamepad && SDL_GamepadConnected(gamepad); }
static void note_input(void)
{
    if (!any_input) result(L_EVENTS, PROBE_PASS, "selected gamepad delivered physical button/axis events");
    any_input = 1;
}
static int input_event(const SDL_Event *e, int *button, int *axis, int *value)
{
    *button = *axis = -1;
    *value = 0;
    if (e->type == SDL_EVENT_GAMEPAD_BUTTON_DOWN && e->gbutton.which == selected) {
        *button = e->gbutton.button;
        note_input();
        return 1;
    }
    if (e->type == SDL_EVENT_GAMEPAD_AXIS_MOTION && e->gaxis.which == selected &&
        (e->gaxis.value > 16383 || e->gaxis.value < -16383)) {
        *axis = e->gaxis.axis;
        *value = e->gaxis.value;
        note_input();
        return 1;
    }
    return 0;
}
static void guided_controls(void)
{
    int extras_pass = 0, extras_fail = 0, controls_fail = 0, controls_skip = 0;
    emit("Guided input: press ONLY the named control, then release/center. No Enter needed; 6 s auto timeout.\n");
    if (!selected_switch2) emit("Generic controller: A/B/X/Y prompts use Nintendo physical positions; C/GL/GR are SKIP.\n");
    for (size_t i = 0; i < probe_control_count; ++i) {
        const ProbeControl *c = &probe_controls[i];
        if (c->switch2_only && !selected_switch2) { emit("SKIP control %s: not Switch 2 Pro\n", c->label); continue; }
        /* Preserve six seconds for sensor/rumble checks, within the total limit. */
        if (!connected() || SDL_GetTicks() + 6000 >= deadline) {
            emit("SKIP control %s: %s\n", c->label, connected() ? "total time budget; increase -t" : "gamepad disconnected");
            ++controls_skip;
            continue;
        }
        /* Consume the release of the previous control. Stale motion cannot pass
         * the next prompt; each axis must return below 25% before crossing 50%. */
        SDL_Event e;
        while (SDL_PollEvent(&e)) { }
        int armed = c->button >= 0 || abs(SDL_GetGamepadAxis(gamepad, (SDL_GamepadAxis)c->axis)) < 8192;
        emit("NOW %s -> expected %s id=%d (up to 6 s)\n", c->label,
             c->button >= 0 ? probe_button_name(c->button) : probe_axis_name(c->axis), c->button >= 0 ? c->button : c->axis);
        Uint64 until = SDL_GetTicks() + 6000;
        if (until > deadline - 6000) until = deadline - 6000;
        int matched = 0, unexpected = 0, shown = 0;
        char observed[128] = "none";
        char first_unexpected[128] = "none";
        while (SDL_GetTicks() < until && connected() && !matched) {
            while (SDL_PollEvent(&e)) {
                if (c->button < 0 && e.type == SDL_EVENT_GAMEPAD_AXIS_MOTION && e.gaxis.which == selected &&
                    e.gaxis.axis == c->axis && abs(e.gaxis.value) < 8192) armed = 1;
                int button, axis, value;
                if (!input_event(&e, &button, &axis, &value)) continue;
                if (c->button < 0 && !armed && axis == c->axis) continue;
                snprintf(observed, sizeof(observed), "%s id=%d value=%d", button >= 0 ? probe_button_name(button) : probe_axis_name(axis), button >= 0 ? button : axis, value);
                if (shown++ < 3) emit("  arrived SDL %s %s\n", button >= 0 ? "button" : "axis", observed);
                if (probe_control_matches(c, button, axis, value)) { matched = 1; break; }
                if (button >= 0) {
                    if (!unexpected) snprintf(first_unexpected, sizeof(first_unexpected), "%s", observed);
                    unexpected = 1;
                }
            }
            SDL_Delay(5);
        }
        emit("%s control %s: %s%s; last=%s\n", matched && !unexpected ? "PASS" : "FAIL", c->label,
             matched ? "expected input arrived" : connected() ? "TIMEOUT" : "disconnected",
             unexpected ? "; UNEXPECTED SDL BUTTON (check physical label/mapping)" : "", observed);
        if (unexpected) emit("FAIL SDL mapping for %s: unexpected %s\n", c->label, first_unexpected);
        if (!matched || unexpected) ++controls_fail;
        if (c->extra) { if (matched && !unexpected) ++extras_pass; else ++extras_fail; }
    }
    emit("%s guided controls: %d failed/timeouts, %d skipped; inspect every control row\n", controls_fail ? "FAIL" : controls_skip ? "SKIP" : "PASS", controls_fail, controls_skip);
    result(L_MAPPING, controls_fail ? PROBE_FAIL : controls_skip ? PROBE_SKIP : PROBE_PASS,
           "guided physical controls: %d failed/timeouts, %d skipped", controls_fail, controls_skip);
    result(L_EXTRAS, !selected_switch2 ? PROBE_SKIP : extras_fail ? PROBE_FAIL : extras_pass == 4 ? PROBE_PASS : PROBE_SKIP,
           "Capture=MISC1 C=MISC2 GL=LEFT_PADDLE1 GR=RIGHT_PADDLE1: %d passed, %d failed; Switch2=%d", extras_pass, extras_fail, selected_switch2);
}

static void sensors_and_rumble(void)
{
    if (!connected()) return;
    SDL_SensorType types[] = {SDL_SENSOR_ACCEL, SDL_SENSOR_GYRO};
    int enabled[2] = {0}, samples[2][2] = {{0}};
    double energy[2][2] = {{0}};
    float previous[2][3] = {{0}};
    for (int i = 0; i < 2; ++i) {
        int has = SDL_GamepadHasSensor(gamepad, types[i]);
        enabled[i] = has && SDL_SetGamepadSensorEnabled(gamepad, types[i], true);
        emit("%s sensors %s enable=%d advertisedRate=%.2f Hz: %s\n", enabled[i] ? "PASS" : "FAIL", i ? "gyro" : "accel",
             enabled[i], has ? SDL_GetGamepadSensorDataRate(gamepad, types[i]) : 0, enabled[i] ? "enabled" : has ? SDL_GetError() : "unavailable");
    }
    if ((enabled[0] || enabled[1]) && SDL_GetTicks() + 5100 < deadline) {
        for (int phase = 0; phase < 2; ++phase) {
            emit("NOW sensors: %s\n", phase ? "shake it for 3 s" : "hold still for 1.5 s");
            Uint64 begin = SDL_GetTicks(), until = begin + (phase ? 3000 : 1500);
            while (SDL_GetTicks() < until && connected()) {
                SDL_Event e;
                while (SDL_PollEvent(&e)) {
                    if (e.type != SDL_EVENT_GAMEPAD_SENSOR_UPDATE || e.gsensor.which != selected) continue;
                    for (int i = 0; i < 2; ++i) if (enabled[i] && e.gsensor.sensor == types[i]) {
                        if (samples[i][phase]) for (int k = 0; k < 3; ++k) {
                            double delta = e.gsensor.data[k] - previous[i][k];
                            energy[i][phase] += delta * delta;
                        }
                        memcpy(previous[i], e.gsensor.data, sizeof(previous[i]));
                        ++samples[i][phase];
                    }
                }
                SDL_Delay(5);
            }
            double seconds = (SDL_GetTicks() - begin) / 1000.0;
            for (int i = 0; i < 2; ++i) emit("sensors %s %s samples=%d observedRate=%.2f Hz meanDeltaSquared=%.6f\n",
                i ? "gyro" : "accel", phase ? "moving" : "still", samples[i][phase], seconds > 0 ? samples[i][phase] / seconds : 0,
                samples[i][phase] > 1 ? energy[i][phase] / (samples[i][phase]-1) : 0);
        }
        int passed = enabled[0] && enabled[1];
        for (int i = 0; i < 2; ++i) {
            double still = samples[i][0] > 1 ? energy[i][0] / (samples[i][0]-1) : 0;
            double moving = samples[i][1] > 1 ? energy[i][1] / (samples[i][1]-1) : 0;
            passed = passed && samples[i][0] > 1 && samples[i][1] > 1 && moving > still * 2 + 0.000001;
        }
        result(L_SENSORS, passed ? PROBE_PASS : PROBE_FAIL, "accel+gyro events and moving variation >2x still + epsilon required; see rates (repeat if not shaken)");
    } else result(L_SENSORS, !enabled[0] || !enabled[1] ? PROBE_FAIL : PROBE_SKIP,
                  "sensors unavailable or insufficient total time for still/moving comparison");
    for (int i = 0; i < 2; ++i) if (enabled[i]) SDL_SetGamepadSensorEnabled(gamepad, types[i], false);
    if (!connected() || SDL_GetTicks() + 500 > deadline) { result(L_RUMBLE, PROBE_SKIP, "disconnected or less than 0.5 s remaining"); return; }
    bool ok = SDL_RumbleGamepad(gamepad, 0x7000, 0x7000, 500);
    result(L_RUMBLE, ok ? PROBE_PASS : PROBE_FAIL, "SDL_RumbleGamepad(0.5 s) returned %s; %s; person must note felt/not felt",
           ok ? "true" : "false", ok ? "API accepted" : SDL_GetError());
    SDL_Delay(500);
    SDL_RumbleGamepad(gamepad, 0, 0, 0);
}

int main(int argc, char **argv)
{
    double seconds = 60;
    int all = 0;
    for (int i = 1; i < argc; ++i) {
        if (!strcmp(argv[i], "-a")) all = 1;
        else if (!strcmp(argv[i], "-t") && i+1 < argc) {
            char *end;
            seconds = strtod(argv[++i], &end);
            if (*end || !(seconds >= 1 && seconds <= 3600)) { fprintf(stderr, "-t must be 1..3600 seconds\n"); return 2; }
        } else { fprintf(stderr, "Usage: %s [-t SECONDS (1..3600, default 60)] [-a (list all USB vendors)]\n", argv[0]); return 2; }
    }
    setvbuf(stdout, NULL, _IONBF, 0);
    report = fopen("padprobe-report.txt", "w");
    if (!report) { perror("padprobe-report.txt (run from a writable directory)"); return 2; }
    deadline = SDL_GetTicks() + (Uint64)(seconds * 1000);
    for (int i = 0; i < L_COUNT; ++i) snprintf(layers[i].reason, sizeof(layers[i].reason), "prerequisite absent or total time budget exhausted");
    layers[L_LOAD].status = layers[L_USB_ENUM].status = layers[L_DRIVER].status = PROBE_FAIL;
    struct utsname machine;
    uname(&machine);
    char os[80] = "unknown", build[80] = "unknown", date[80];
    size_t size = sizeof(os); sysctlbyname("kern.osproductversion", os, &size, NULL, 0);
    size = sizeof(build); sysctlbyname("kern.osversion", build, &size, NULL, 0);
    time_t now = time(NULL); struct tm local; localtime_r(&now, &local);
    strftime(date, sizeof(date), "%Y-%m-%d %H:%M:%S %z", &local);
    int version = SDL_GetVersion();
    emit("PadProbe git=%s date=%s macOS=%s build=%s arch=%s\n", PADPROBE_GIT_ID, date, os, build, machine.machine);
    emit("SDL=%d.%d.%d revision=%s totalLimit=%.1f s allUSB=%d\n", SDL_VERSIONNUM_MAJOR(version), SDL_VERSIONNUM_MINOR(version), SDL_VERSIONNUM_MICRO(version), SDL_GetRevision(), seconds, all);
    SDL_SetLogOutputFunction(capture_log, NULL);
    SDL_SetLogPriority(SDL_LOG_CATEGORY_INPUT, SDL_LOG_PRIORITY_DEBUG);
    print_hints();
    emit("USB claims/IOHID opens run with SDL shut down; handles released before SDL init.\n");
    unsigned long usb_sig = usb_snapshot(1, all), hid_sig = hid_snapshot(1);
    detail_limit = 13000;
    detail_capped = 0;
    int sdl_ok = init_sdl();
    emit("Plug the pad in now (USB data cable); leave other gamepads disconnected. Waiting up to %.1f s total.\n", seconds);
    Uint64 next_scan = SDL_GetTicks() + 500, next_prompt = SDL_GetTicks() + 5000;
    while (SDL_GetTicks() < deadline && !gamepad) {
        if (SDL_GetTicks() >= next_scan) {
            unsigned long usb_now = usb_snapshot(0, 0), hid_now = hid_snapshot(0);
            if (usb_now != usb_sig || hid_now != hid_sig) {
                emit("PASS hotplug: Nintendo USB/HID inventory changed; releasing SDL before independent diagnostics\n");
                SDL_Quit();
                usb_sig = usb_snapshot(1, all);
                hid_sig = hid_snapshot(1);
                sdl_ok = init_sdl();
            }
            next_scan = SDL_GetTicks() + 500;
        }
        if (sdl_ok) { discovery_events(); select_gamepad(); }
        if (SDL_GetTicks() >= next_prompt && !gamepad) {
            Uint64 now_ticks = SDL_GetTicks();
            emit("SKIP waiting: plug the pad in now; %.1f s remaining\n", (now_ticks < deadline ? deadline - now_ticks : 0) / 1000.0);
            next_prompt += 5000;
        }
        SDL_Delay(10);
    }
    if (gamepad) {
        detail_limit = 26000;
        detail_capped = 0;
        if (!selected_switch2 && !switch2_usb) {
            result(L_USB_ENUM, PROBE_SKIP, "other gamepad selected; Nintendo USB qualification not applicable");
            result(L_USB_OPEN, PROBE_SKIP, "other gamepad selected; no non-Nintendo devices opened");
            result(L_BULK, PROBE_SKIP, "not Switch 2 Pro");
            result(L_HID, PROBE_SKIP, "other gamepad selected; independent IOHID check filters Nintendo");
        }
        guided_controls();
        sensors_and_rumble();
    } else {
        emit("SKIP SDL mapping/guided input/sensors/rumble: no SDL gamepad opened before deadline\n");
        if (joystick_seen) result(L_MAPPING, PROBE_FAIL, "joystick appeared but no gamepad mapping/open succeeded");
        if (joystick_seen && !usb_seen && !hid_seen) {
            result(L_USB_ENUM, PROBE_SKIP, "other joystick selected; Nintendo USB qualification not applicable");
            result(L_USB_OPEN, PROBE_SKIP, "no Nintendo USB target; inspect SDL mapping failure");
        }
        if (!joystick_seen) result(L_DRIVER, PROBE_FAIL, "no SDL joystick/gamepad appeared before deadline");
    }
    if (gamepad) SDL_CloseGamepad(gamepad);
    SDL_Quit();
    SDL_SetLogOutputFunction(NULL, NULL);
    summary_mode = 1;
    emit("\nFINAL SUMMARY (target Switch 2 057e:2069 preferred; diagnostics do not certify gameplay)\n");
    for (int i = 0; i < L_COUNT; ++i) emit("%s %s: %s\n", probe_status_name(layers[i].status), probe_layer_names[i], layers[i].reason);
    emit("VERDICT: %s\n", probe_verdict(layers, usb_enumeration_error ? -1 : usb_seen || hid_seen || joystick_seen, gamepad != NULL, selected_switch2));
    char cwd[4096];
    emit("Send me padprobe-report.txt\nReport path: %.1100s/padprobe-report.txt\n", getcwd(cwd, sizeof(cwd)) ? cwd : ".");
    if (fclose(report)) report_error = 1;
    if (report_error) { fprintf(stderr, "FAIL report write: padprobe-report.txt may be incomplete\n"); return 2; }
    return 0; /* Diagnostic failures are in the report, not the process status. */
}
