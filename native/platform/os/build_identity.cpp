#include "petari/platform/build_identity.hpp"

#include <CoreFoundation/CoreFoundation.h>
#include <mach-o/dyld.h>
#include <sys/sysctl.h>
#include <cstdio>
#include <cstring>

namespace PetariNative::Platform::BuildIdentity {
namespace {
Identity gIdentity{};

void bundleString(CFBundleRef bundle, CFStringRef key, char* output, size_t size) {
    CFTypeRef value = bundle ? CFBundleGetValueForInfoDictionaryKey(bundle, key) : nullptr;
    if (value && CFGetTypeID(value) == CFStringGetTypeID() &&
        !CFStringGetCString(static_cast<CFStringRef>(value), output, size, kCFStringEncodingUTF8)) {
        output[0] = 0;
    }
}
void systemString(const char* key, char* output, size_t size) {
    if (sysctlbyname(key, output, &size, nullptr, 0) != 0) output[0] = 0;
}
}  // namespace

void imageUUID(const mach_header* header, char (&uuid)[37]) {
    uuid[0] = 0;
    if (!header || header->magic != MH_MAGIC_64) return;
    const auto* h = reinterpret_cast<const mach_header_64*>(header);
    const auto* command = reinterpret_cast<const load_command*>(h + 1);
    const auto* end = reinterpret_cast<const char*>(command) + h->sizeofcmds;
    for (uint32_t i = 0; i < h->ncmds; ++i) {
        if (reinterpret_cast<const char*>(command) + sizeof(load_command) > end ||
            command->cmdsize < sizeof(load_command) || reinterpret_cast<const char*>(command) + command->cmdsize > end) return;
        if (command->cmd == LC_UUID && command->cmdsize >= sizeof(uuid_command)) {
            const auto* bytes = reinterpret_cast<const uuid_command*>(command)->uuid;
            unsigned n = 0;
            for (unsigned b = 0; b < 16; ++b) {
                if (b == 4 || b == 6 || b == 8 || b == 10) uuid[n++] = '-';
                uuid[n++] = "0123456789ABCDEF"[bytes[b] >> 4];
                uuid[n++] = "0123456789ABCDEF"[bytes[b] & 15];
            }
            uuid[n] = 0;
            return;
        }
        command = reinterpret_cast<const load_command*>(reinterpret_cast<const char*>(command) + command->cmdsize);
    }
}

void initialize() {
    static bool initialized = false;
    if (initialized) return;
    initialized = true;
    gIdentity = {};
    imageUUID(_dyld_get_image_header(0), gIdentity.executableUUID);
    const auto bundle = CFBundleGetMainBundle();
    bundleString(bundle, CFSTR("CFBundleShortVersionString"), gIdentity.appVersion, sizeof(gIdentity.appVersion));
    bundleString(bundle, CFSTR("AstralSourceCommit"), gIdentity.sourceCommit, sizeof(gIdentity.sourceCommit));
    bundleString(bundle, CFSTR("AstralExecutableUUID"), gIdentity.bundleExecutableUUID, sizeof(gIdentity.bundleExecutableUUID));
    char version[64]{}, build[32]{};
    systemString("kern.osproductversion", version, sizeof(version));
    systemString("kern.osversion", build, sizeof(build));
    if (version[0] && build[0]) std::snprintf(gIdentity.macos, sizeof(gIdentity.macos), "%s (%s)", version, build);
    else if (version[0]) std::snprintf(gIdentity.macos, sizeof(gIdentity.macos), "%s", version);
    systemString("hw.model", gIdentity.model, sizeof(gIdentity.model));
}
const Identity& get() { return gIdentity; }
}  // namespace PetariNative::Platform::BuildIdentity
