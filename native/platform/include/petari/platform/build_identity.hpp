#pragma once

#include <mach-o/loader.h>

namespace PetariNative::Platform::BuildIdentity {

// Populated before installing signal handlers; reads never allocate.
struct Identity {
    char appVersion[128];
    char sourceCommit[128];
    char executableUUID[37];
    char bundleExecutableUUID[128];
    char macos[128];
    char model[128];
};
void initialize();
const Identity& get();
void imageUUID(const mach_header* header, char (&uuid)[37]);

}  // namespace PetariNative::Platform::BuildIdentity
