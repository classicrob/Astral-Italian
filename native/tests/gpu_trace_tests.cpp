// Server-only C++ test. This fixture is never compiled on a measurement M1.
#include <petari/gpu_trace.hpp>
#include <cassert>
#include <vector>
#include <algorithm>
#include <fstream>
#include <string>

using namespace PetariNative::GpuTrace;
// PORT can inspect this Release symbol: one flag load/branch before any access.
extern "C" void petari_gpu_trace_off_probe(int* pointer) {
    PETARI_GPU_TRACE(++*pointer; emitScheduling("off_host", {}, {}, *pointer);
        waitBegin("off_wait", *pointer); waitEnd();
        baton.grant(*pointer, *pointer, "off_grant"));
}
static int sideEffects = 0;
static std::vector<int> transcript;
static int sideEffect() { ++sideEffects; return sideEffects; }
static void protocol(bool withTrace) {
    for (int event = 1; event <= 8; ++event) {
        if (withTrace) PETARI_GPU_TRACE(emit("fixture", {}, sideEffect()));
        // Fake request/encode/submit/map/ready/delivery/VI/resume baseline.
        transcript.push_back(event);
    }
}
int main() {
    assert(!configure(nullptr, "/unused"));
    assert(!configure("1", nullptr));
    assert(!configure("0", "/unused"));
    assert(!configure("1", "relative.jsonl"));
    assert(run == nullptr && !enabled.load());
    petari_gpu_trace_off_probe(nullptr); // disabled path must not dereference arguments
    protocol(false); const auto baseline = transcript; transcript.clear();
    protocol(true);
    assert(transcript == baseline && sideEffects == 0 && run == nullptr);
    assert(threadId == 0 && producerSequence == 0); // no trace-side evaluation

    // Fake-clock state tests: reassignment closes an unresumed assignment;
    // refusal preserves ownership, and an idle release ends the duration.
    BatonState state;
    auto first = state.grant(100, 7, "initial");
    assert(first.holder == 0 && first.waiter == 7 && first.heldNs == 0);
    auto refused = state.attributes(115, "refusal-system-code", 9);
    assert(refused.holder == 7 && refused.waiter == 9 && refused.heldNs == 15);
    assert(state.holder == 7 && state.sinceNs == 100 && state.generation == 1);
    auto second = state.grant(130, 9, "reassign-unresumed");
    assert(second.holder == 7 && second.waiter == 9 && second.heldNs == 30);
    auto idle = state.grant(160, 0, "host-blocking");
    assert(idle.holder == 9 && idle.heldNs == 30 && state.sinceNs == 0);
    assert(state.grant(200, 7, "interrupt-idle-grant").heldNs == 0);
    auto late = waitAttributes({"timed-cond", {}, 100, 120}, 135);
    assert(late.deadlineNs == 120 && late.waitNs == 35 && late.lateNs == 15);
    auto early = waitAttributes({"timed-cond", {}, 100, 120}, 110);
    assert(early.waitNs == 10 && early.lateNs == 0);
    auto untimed = waitAttributes({"untimed-cond", {}, 100, 0}, 500);
    assert(untimed.waitNs == 400 && untimed.deadlineNs == 0 && untimed.lateNs == 0);
    CpuTopology topology; topology.ncpu = 10; topology.logical = {8, 2};
    assert(std::all_of(topology.cluster.begin(), topology.cluster.end(), [](char c) { return !c; }));
    // Deliberately non-monotone CPU IDs: no ordering/count-derived P/E map.
    topology.add(8, 'E'); topology.add(1, 'P'); topology.add(3, 'X');
    assert(topology.cluster[8] == 'E' && topology.cluster[1] == 'P' && !topology.cluster[3]);
    topology.add(8, 'P'); assert(topology.conflict[8] && !topology.cluster[8]);
    topology.add(8, 'E'); assert(!topology.cluster[8]); // conflict is sticky
    topology.add(1000, 'P'); // bounded, never indexes outside the table
    assert(decodeTopologyCluster("P", 1) == 'P');
    assert(decodeTopologyCluster("E", 2) == 'E');
    assert(decodeTopologyCluster("Efficiency", 10) == 0);
    assert(decodeTopologyCluster(nullptr, 1) == 0);
    unsigned cpu = 99; const std::uint32_t cpuCell = 2, invalidCell = 256;
    assert(decodeTopologyCpu(&cpuCell, sizeof(cpuCell), cpu) && cpu == 2);
    assert(!decodeTopologyCpu(&invalidCell, sizeof(invalidCell), cpu));
    assert(!decodeTopologyCpu(&cpuCell, 1, cpu));

    Ring<4> ring;
    Record event{"a", 10, 1, 1, {}};
    for (int i = 0; i < 4; ++i) { event.context.packet = i + 1; assert(ring.push(event)); }
    assert(!ring.push(event) && ring.drops.load() == 1);
    Record read;
    for (int i = 0; i < 4; ++i) { assert(ring.pop(read)); assert(read.context.packet == unsigned(i + 1)); }
    assert(!ring.pop(read));
    for (int i = 0; i < 100; ++i) { assert(ring.push(event)); assert(ring.pop(read)); }
    // Adversarial callback delivery: identities are by value, not a live frame.
    Context original{}; original.packet = 19; original.ticket = 7;
    auto callback = [original] { return original; };
    original.packet = 20;
    assert(callback().packet == 19);

    // MPSC stress: each accepted record appears exactly once, all rejected
    // insertions are counted. This tests bounded contention/full behavior.
    auto* concurrent = new Ring<1024>;
    std::atomic<unsigned> finished{0};
    std::vector<std::thread> producers;
    for (unsigned p = 0; p < 4; ++p) producers.emplace_back([&, p] {
        for (unsigned i = 0; i < 5000; ++i) {
            Record r{"multi", i, p + 1, i + 1, {}};
            concurrent->push(r);
        }
        ++finished;
    });
    std::vector<std::uint64_t> received;
    while (finished.load() < 4) {
        if (concurrent->pop(read)) received.push_back(read.thread * 100000 + read.sequence);
        else std::this_thread::yield();
    }
    for (auto& thread : producers) thread.join();
    while (concurrent->pop(read)) received.push_back(read.thread * 100000 + read.sequence);
    assert(received.size() + concurrent->drops.load() == 20000);
    std::sort(received.begin(), received.end());
    assert(std::adjacent_find(received.begin(), received.end()) == received.end());
    delete concurrent;
    char temporary[] = "/tmp/petari-gpu-trace-tests-XXXXXX";
    assert(mkdtemp(temporary));
    const std::string path = std::string(temporary) + "/trace.jsonl";
    const char* binding = "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef";
    assert(configure("1", path.c_str(), binding));
    PETARI_GPU_TRACE(emit("writer_fixture");
        emitScheduling("host_fixture");
        waitBegin("outer-untimed"); waitBegin("inner-timed", deadlineAfter(1000000000));
        waitEnd(); waitEndIf("different"); assert(waitDepth == 1);
        waitEndIf("outer-untimed"); assert(waitDepth == 0));
    PETARI_GPU_TRACE(Attributes sample; sample.mach.present = true; sample.mach.port = 7;
        sample.mach.publicationBefore = 2; sample.mach.publicationAfter = 4;
        sample.mach.generationBefore = 3; sample.mach.generationAfter = 4; sample.mach.error = 5;
        emitAt("mach_error_fixture", {}, nowNs(), -1, sample));
    shutdown();
    assert(!enabled.load() && !run->ioErrors.load());
    const auto sequence = producerSequence;
    PETARI_GPU_TRACE(emit("must_not_exist", {}, sideEffect()));
    assert(producerSequence == sequence && sideEffects == 0);
    std::ifstream input(path);
    std::string contents((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
    assert(contents.find("writer_fixture") != std::string::npos);
    assert(contents.find("\"schema\":2") != std::string::npos);
    assert(contents.find("\"topology\":") != std::string::npos);
    assert(contents.find("\"publication_before\":2,\"publication_after\":4") != std::string::npos);
    assert(contents.find("\"remote_cpu\":-1,\"stable_assigned_unresumed\":false") != std::string::npos);
    assert(contents.find("\"host\":") != std::string::npos);
    assert(contents.find("\"primitive\":\"inner-timed\"") != std::string::npos);
    assert(contents.find("must_not_exist") == std::string::npos);
    assert(contents.find("\"drops\":0") != std::string::npos);
    assert(contents.find("\"complete\":true") != std::string::npos);
    input.close();
    std::remove(path.c_str());
    rmdir(temporary);
}
