#pragma once
#include <array>
#include <atomic>
#include <cstdint>

namespace PetariNative::Platform::AudioWorkgroup::History {
enum class Kind : unsigned { Cycle=1, Subframe, Finish, Assigned, Resumed, Callback, Shortfall, Trigger,
    Shape, Left, Right, Queued, Deadline, Registration, RegistrationFrames, RegistrationAddress,
    AQEntry, AQExit, DeviceLockRequest, DeviceLockAcquired, DeviceLockReleased,
    GetBegin, GetLockRequest, GetLockAcquired, FillEntry, FillExit, ConvertBegin, ConvertEnd, GetEnd,
    PutBegin, PutLockRequest, PutLockAcquired, PutLockReleased, PutEnd, AQFormat,
    AQQos, AQQosDetail, AQSchedule, AQMachPriority, AQCpu, AQSampleErrors, AQMachPolicy,
    AUEntry, AUExit, AURingBefore, AURingAfter, AUShortfall, AUSampleTime, AUHostTime,
    AUConverterOutput, AUTrigger, AUConverterInput, AUConsumed, AUSourceEnqueued };
struct Event {
    std::uint64_t sequence=0, mach=0, thread=0, value=0;
    unsigned kind=0;
    std::uint64_t steadyNs=0, ticket=0, object=0;
};
// Bounded MPSC flight history; no allocation, waiting, formatting or I/O on
// producers. Single attempt per cell; any busy/stale write is counted, never
// spun on. Every payload is atomic so a concurrent reader has no data race.
template<unsigned Capacity, unsigned Limit>
class Recorder {
    static_assert(std::atomic<std::uint64_t>::is_always_lock_free, "history needs lock-free words");
    struct Published {
        std::atomic<std::uint64_t> version{0}, sequence{0}, mach{0}, thread{0}, value{0}, kind{0};
        std::atomic<std::uint64_t> steadyNs{0}, ticket{0}, object{0};
        bool read(Event& e, std::uint64_t& observedVersion) const {
            observedVersion=version.load(std::memory_order_seq_cst);
            e={sequence.load(std::memory_order_seq_cst),mach.load(std::memory_order_seq_cst),
                thread.load(std::memory_order_seq_cst),value.load(std::memory_order_seq_cst),
                static_cast<unsigned>(kind.load(std::memory_order_seq_cst)),steadyNs.load(std::memory_order_seq_cst),
                ticket.load(std::memory_order_seq_cst),object.load(std::memory_order_seq_cst)};
            return !(observedVersion&1) && observedVersion==version.load(std::memory_order_seq_cst);
        }
        void publish(const Event& e) {
            version.store(e.sequence*2+1,std::memory_order_seq_cst);
            sequence.store(e.sequence,std::memory_order_seq_cst); mach.store(e.mach,std::memory_order_seq_cst);
            kind.store(e.kind,std::memory_order_seq_cst); thread.store(e.thread,std::memory_order_seq_cst);
            value.store(e.value,std::memory_order_seq_cst); steadyNs.store(e.steadyNs,std::memory_order_seq_cst);
            ticket.store(e.ticket,std::memory_order_seq_cst); object.store(e.object,std::memory_order_seq_cst);
            version.store(e.sequence*2+2,std::memory_order_seq_cst);
        }
    };
    struct Cell {
        std::atomic_flag writer = ATOMIC_FLAG_INIT;
        // Publish the complete old payload here BEFORE changing current. A
        // frozen trigger prefix remains readable across one post-trigger wrap.
        // This is retention, not permission to ignore an overwritten event.
        Published current, previous;
    };
    std::array<Cell,Capacity> cells{};
    std::atomic<std::uint64_t> next{1}, lost{0}, triggerAt{0}, triggerEnd{1}, triggerCount{0};
    static constexpr std::uint64_t kPublishing = UINT64_MAX;
public:
    void prepare() { for(auto& cell:cells) { cell.current.mach.fetch_add(0,std::memory_order_relaxed);
        cell.previous.mach.fetch_add(0,std::memory_order_relaxed); } }
    void push(std::uint64_t mach, unsigned kind, std::uint64_t thread, std::uint64_t value,
              std::uint64_t steadyNs=0, std::uint64_t ticket=0, std::uint64_t object=0) {
        const auto seq=next.fetch_add(1,std::memory_order_seq_cst);
        auto& c=cells[seq%Capacity];
        if(c.writer.test_and_set(std::memory_order_acquire)) { ++lost; return; }
        Event old; std::uint64_t version=0;
        if(!c.current.read(old,version) || old.sequence>seq) { ++lost; c.writer.clear(std::memory_order_release); return; }
        if(old.sequence) c.previous.publish(old);
        c.current.publish({seq,mach,thread,value,kind,steadyNs,ticket,object});
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
        struct Rejected {
            std::uint64_t expected=0, currentSequence=0, previousSequence=0, currentVersion=0, previousVersion=0;
            std::uint64_t mach=0, thread=0;
            unsigned kind=0;
            bool inWindow=false; // false means membership UNKNOWN, never "outside"
            bool currentStable=false, previousStable=false;
        };
        std::array<Event,Limit> events{};
        std::array<Rejected,128> rejected{};
        std::uint64_t trigger=0, endMach=0, triggersSincePrevious=0, drops=0, unstable=0, clipped=0,
            retentionUnknown=0, recovered=0, outsideWindow=0, rejectedOmitted=0;
        unsigned count=0, rejectedCount=0;
    };
    // throughMach optionally includes already-published post-trigger events;
    // it never waits for them. Caller declares/caps that window in the config.
    bool take(std::uint64_t preTicks, Dump& out, std::uint64_t throughMach=0, std::uint64_t maxPostTicks=0) {
        out.trigger=0; out.triggersSincePrevious=0; out.drops=0;
        out.unstable=0; out.clipped=0; out.count=0;
        out.retentionUnknown=0; out.recovered=0; out.outsideWindow=0; out.rejectedCount=0; out.rejectedOmitted=0;
        auto stamp=triggerAt.load(std::memory_order_acquire);
        if(!stamp || stamp==kPublishing ||
            !triggerAt.compare_exchange_strong(stamp,kPublishing,std::memory_order_acq_rel)) return false;
        out.trigger=stamp;
        const auto cap=maxPostTicks>UINT64_MAX-stamp ? UINT64_MAX : stamp+maxPostTicks;
        out.endMach=throughMach>stamp ? (throughMach<cap ? throughMach : cap) : stamp;
        const auto end=out.endMach>stamp ? next.load(std::memory_order_seq_cst) : triggerEnd.load(std::memory_order_relaxed);
        out.triggersSincePrevious=triggerCount.exchange(0,std::memory_order_relaxed);
        triggerAt.store(0,std::memory_order_release);
        out.drops=lost.load(std::memory_order_acquire);
        const auto first=end>Capacity ? end-Capacity : 1;
        const auto begin=out.trigger>preTicks ? out.trigger-preTicks : 0;
        for(auto seq=first;seq<end;++seq) {
            auto& c=cells[seq%Capacity];
            Event e, retired; std::uint64_t a=0,b=0;
            bool currentStable=c.current.read(e,a), previousStable=false;
            bool exact=currentStable && e.sequence==seq;
            if(!exact) {
                previousStable=c.previous.read(retired,b);
                exact=previousStable && retired.sequence==seq;
                if(exact) { e=retired; ++out.recovered; }
            }
            if(!exact) {
                // One bounded retry closes the old->previous publication seam.
                currentStable=c.current.read(e,a);
                exact=currentStable && e.sequence==seq;
            }
            if(!exact) {
                // Timestamp/payload cannot be trusted: membership in the
                // window is UNKNOWN and the dump is refused separately. Never
                // count arbitrary old prefix cells as in-window instability.
                ++out.retentionUnknown;
                if(out.rejectedCount<out.rejected.size())
                    out.rejected[out.rejectedCount++]={seq,e.sequence,retired.sequence,a,b,0,0,0,false,currentStable,previousStable};
                else ++out.rejectedOmitted;
                continue;
            }
            if(seq==first && first>1 && e.mach>begin) ++out.clipped;
            if(e.mach<begin || e.mach>out.endMach) { ++out.outsideWindow; continue; }
            if(out.count==Limit) { ++out.clipped; continue; }
            out.events[out.count++]=e;
        }
        return true;
    }
};
}
