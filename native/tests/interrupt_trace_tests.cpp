// Server-only until PORT assigns a measurement slot. No latency predicate changes.
#include "interrupt_trace.hpp"
#include "interrupt_trace_state.hpp"
#include <petari/gpu_trace.hpp>
#include <cassert>
#include <fstream>
#include <iterator>
#include <string>
#include <thread>
#include <unistd.h>
namespace IT = PetariNative::Platform::OS::InterruptTrace;
namespace GT = PetariNative::GpuTrace;
extern "C" void petari_interrupt_trace_off_probe(int* pointer) {
    PETARI_GPU_TRACE(++*pointer; IT::request(pointer, "probe");
        IT::acquired(pointer, "probe"); IT::releaseBegin(pointer, "probe"); IT::released(pointer, "probe"));
}
int main() {
    assert(!GT::enabled.load() && GT::run == nullptr);
    petari_interrupt_trace_off_probe(nullptr); // No trace TLS, allocation or pointer access when disabled.
    assert(GT::run == nullptr && GT::threadId == 0 && GT::producerSequence == 0);
    IT::State s;
    assert(!s.acquired(10) && !s.failed() && !s.released());
    assert(s.request(1, 10) && s.pending && !s.held);
    assert(!s.request(2, 11) && s.failed() && !s.pending);
    assert(s.request(3, 20) && s.acquired(30) && s.heldNs(50) == 20);
    assert(!s.request(4, 31) && !s.cvAcquire(4, 31));
    assert(s.cvRelease() && s.cvUnknown && !s.held && s.heldNs(80) == 0);
    assert(!s.released() && !s.request(5, 81));
    assert(s.cvAcquire(6, 90) && s.span == 6 && s.requestNs == 0 && s.heldNs(100) == 10);
    assert(s.cvRelease() && s.cvAcquire(7, 110)); // spurious/false predicate repeats same actual CV wait
    assert(s.releaseBegin() && s.released() && !s.held && !s.cvUnknown);
    char temporary[] = "/tmp/petari-interrupt-trace-XXXXXX";
    assert(mkdtemp(temporary));
    const std::string path = std::string(temporary) + "/trace.jsonl";
    const char* binding = "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef";
    assert(GT::configure("1", path.c_str(), binding));
    std::mutex mutex; std::condition_variable cv;
    PETARI_GPU_TRACE(IT::request(&mutex, "fixture"));
    mutex.lock();
    PETARI_GPU_TRACE(IT::acquired(&mutex, "fixture"));
    std::unique_lock<std::mutex> lock(mutex, std::adopt_lock);
    int predicates = 0;
    IT::wait(cv, lock, [&] { ++predicates; return true; });
    assert(predicates == 1 && lock.owns_lock()); // no fabricated release/reacquire when already true
    bool ready = false;
    std::thread notifier([&] {
        PETARI_GPU_TRACE(IT::request(&mutex, "fixture-notifier"));
        mutex.lock();
        PETARI_GPU_TRACE(IT::acquired(&mutex, "fixture-notifier"));
        ready = true;
        PETARI_GPU_TRACE(IT::notify(&mutex, &cv, nullptr, "fixture-cv", false));
        cv.notify_all();
        PETARI_GPU_TRACE(IT::notify(&mutex, &cv, nullptr, "fixture-cv", true));
        PETARI_GPU_TRACE(IT::releaseBegin(&mutex, "fixture-notifier"));
        mutex.unlock();
        PETARI_GPU_TRACE(IT::released(&mutex, "fixture-notifier"));
    });
    IT::wait(cv, lock, [&] { return ready; }); // unchanged untimed std::condition_variable
    assert(ready && lock.owns_lock());
    PETARI_GPU_TRACE(IT::releaseBegin(&mutex, "fixture"));
    lock.unlock();
    PETARI_GPU_TRACE(IT::released(&mutex, "fixture"));
    notifier.join();
    GT::shutdown();
    assert(GT::run->ring.drops.load() == 0 && GT::run->ioErrors.load() == 0);
    std::ifstream input(path);
    const std::string contents((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
    assert(contents.find("interrupt_mutex_cv_release_pending") != std::string::npos);
    assert(contents.find("interrupt_mutex_cv_reacquired_bound") != std::string::npos);
    assert(contents.find("cv-predicate-reacquired") != std::string::npos);
    assert(contents.find("interrupt_cv_notify_begin") != std::string::npos);
    assert(contents.find("interrupt_cv_notify_return") != std::string::npos);
    assert(contents.find("cv_waits_reacquire_unknown") != std::string::npos);
    assert(contents.find("cv_wake") == std::string::npos);
    assert(contents.find("interrupt_mutex_protocol_error") == std::string::npos);
    assert(contents.find("\"complete\":true") != std::string::npos);
    input.close(); std::remove(path.c_str()); rmdir(temporary);
}
