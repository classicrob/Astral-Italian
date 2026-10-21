#pragma once
// SCRATCH ONLY: world-map observation; never changes FIFO bytes or decoder checks.
#include <petari/host_allocation.hpp>
#include <algorithm>
#include <atomic>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <mutex>
#include <execinfo.h>
#include <dlfcn.h>

namespace PetariWmDiag {
inline bool enabled() {
    static const bool on = [] { const char* p = std::getenv("PETARI_WM_DIAG"); return p && !std::strcmp(p, "1"); }();
    return on;
}
inline std::mutex mutex;
struct Model { const void* pointer{}; char name[96]{}; };
inline Model models[8192];
inline unsigned modelCount{};
inline void model(const void* p, const char* name) {
    if (!enabled() || !p) return;
    std::lock_guard<std::mutex> guard(mutex);
    unsigned i = 0;
    for (; i < modelCount && models[i].pointer != p; ++i) {}
    if (i == 8192) return;
    if (i == modelCount) ++modelCount;
    models[i].pointer = p;
    std::snprintf(models[i].name, sizeof(models[i].name), "%s", name ? name : "unknown");
}
struct ContextData { const char* caller = "GX/direct"; const void* model{}; };
inline thread_local ContextData context;
struct Context {
    ContextData previous;
    Context(const char* caller, const void* model) : previous(context) { context = {caller, model}; }
    ~Context() { context = previous; }
};
struct Record {
    uint64_t seq{}, pre{}, begin{}, end{}, hash{};
    const void* pointer{};
    uint32_t size{};
    bool recording{};
    char caller[48]{}, name[96]{};
    unsigned char first[16]{};
    unsigned sampled{};
    void* stack[12]{};
    int frames{};
};
inline Record records[4096];
inline uint64_t sequence{};
inline void print(const Record& r, const char* reason) {
    std::fprintf(stderr, "[wm-dl] %s seq=%llu original=%p size=%u pre=%llu inline=[%llu,%llu) recording=%d caller=%s model=%s hash-fnv1a64=%016llx hash-bytes=%u first=",
        reason, (unsigned long long)r.seq, r.pointer, r.size, (unsigned long long)r.pre,
        (unsigned long long)r.begin, (unsigned long long)r.end, r.recording, r.caller, r.name,
        (unsigned long long)r.hash, r.sampled);
    for (unsigned i = 0; i < std::min(r.size, 16u) && i < r.sampled; ++i) std::fprintf(stderr, "%02x", r.first[i]);
    std::fputc('\n', stderr);
    for (int i = 0; i < r.frames; ++i) {
        Dl_info info{}; dladdr(r.stack[i], &info);
        std::fprintf(stderr, "[wm-dl-pc] seq=%llu frame=%d pc=%p image=%p symbol=%s\n",
            (unsigned long long)r.seq, i, r.stack[i], info.dli_fbase, info.dli_sname ? info.dli_sname : "unknown");
    }
}
inline uint64_t begin(const void* p, uint32_t n, uint64_t offset, bool recording) {
    if (!enabled()) return 0;
    PetariNative::HostAllocationScope host;
    Record r{};
    r.pointer = p; r.size = n; r.pre = offset; r.begin = offset; r.end = offset; r.recording = recording;
    std::snprintf(r.caller, sizeof(r.caller), "%s", context.caller);
    r.frames = backtrace(r.stack, 12);
    {
        std::lock_guard<std::mutex> guard(mutex);
        std::snprintf(r.name, sizeof(r.name), "unregistered:%p", context.model);
        for (unsigned i = modelCount; i; --i) if (models[i - 1].pointer == context.model) {
            std::snprintf(r.name, sizeof(r.name), "%s", models[i - 1].name); break;
        }
        r.seq = ++sequence;
        records[r.seq % 4096] = r;
        // Emit caller/address before touching an anomalous input's bytes.
        if (recording || n > 1024 * 1024 || (n & 31) || (reinterpret_cast<uintptr_t>(p) & 31) || (!p && n)) print(r, "producer-anomaly-before-read");
    }
    // Full hash up to 1 MiB, otherwise a bounded prefix; never scan for opcode bytes.
    r.sampled = p ? std::min(n, n <= 1024 * 1024 ? n : 32u) : 0;
    r.hash = 14695981039346656037ull;
    const auto* bytes = static_cast<const unsigned char*>(p);
    for (unsigned i = 0; i < r.sampled; ++i) { r.hash = (r.hash ^ bytes[i]) * 1099511628211ull; if (i < 16) r.first[i] = bytes[i]; }
    {
        std::lock_guard<std::mutex> guard(mutex);
        records[r.seq % 4096] = r;
    }
    return r.seq;
}
inline void span(uint64_t seq, uint64_t offset, uint32_t size) {
    if (!seq) return;
    std::lock_guard<std::mutex> guard(mutex);
    auto& r = records[seq % 4096];
    if (r.seq == seq) { r.begin = offset; r.end = offset + size; }
}
inline thread_local uint64_t workerBase{}, commandOffset{};
struct Command { uint64_t offset{}; unsigned char bytes[16]{}; unsigned n{}; };
inline thread_local Command history[16];
inline thread_local unsigned historyNext{};
inline thread_local uint32_t cp[256]{};
inline thread_local uint64_t cpOffset[256]{};
struct Draw { uint64_t offset{}; unsigned fmt{}, count{}, stride{}, bytes{}, remaining{}; };
inline thread_local Draw lastDraw;
struct Base { const void* pointer{}; unsigned size{}; bool little{}; uint64_t offset{}; };
inline thread_local Base bases[25];
inline void baseWrite(unsigned attr, const void* p, unsigned size, bool little) {
    if (enabled() && attr < 25) bases[attr] = {p, size, little, commandOffset};
}
inline void command(uint64_t offset, const void* p, unsigned n) {
    if (!enabled()) return;
    commandOffset = workerBase + offset;
    auto& c = history[historyNext++ % 16]; c.offset = commandOffset; c.n = std::min(n, 16u);
    std::memcpy(c.bytes, p, c.n);
}
inline void cpWrite(unsigned reg, uint32_t value) {
    if (enabled()) { cp[reg] = value; cpOffset[reg] = commandOffset; }
}
inline std::atomic<unsigned> failureReports{};
inline void fail(const char* reason) {
    if (!enabled()) return;
    const auto report = failureReports.fetch_add(1);
    if (report >= 32) {
        if (report == 32) std::fprintf(stderr, "[wm-fifo] diagnostic detail limit32 reached; decoder checks remain active\n");
        return;
    }
    std::lock_guard<std::mutex> guard(mutex);
    std::fprintf(stderr, "[wm-fifo] %s absolute=%llu last-draw=%llu fmt=%u count=%u vtx-size=%u total=%u remaining=%u\n",
        reason, (unsigned long long)commandOffset, (unsigned long long)lastDraw.offset,
        lastDraw.fmt, lastDraw.count, lastDraw.stride, lastDraw.bytes, lastDraw.remaining);
    const uint64_t oldest = sequence > 4095 ? sequence - 4095 : 1;
    uint64_t matched = 0;
    for (uint64_t s = oldest; s <= sequence; ++s) {
        const auto& r = records[s % 4096];
        if (r.seq == s && r.pre <= commandOffset && commandOffset < r.end) { matched = s; break; }
    }
    std::fprintf(stderr, "[wm-fifo] containing-seq=%llu retained=[%llu,%llu] (0 means direct/prepublish/unretained)\n",
        (unsigned long long)matched, (unsigned long long)oldest, (unsigned long long)sequence);
    if (matched) {
        const auto& r = records[matched % 4096];
        std::fprintf(stderr, "[wm-fifo] phase=%s relative=%lld\n", commandOffset < r.begin ? "preflush" : "inline-DL",
            (long long)commandOffset - (long long)r.begin);
        for (uint64_t s = std::max(oldest, matched > 3 ? matched - 3 : 1); s <= matched; ++s) print(records[s % 4096], "worker-context");
    }
    if (!matched) for (uint64_t s = std::max(oldest, sequence > 3 ? sequence - 3 : 1); s <= sequence; ++s)
        print(records[s % 4096], "recent-not-containing");
    for (unsigned a = 21; a <= 24; ++a) {
        const auto& b = bases[a];
        std::fprintf(stderr, "[wm-base] attr=%u pointer=%p size=%u le=%d absolute=%llu\n", a, b.pointer, b.size, b.little, (unsigned long long)b.offset);
    }
    for (unsigned fmt = 0; fmt < 8; ++fmt)
        std::fprintf(stderr, "[wm-vat] fmt=%u a=%08x b=%08x c=%08x offsets=%llu/%llu/%llu\n", fmt, cp[0x70+fmt], cp[0x80+fmt], cp[0x90+fmt],
            (unsigned long long)cpOffset[0x70+fmt], (unsigned long long)cpOffset[0x80+fmt], (unsigned long long)cpOffset[0x90+fmt]);
    for (unsigned reg : {0x50u, 0x60u, 0x70u, 0x80u, 0x90u, 0xacu, 0xadu, 0xaeu, 0xafu, 0xbcu, 0xbdu, 0xbeu, 0xbfu})
        std::fprintf(stderr, "[wm-cp] reg=%02x value=%08x absolute=%llu\n", reg, cp[reg], (unsigned long long)cpOffset[reg]);
    for (unsigned k = historyNext > 16 ? historyNext - 16 : 0; k < historyNext; ++k) {
        const auto& c = history[k % 16];
        std::fprintf(stderr, "[wm-cmd] absolute=%llu raw=", (unsigned long long)c.offset);
        for (unsigned i = 0; i < c.n; ++i) std::fprintf(stderr, "%02x", c.bytes[i]);
        std::fputc('\n', stderr);
    }
    std::fflush(stderr);
}
// Rebuilt each rendered frame; model pointers never survive a stage transition.
struct Node { const void* model{}; int point{}; bool drawn{}; };
inline Node nodes[1024];
inline unsigned nodeCount{}, nodeOverflow{};
inline bool worldFrame{};
inline void addNode(const void* model, int point) {
    if (!model || point < 0) return;
    if (nodeCount < 1024) nodes[nodeCount++] = {model, point, false}; else ++nodeOverflow;
}
inline void drawn(const void* model) {
    if (!enabled() || !worldFrame) return;
    for (unsigned i = 0; i < nodeCount; ++i) if (nodes[i].model == model) nodes[i].drawn = true;
}
inline unsigned drawnNodes() {
    unsigned count = 0;
    for (unsigned i = 0; i < nodeCount; ++i) if (nodes[i].drawn) {
        bool earlier = false;
        for (unsigned j = 0; j < i; ++j) if (nodes[j].drawn && nodes[j].point == nodes[i].point) { earlier = true; break; }
        if (!earlier) ++count;
    }
    return count;
}
}
