#pragma once
// Trace-on only. Callers must enter through PETARI_GPU_TRACE/invokeEnabled.
// No alternative CV, timeout, polling or scheduler policy is introduced.
#include <condition_variable>
#include <cstdint>
#include <mutex>
namespace PetariNative::Platform::OS::InterruptTrace {
void request(const void* mutex, const char* reason);
void acquired(const void* mutex, const char* reason);
void failed(const void* mutex, const char* reason);
void releaseBegin(const void* mutex, const char* reason);
void released(const void* mutex, const char* reason);
void releasePending(const void* mutex, const char* reason);
void cvReleasePending(const void* mutex, const void* cv, const char* reason);
void cvReacquired(const void* mutex, const void* cv, const char* reason);
void predicate(const void* mutex, const void* cv, bool ready);
void notify(const void* mutex, const void* cv, const void* recipient, const char* reason, bool returned);
// Each predicate invocation after the first follows the ORIGINAL CV wait's
// mutex reacquisition. It is not a kernel-wake timestamp. A false predicate
// marks an impending, not yet completed, implicit unlock.
template<class Predicate>
void wait(std::condition_variable& cv, std::unique_lock<std::mutex>& lock, Predicate predicateFn) {
    bool initial = true;
    cv.wait(lock, [&] {
        if (!initial) cvReacquired(lock.mutex(), &cv, "cv-predicate-reacquired");
        const bool ready = predicateFn();
        predicate(lock.mutex(), &cv, ready);
        if (!ready) cvReleasePending(lock.mutex(), &cv, "cv-predicate-false-release-pending");
        initial = false;
        return ready;
    });
}
}
