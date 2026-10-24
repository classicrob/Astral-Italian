#pragma once

namespace PetariNative::App::TestCrash {
// Test-only opt-in: PETARI_TEST_CRASH=null-call|segv-write|abort. Call at the
// first-frame seam; fires once after three seconds of wall time. Unset: inert.
void frame();
// Shared by the release verification demo; callers must supply a valid kind.
void trigger(const char* kind);
}  // namespace PetariNative::App::TestCrash
