#pragma once
#include <petari/game_wait_history.hpp>
#include <array>
#include <limits>

namespace PetariNative::GameWaitHistory {
struct Event {
    std::uint64_t kind=0, begin=0, end=0, thread=0, object=0, value=0,
                  frame=0, id=0, detail=0, cpu=0, packet=0, ticket=0, generation=0, aux0=0, aux1=0;
};
static_assert(sizeof(Event)==15*sizeof(std::uint64_t), "versioned record shape");
enum class Verdict { Ordinary, NearMiss, Trigger, Invalid };
inline Verdict classify(std::uint64_t start, std::uint64_t end, std::int64_t cpuStart, std::int64_t cpuEnd) {
    if (!start || end<start || cpuStart<0 || cpuEnd<cpuStart) return Verdict::Invalid;
    const auto wall=end-start, cpu=static_cast<std::uint64_t>(cpuEnd-cpuStart);
    // cpu*10 < wall, without overflow or floating-point boundary tolerances.
    if (wall>100000000 && cpu <= (wall-1)/10) return Verdict::Trigger;
    return wall>50000000 ? Verdict::NearMiss : Verdict::Ordinary;
}
// MPSC overwrite history, independent from all other diagnostics. No producer
// allocation, wait, formatting or I/O. Snapshot refuses ANY missing sequence.
template<std::size_t Capacity> class Ring {
    static_assert(Capacity>1);
    static_assert(std::atomic<std::uint64_t>::is_always_lock_free);
    struct Cell {
        std::atomic_flag writing=ATOMIC_FLAG_INIT;
        std::atomic<std::uint64_t> version{0};
        std::array<std::atomic<std::uint64_t>,15> payload{};
    };
    std::array<Cell,Capacity> cells{};
    std::atomic<std::uint64_t> next{1}, lost{0};
public:
    static constexpr std::size_t capacity=Capacity;
    void prefault() noexcept { for (auto& c:cells) c.version.fetch_add(0,std::memory_order_relaxed); }
    std::uint64_t frontier() const noexcept { return next.load(std::memory_order_acquire); }
    std::uint64_t drops() const noexcept { return lost.load(std::memory_order_acquire); }
    bool push(const Event& e) noexcept {
        const auto seq=next.fetch_add(1,std::memory_order_relaxed);
        if (seq>std::numeric_limits<std::uint64_t>::max()/2-1) { ++lost; return false; }
        auto& cell=cells[seq%Capacity];
        if (cell.writing.test_and_set(std::memory_order_acquire)) { ++lost; return false; }
        if (cell.version.load(std::memory_order_relaxed)/2>seq) {
            ++lost; cell.writing.clear(std::memory_order_release); return false;
        }
        cell.version.store(seq*2+1,std::memory_order_seq_cst);
        const std::uint64_t words[]={e.kind,e.begin,e.end,e.thread,e.object,e.value,e.frame,e.id,e.detail,e.cpu,e.packet,e.ticket,e.generation,e.aux0,e.aux1};
        for (unsigned i=0;i<15;++i) cell.payload[i].store(words[i],std::memory_order_seq_cst);
        cell.version.store(seq*2+2,std::memory_order_seq_cst);
        cell.writing.clear(std::memory_order_release);
        return true;
    }
    bool read(std::uint64_t seq, Event& out) const noexcept {
        if (!seq || seq>std::numeric_limits<std::uint64_t>::max()/2-1) return false;
        const auto& cell=cells[seq%Capacity];
        const auto before=cell.version.load(std::memory_order_seq_cst);
        if (before!=seq*2+2) return false;
        std::uint64_t w[15];
        for (unsigned i=0;i<15;++i) w[i]=cell.payload[i].load(std::memory_order_seq_cst);
        if (cell.version.load(std::memory_order_seq_cst)!=before) return false;
        out={w[0],w[1],w[2],w[3],w[4],w[5],w[6],w[7],w[8],w[9],w[10],w[11],w[12],w[13],w[14]};
        return true;
    }
};
struct Budget {
    static constexpr std::uint64_t maxBytes=64ull*1024*1024;
    static constexpr unsigned maxWindows=16;
    std::uint64_t bytes=0; unsigned windows=0;
    bool take(std::uint64_t count, bool window=false) noexcept {
        if (count>maxBytes-bytes || (window && windows==maxWindows)) return false;
        bytes+=count; windows+=window; return true;
    }
};
struct Coverage {
    bool prefix=false, suffix=false; std::uint64_t missing=0,drops=0,clipped=0;
    bool complete() const noexcept { return prefix && suffix && !missing && !drops && !clipped; }
};
} // namespace PetariNative::GameWaitHistory
