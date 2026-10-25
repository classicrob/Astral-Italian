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
    PETARI_GPU_TRACE(++*pointer);
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
    PETARI_GPU_TRACE(emit("writer_fixture"));
    shutdown();
    assert(!enabled.load() && !run->ioErrors.load());
    const auto sequence = producerSequence;
    PETARI_GPU_TRACE(emit("must_not_exist", {}, sideEffect()));
    assert(producerSequence == sequence && sideEffects == 0);
    std::ifstream input(path);
    std::string contents((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
    assert(contents.find("writer_fixture") != std::string::npos);
    assert(contents.find("must_not_exist") == std::string::npos);
    assert(contents.find("\"drops\":0") != std::string::npos);
    assert(contents.find("\"complete\":true") != std::string::npos);
    input.close();
    std::remove(path.c_str());
    rmdir(temporary);
}
