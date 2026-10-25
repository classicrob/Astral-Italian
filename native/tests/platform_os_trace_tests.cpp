// Diagnostic adapter, not a replacement CTest or a changed latency fixture.
// The original main/workload/assertions compile verbatim in this translation
// unit; only trace configuration occurs before it initializes OS threads.
#include <petari/gpu_trace.hpp>
#include <cstdlib>
#include <cstdio>
#define main petari_original_platform_os_main
#include "platform_os_tests.cpp"
#undef main
int main(int argc, char** argv) {
    if (!::PetariNative::GpuTrace::configure(std::getenv("PETARI_PIPELINE_DIAG"),
            std::getenv("PETARI_GPU_TRACE"), std::getenv("PETARI_GPU_TRACE_BINDING_SHA256"))) {
        std::fprintf(stderr, "OS trace adapter requires explicit DIAG/output/binding; original fixture not executed\n");
        return 86;
    }
    const int result = petari_original_platform_os_main(argc, argv);
    ::PetariNative::GpuTrace::shutdown();
    return result; // Original check failures call exit(1); configure's atexit flushes them too.
}
