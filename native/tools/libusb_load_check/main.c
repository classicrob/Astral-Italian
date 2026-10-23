#include <SDL3/SDL.h>
#include <SDL3/SDL_hidapi.h>
#include <mach-o/dyld.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv)
{
    const char *loaded = NULL;
    const char *suffix = "/libusb-1.0.0.dylib";
    size_t count = 0;
    struct SDL_hid_device_info *devices, *device;
    int expect_loaded = argc == 2 && strcmp(argv[1], "--expect-loaded") == 0;
    if (!SDL_Init(0)) {
        fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
        return 1;
    }
    int status = SDL_hid_init();
    printf("SDL_hid_init=%d\n", status);
    if (status != 0) {
        SDL_Quit();
        return 1;
    }
    devices = SDL_hid_enumerate(0, 0);
    for (device = devices; device; device = device->next) {
        ++count;
    }
    printf("SDL_hid_enumerate: %zu devices\n", count);
    for (uint32_t i = 0; i < _dyld_image_count(); ++i) {
        const char *name = _dyld_get_image_name(i);
        size_t length = strlen(name), suffix_length = strlen(suffix);
        if (length >= suffix_length && strcmp(name + length - suffix_length, suffix) == 0) {
            loaded = name;
            printf("libusb loaded from: %s\n", name);
        }
    }
    if (!loaded) {
        printf("libusb not loaded; HID enumeration completed without libusb\n");
    }
    int passed = expect_loaded ? loaded && strstr(loaded, "/Contents/Frameworks/") : !loaded;
    SDL_hid_free_enumeration(devices);
    SDL_hid_exit();
    SDL_Quit();
    printf("result: %s\n", passed ? "PASS" : "FAIL");
    return passed ? 0 : 1;
}
