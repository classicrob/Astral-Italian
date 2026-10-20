#include <petari/platform/null_gx_processor.hpp>
#include <condition_variable>
#include <cstdio>
#include <cstdlib>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <vector>
using Processor = PetariNative::Platform::NullGxProcessor;
namespace {
int checks = 0;
void check(bool value, const char* message) { ++checks; if (!value) { std::fprintf(stderr, "%s\n", message); std::exit(1); } }
struct Oracle {
    std::uint64_t last = 0;
    std::vector<std::uint64_t> events;
    std::vector<std::uint64_t> hits;
    void event(std::uint64_t position) {
        if (position < last) throw std::logic_error("event reported out of stream order");
        last = position; events.push_back(position);
    }
    void pump(Processor& processor, std::uint64_t bp) {
        processor.pump(bp, [&](const auto& e) { event(e.position); },
                       [](auto, auto) {}, [&](auto position) { event(position); hits.push_back(position); });
    }
};
}
int main() {
    Processor processor; Oracle oracle;
    processor.publish(100); oracle.pump(processor, Processor::NoBreakpoint);
    const auto savedPointer = processor.written(); // DrawSyncManager::pushBreakPoint
    std::mutex lock; std::condition_variable changed; bool tokenProcessed = false;
    std::thread producer([&] {
        std::lock_guard<std::mutex> guard(lock);
        processor.publish(105); processor.append(Processor::Kind::Token, 1, 105);
        oracle.pump(processor, Processor::NoBreakpoint);
        tokenProcessed = true; changed.notify_one();
    });
    std::thread manager([&] {
        std::unique_lock<std::mutex> guard(lock);
        changed.wait(guard, [&] { return tokenProcessed; });
        oracle.pump(processor, savedPointer); // delayed arm of address 100
    });
    producer.join(); manager.join();
    check(processor.processed() == 105 && oracle.hits.empty(), "late breakpoint must not fire retroactively");
    bool oldRejected = false;
    try { oracle.event(savedPointer); } catch (const std::logic_error&) { oldRejected = true; }
    check(oldRejected, "old immediate-report implementation deterministically violates ordering by five bytes");
    oracle.pump(processor, 110); // future breakpoint
    processor.publish(110); processor.append(Processor::Kind::Token, 2, 110); oracle.pump(processor, 110);
    check(oracle.events == std::vector<std::uint64_t>({105, 110, 110}), "token ending at breakpoint reports before breakpoint");
    processor.publish(115); processor.append(Processor::Kind::DrawDone, 0, 115); oracle.pump(processor, 110);
    check(processor.processed() == 110 && processor.pending() == 1, "breakpoint holds processor, not writer; event retained");
    oracle.pump(processor, Processor::NoBreakpoint);
    check(processor.processed() == 115 && processor.pending() == 0 && oracle.events.back() == 115, "clearing breakpoint drains retained draw done");
    processor.publish(120); processor.append(Processor::Kind::Token, 3, 120); oracle.pump(processor, 115);
    check(processor.pending() == 1, "halted pending token");
    processor.abort(120); oracle.pump(processor, Processor::NoBreakpoint);
    check(processor.processed() == 120 && processor.pending() == 0 && oracle.events.back() == 115, "abort discards unprocessed commands without inventing an event");
    const auto beforeStress = oracle.events.size();
    for (unsigned i = 0; i < 10000; ++i) {
        const auto old = processor.written();
        processor.publish(old + 5); processor.append(Processor::Kind::Token, 4, old + 5);
        oracle.pump(processor, Processor::NoBreakpoint); oracle.pump(processor, old);
        check(processor.processed() == old + 5, "stress: delayed stale breakpoint never rewinds cursor");
    }
    check(oracle.events.size() == beforeStress + 10000, "stress delivers every new token exactly once, no stale hits");
    std::puts("null GX processor deterministic interleaving and stress PASS");
}
