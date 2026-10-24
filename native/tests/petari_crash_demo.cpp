// Synthetic release crash verification: links the production handler and the
// same opt-in three-function chain as the app. No disc or game data required.
#include "petari/platform/crash.hpp"
#include "petari/platform/build_identity.hpp"
#include "test_crash.hpp"
#include <cstdio>
#include <cstring>
#include <thread>
#include <chrono>
#include <pthread.h>

int main(int argc, char** argv) {
    namespace Identity = PetariNative::Platform::BuildIdentity;
    if (argc == 2 && std::strcmp(argv[1], "--identity") == 0) {
        Identity::initialize();
        const auto& i = Identity::get();
        std::printf("%s\n%s\n%s\n%s\n%s\n%s\n", i.executableUUID, i.appVersion, i.sourceCommit,
                    i.bundleExecutableUUID, i.macos, i.model);
        return 0;
    }
    if (argc < 2) return 2;
    pthread_setname_np("Petari crash demo");
    if (argc < 3 || std::strcmp(argv[2], "--no-handler") != 0)
        PetariNative::Platform::Crash::install(argv[1]);
    for (;;) {
        PetariNative::App::TestCrash::frame();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}
