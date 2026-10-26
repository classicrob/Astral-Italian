#pragma once
#include <array>
#include <atomic>
#include <cstdint>

namespace PetariNative::Platform::AudioWorkgroup::History {
enum class Kind : unsigned { Cycle=1, Subframe, Finish, Assigned, Resumed, Callback, Shortfall, Trigger,
    Shape, Left, Right, Queued, Deadline, Registration, RegistrationFrames, RegistrationAddress };
struct Event { std::uint64_t sequence=0, mach=0, thread=0, value=0; unsigned kind=0; };
// Bounded MPSC flight history; no allocation, waiting, formatting or I/O on
// producers. Single attempt per cell; any busy/stale write is counted, never
// spun on. Every payload is atomic so a concurrent reader has no data race.
template<unsigned Capacity, unsigned Limit>
class Recorder {
    static_assert(std::atomic<std::uint64_t>::is_always_lock_free, "history needs lock-free words");
    struct Cell {
        std::atomic_flag writer = ATOMIC_FLAG_INIT;
        std::atomic<std::uint64_t> version{0}, sequence{0}, mach{0}, thread{0}, value{0}, kind{0};
    };
    std::array<Cell,Capacity> cells{};
    std::atomic<std::uint64_t> next{1}, lost{0}, triggerAt{0}, triggerEnd{1}, triggerCount{0};
    static constexpr std::uint64_t kPublishing = UINT64_MAX;
public:
    void prepare() { for(auto& cell:cells) cell.mach.fetch_add(0,std::memory_order_relaxed); }
    void push(std::uint64_t mach, unsigned kind, std::uint64_t thread, std::uint64_t value) {
        const auto seq=next.fetch_add(1,std::memory_order_seq_cst);
        auto& c=cells[seq%Capacity];
        if(c.writer.test_and_set(std::memory_order_acquire)) { ++lost; return; }
        if(c.sequence.load(std::memory_order_seq_cst)>seq) { ++lost; c.writer.clear(std::memory_order_release); return; }
        c.version.store(seq*2+1,std::memory_order_seq_cst);
        c.sequence.store(seq,std::memory_order_seq_cst); c.mach.store(mach,std::memory_order_seq_cst);
        c.kind.store(kind,std::memory_order_seq_cst); c.thread.store(thread,std::memory_order_seq_cst);
        c.value.store(value,std::memory_order_seq_cst); c.version.store(seq*2+2,std::memory_order_seq_cst);
        c.writer.clear(std::memory_order_release);
    }
    void trigger(std::uint64_t ns) {
        std::uint64_t empty=0;
        if(triggerAt.compare_exchange_strong(empty,kPublishing,std::memory_order_acq_rel)) {
            triggerEnd.store(next.load(std::memory_order_seq_cst),std::memory_order_relaxed);
            triggerAt.store(ns,std::memory_order_release);
        }
        triggerCount.fetch_add(1,std::memory_order_relaxed);
    }
    struct Dump {
        std::array<Event,Limit> events{};
        std::uint64_t trigger=0, triggersSincePrevious=0, drops=0, unstable=0, clipped=0;
        unsigned count=0;
    };
    bool take(std::uint64_t preTicks, Dump& out) {
        out.trigger=0; out.triggersSincePrevious=0; out.drops=0;
        out.unstable=0; out.clipped=0; out.count=0;
        auto stamp=triggerAt.load(std::memory_order_acquire);
        if(!stamp || stamp==kPublishing ||
            !triggerAt.compare_exchange_strong(stamp,kPublishing,std::memory_order_acq_rel)) return false;
        out.trigger=stamp;
        const auto end=triggerEnd.load(std::memory_order_relaxed);
        out.triggersSincePrevious=triggerCount.exchange(0,std::memory_order_relaxed);
        triggerAt.store(0,std::memory_order_release);
        out.drops=lost.load(std::memory_order_acquire);
        const auto first=end>Capacity ? end-Capacity : 1;
        const auto begin=out.trigger>preTicks ? out.trigger-preTicks : 0;
        for(auto seq=first;seq<end;++seq) {
            auto& c=cells[seq%Capacity];
            const auto a=c.version.load(std::memory_order_seq_cst);
            Event e{c.sequence.load(std::memory_order_seq_cst),c.mach.load(std::memory_order_seq_cst),
                c.thread.load(std::memory_order_seq_cst),c.value.load(std::memory_order_seq_cst),
                static_cast<unsigned>(c.kind.load(std::memory_order_seq_cst))};
            if((a&1) || a!=c.version.load(std::memory_order_seq_cst) || e.sequence!=seq) { ++out.unstable; continue; }
            if(seq==first && first>1 && e.mach>begin) ++out.clipped;
            if(e.mach<begin || e.mach>out.trigger) continue;
            if(out.count==Limit) { ++out.clipped; continue; }
            out.events[out.count++]=e;
        }
        return true;
    }
};
}
