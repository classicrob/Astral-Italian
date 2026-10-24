#include "petari/platform/build_identity.hpp"
#include <mach-o/dyld.h>
#include <cassert>
#include <cstring>

int main() {
    namespace I = PetariNative::Platform::BuildIdentity;
    I::initialize();
    const auto& identity = I::get();
    assert(std::strlen(identity.executableUUID) == 36);
    assert(identity.macos[0] && identity.model[0]);
    assert(!identity.appVersion[0] && !identity.sourceCommit[0] && !identity.bundleExecutableUUID[0]);
    struct { mach_header_64 header; uuid_command uuid; } image{};
    image.header.magic = MH_MAGIC_64;
    image.header.ncmds = 1;
    image.header.sizeofcmds = sizeof(uuid_command);
    image.uuid.cmd = LC_UUID;
    image.uuid.cmdsize = sizeof(uuid_command);
    for (unsigned i = 0; i < 16; ++i) image.uuid.uuid[i] = i;
    char uuid[37];
    I::imageUUID(reinterpret_cast<mach_header*>(&image), uuid);
    assert(std::strcmp(uuid, "00010203-0405-0607-0809-0A0B0C0D0E0F") == 0);
    image.uuid.cmdsize = 0;
    I::imageUUID(reinterpret_cast<mach_header*>(&image), uuid);
    assert(!uuid[0]);
    I::imageUUID(nullptr, uuid);
    assert(!uuid[0]);
}
