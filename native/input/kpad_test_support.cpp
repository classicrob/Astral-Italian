// Standalone tests reconstruct WPAD/KPAD many times in one process. SMG2's
// real SDK initializes KPAD once per process; include that unchanged source in
// this test-only translation unit so its private initialization guard can be
// reset together with the host. The app builds the original SDK TU directly.
#include <cstring>
#include PETARI_KPAD_TEST_SOURCE

namespace PetariNative::Input::Detail {
void resetKpadForTesting() {
    kpadInitialized = FALSE;
}
}  // namespace PetariNative::Input::Detail
