#include "interrupt_trace.hpp"
#include "interrupt_trace_state.hpp"
#include <petari/gpu_trace.hpp>
#include <atomic>
#include <pthread.h>
namespace PetariNative::Platform::OS::InterruptTrace {
namespace {
namespace Trace = ::PetariNative::GpuTrace;
std::atomic<std::uint64_t> explicitRequests{0}, cvUnknown{0};
thread_local State state;
std::uintptr_t host() { return reinterpret_cast<std::uintptr_t>(pthread_self()); }
void emit(const char* event, const void* mutex, const char* reason, std::uint64_t ns,
          const void* cv = nullptr, std::int64_t value = 0) {
    Trace::Context c{}; c.span = state.span; c.subject = host();
    c.detail = reinterpret_cast<std::uintptr_t>(mutex);
    Trace::Attributes a; a.reason = reason; a.holder = state.held ? host() : 0;
    a.waiter = state.pending ? host() : 0; a.heldNs = state.heldNs(ns);
    a.mutex.present = true; a.mutex.address = reinterpret_cast<std::uintptr_t>(mutex); a.mutex.cv = reinterpret_cast<std::uintptr_t>(cv);
    a.mutex.explicitRequests = explicitRequests.load(std::memory_order_relaxed);
    a.mutex.cvUnknown = cvUnknown.load(std::memory_order_relaxed);
    Trace::emitAt(event, c, ns, value, a);
}
void protocolError(const void* mutex, const char* reason) {
    emit("interrupt_mutex_protocol_error", mutex, reason, Trace::nowNs());
    Trace::run->ring.drops.fetch_add(1, std::memory_order_relaxed); // invalidates window, never changes native lock behavior
}
}
void request(const void* mutex, const char* reason) {
    const auto ns = Trace::nowNs();
    if (!state.request(Trace::nextId(), ns)) { protocolError(mutex, reason); return; }
    explicitRequests.fetch_add(1, std::memory_order_relaxed);
    emit("interrupt_mutex_request", mutex, reason, ns);
}
void acquired(const void* mutex, const char* reason) {
    const auto ns = Trace::nowNs();
    if (!state.acquired(ns)) { protocolError(mutex, reason); return; }
    explicitRequests.fetch_sub(1, std::memory_order_relaxed);
    emit("interrupt_mutex_acquired", mutex, reason, ns);
}
void failed(const void* mutex, const char* reason) {
    const auto ns = Trace::nowNs();
    if (!state.failed()) { protocolError(mutex, reason); return; }
    explicitRequests.fetch_sub(1, std::memory_order_relaxed);
    emit("interrupt_mutex_try_failed", mutex, reason, ns);
}
void releaseBegin(const void* mutex, const char* reason) {
    if (!state.releaseBegin()) { protocolError(mutex, reason); return; }
    emit("interrupt_mutex_release_begin", mutex, reason, Trace::nowNs());
}
void released(const void* mutex, const char* reason) {
    // Actual unlock precedes this upper-bound timestamp; another thread may
    // already own the mutex. State is per emitter, never a global fake owner.
    const auto ns = Trace::nowNs();
    emit("interrupt_mutex_released", mutex, reason, ns);
    if (!state.released()) protocolError(mutex, reason);
}
void releasePending(const void* mutex, const char* reason) {
    emit("interrupt_mutex_release_pending", mutex, reason, Trace::nowNs());
    if (!state.released()) protocolError(mutex, reason);
}
void cvReleasePending(const void* mutex, const void* cv, const char* reason) {
    const auto ns = Trace::nowNs();
    emit("interrupt_mutex_cv_release_pending", mutex, reason, ns, cv);
    if (!state.cvRelease()) { protocolError(mutex, reason); return; }
    cvUnknown.fetch_add(1, std::memory_order_relaxed);
}
void cvReacquired(const void* mutex, const void* cv, const char* reason) {
    const auto ns = Trace::nowNs();
    if (!state.cvAcquire(Trace::nextId(), ns)) { protocolError(mutex, reason); return; }
    cvUnknown.fetch_sub(1, std::memory_order_relaxed);
    emit("interrupt_mutex_cv_reacquired_bound", mutex, reason, ns, cv);
}
void predicate(const void* mutex, const void* cv, bool ready) {
    emit("interrupt_cv_predicate", mutex, "predicate-after-mutex-acquisition", Trace::nowNs(), cv, ready);
}
void notify(const void* mutex, const void* cv, const void* recipient, const char* reason, bool returned) {
    const auto ns = Trace::nowNs();
    Trace::Context c{}; c.span = state.span; c.subject = reinterpret_cast<std::uintptr_t>(recipient);
    c.detail = reinterpret_cast<std::uintptr_t>(cv);
    Trace::Attributes a; a.reason = reason; a.holder = host(); a.heldNs = state.heldNs(ns);
    a.mutex.present = true; a.mutex.address = reinterpret_cast<std::uintptr_t>(mutex); a.mutex.cv = reinterpret_cast<std::uintptr_t>(cv); a.mutex.explicitRequests = explicitRequests.load(); a.mutex.cvUnknown = cvUnknown.load();
    Trace::emitAt(returned ? "interrupt_cv_notify_return" : "interrupt_cv_notify_begin", c, ns,
                  reinterpret_cast<std::uintptr_t>(mutex), a);
}
}
