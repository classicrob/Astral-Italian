#pragma once

// Native-only diagnostic. The macro is the complete disabled path: arguments
// are not evaluated and no trace state (other than the flag) is accessed.
#include <petari/frame_telemetry.hpp>
#include <array>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <thread>
#include <unistd.h>
#if defined(__APPLE__)
#include <pthread.h>
#endif

namespace PetariNative::GpuTrace {
inline std::atomic<bool> enabled{false};
static_assert(std::atomic<bool>::is_always_lock_free, "trace flag must be one cached lock-free load");
struct Context {
    std::uint64_t packet = 0, ticket = 0, submission = 0, span = 0;
    std::uint64_t csv = 0, route = 0, subject = 0, detail = 0;
};
struct Record {
    const char* event = nullptr; // static string only; never a borrowed buffer
    std::uint64_t ns = 0, thread = 0, sequence = 0;
    Context context;
    std::int64_t value = 0;
};

// Bounded MPSC ring. A stalled producer cannot block another producer or a
// realtime/interrupt callback. Contention/full both count as explicit drops.
// The sole reader never skips an unpublished reservation.
template<std::size_t Capacity> class Ring {
    static_assert(Capacity && (Capacity & (Capacity - 1)) == 0);
    struct Cell { std::atomic<std::uint64_t> sequence{0}; Record record; };
    std::array<Cell, Capacity> cells{};
    alignas(64) std::atomic<std::uint64_t> write{0};
    alignas(64) std::uint64_t read = 0;
public:
    std::atomic<std::uint64_t> drops{0};
    Ring() { for (std::size_t i = 0; i < Capacity; ++i) cells[i].sequence.store(i); }
    bool push(const Record& record) noexcept {
        auto position = write.load(std::memory_order_relaxed);
        for (unsigned attempt = 0; attempt < 8; ++attempt) {
            auto& cell = cells[position & (Capacity - 1)];
            const auto sequence = cell.sequence.load(std::memory_order_acquire);
            if (sequence == position) {
                if (write.compare_exchange_weak(position, position + 1, std::memory_order_relaxed)) {
                    cell.record = record;
                    cell.sequence.store(position + 1, std::memory_order_release);
                    return true;
                }
                continue; // CAS updated position; do not compare the stale sequence
            }
            if (sequence < position) break; // full; never wait
            position = write.load(std::memory_order_relaxed);
        }
        drops.fetch_add(1, std::memory_order_relaxed);
        return false;
    }
    bool pop(Record& record) noexcept {
        auto& cell = cells[read & (Capacity - 1)];
        if (cell.sequence.load(std::memory_order_acquire) != read + 1) return false;
        record = cell.record;
        cell.sequence.store(read + Capacity, std::memory_order_release);
        ++read;
        return true;
    }
};
static_assert(std::atomic<std::uint64_t>::is_always_lock_free,
              "GPU trace producers require lock-free 64-bit atomics");

struct Run {
    Ring<65536> ring; // allocated/pre-touched only at configure; bounded ~9 MiB
    std::atomic<bool> accepting{true}, stopping{false};
    std::atomic<std::uint64_t> producers{0}, ids{1}, threads{1}, pending{0};
    std::atomic<std::uint64_t> ioErrors{0};
    std::FILE* file = nullptr;
    std::thread writer;
    std::uint64_t startNs = 0;
};
// Intentionally process-lifetime: late spontaneous GPU callbacks never touch
// destroyed state, and shutdown never waits for GPU work to obtain a footer.
inline Run* run = nullptr;
inline thread_local std::uint64_t threadId = 0, producerSequence = 0;
inline thread_local Context workerContext{}, recorderContext{}, waitContext{}, deliveryContext{};
inline thread_local std::uint64_t waitingThread = 0;
inline std::uint64_t nextId() noexcept { return run->ids.fetch_add(1, std::memory_order_relaxed); }
inline std::uint64_t nowNs() noexcept { return FrameTelemetry::nowNs(); }
inline void emitAt(const char* event, Context context, std::uint64_t ns, std::int64_t value = 0) noexcept {
    auto& r = *run;
    r.producers.fetch_add(1, std::memory_order_acq_rel);
    if (r.accepting.load(std::memory_order_acquire)) {
        if (!threadId) threadId = r.threads.fetch_add(1, std::memory_order_relaxed);
        r.ring.push({event, ns, threadId, ++producerSequence, context, value});
    }
    r.producers.fetch_sub(1, std::memory_order_release);
}
inline void emit(const char* event, Context context = {}, std::int64_t value = 0) noexcept {
    emitAt(event, context, nowNs(), value);
}
inline Context withSubject(Context c, const void* subject, std::uint64_t detail = 0) noexcept {
    c.subject = reinterpret_cast<std::uintptr_t>(subject); c.detail = detail; return c;
}
inline Context captureContext(std::uint64_t packet, std::uint64_t ticket) noexcept {
    Context c{}; c.packet = packet; c.ticket = ticket; return c;
}
inline thread_local std::array<Context, 16> workerStack{};
inline thread_local unsigned workerDepth = 0, workerOverflow = 0;
inline void beginWork(Context c) noexcept {
    if (workerDepth >= workerStack.size()) { ++workerOverflow; run->ring.drops.fetch_add(1); return; }
    workerStack[workerDepth++] = workerContext;
    c.span = nextId();
    workerContext = c;
    emit("worker_begin", c);
}
inline void endWork() noexcept {
    if (workerOverflow) { --workerOverflow; return; }
    emit("worker_end", workerContext);
    if (workerDepth) workerContext = workerStack[--workerDepth];
    else run->ring.drops.fetch_add(1);
}
inline void writeRecord(Run& r, const Record& e) {
    const auto& c = e.context;
    const int result = std::fprintf(r.file,
        "{\"type\":\"boundary\",\"event\":\"%s\",\"ns\":%llu,\"thread\":%llu,\"sequence\":%llu,"
        "\"packet\":%llu,\"ticket\":%llu,\"submission\":%llu,\"span\":%llu,\"csv\":%llu,"
        "\"route\":%llu,\"subject\":%llu,\"detail\":%llu,\"value\":%lld}\n",
        e.event, (unsigned long long)e.ns, (unsigned long long)e.thread, (unsigned long long)e.sequence,
        (unsigned long long)c.packet, (unsigned long long)c.ticket, (unsigned long long)c.submission,
        (unsigned long long)c.span, (unsigned long long)c.csv, (unsigned long long)c.route,
        (unsigned long long)c.subject, (unsigned long long)c.detail, (long long)e.value);
    if (result < 0) r.ioErrors.fetch_add(1, std::memory_order_relaxed);
}
inline void writerMain(Run& r) {
#if defined(__APPLE__)
    pthread_setname_np("petari-gpu-trace");
    pthread_set_qos_class_self_np(QOS_CLASS_UTILITY, 0); // never realtime
#endif
    auto flush = std::chrono::steady_clock::now();
    for (;;) {
        Record event;
        bool progress = false;
        while (r.ring.pop(event)) { writeRecord(r, event); progress = true; }
        if (r.stopping.load(std::memory_order_acquire) && !r.producers.load(std::memory_order_acquire)) {
            // Drain records published just before the last producer left.
            while (r.ring.pop(event)) writeRecord(r, event);
            break;
        }
        const auto current = std::chrono::steady_clock::now();
        if (current - flush >= std::chrono::milliseconds(200)) {
            if (std::fflush(r.file)) r.ioErrors.fetch_add(1, std::memory_order_relaxed);
            flush = current;
        }
        if (!progress) std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
    std::fprintf(r.file, "{\"type\":\"footer\",\"ns\":%llu,\"drops\":%llu,\"pending_callbacks\":%llu,"
        "\"io_errors\":%llu,\"complete\":%s}\n", (unsigned long long)nowNs(),
        (unsigned long long)r.ring.drops.load(), (unsigned long long)r.pending.load(),
        (unsigned long long)r.ioErrors.load(),
        !r.ring.drops.load() && !r.pending.load() && !r.ioErrors.load() ? "true" : "false");
    if (std::fflush(r.file)) r.ioErrors.fetch_add(1);
    std::fclose(r.file);
}
inline void shutdown() {
    if (!run || !enabled.exchange(false, std::memory_order_acq_rel)) return;
    run->accepting.store(false, std::memory_order_release);
    run->stopping.store(true, std::memory_order_release);
    if (run->writer.joinable()) run->writer.join(); // writer only, never GPU
}
inline bool configure(const char* diag, const char* path, const char* binding = nullptr) {
    if (!diag || std::strcmp(diag, "1") || !path || path[0] != '/') return false;
    if (!binding || std::strlen(binding) != 64 || std::strspn(binding, "0123456789abcdef") != 64) {
        std::fprintf(stderr, "[gpu trace] valid PETARI_GPU_TRACE_BINDING_SHA256 required; disabled\n");
        return false;
    }
    if (run) return enabled.load(std::memory_order_relaxed); // one run/process
    auto* file = std::fopen(path, "wx"); // never overwrite an earlier trace
    if (!file) { std::fprintf(stderr, "[gpu trace] cannot create output: %s\n", path); return false; }
    run = new Run;
    run->file = file;
    std::setvbuf(file, nullptr, _IOFBF, 256 * 1024);
    run->startNs = nowNs();
    std::fprintf(file, "{\"type\":\"header\",\"schema\":1,\"clock\":\"steady_clock_ns\",\"start_ns\":%llu,"
        "\"completion\":\"cpu-observed-queue-prefix-including-callback-dispatch\","
        "\"ring_capacity\":65536,\"writer\":\"non-realtime-utility\",\"pid\":%ld,\"binding_sha256\":\"%s\"}\n", (unsigned long long)run->startNs, (long)getpid(), binding);
    run->writer = std::thread([] { writerMain(*run); });
    std::atexit(shutdown);
    enabled.store(true, std::memory_order_release);
    return true;
}
} // namespace PetariNative::GpuTrace

#define PETARI_GPU_TRACE(...) do { \
    if (::PetariNative::GpuTrace::enabled.load(std::memory_order_relaxed)) { __VA_ARGS__; } \
} while (false)
