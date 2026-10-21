// Tests for the playtest stall logger (native/app/stall_log.hpp): cause classification, the loading rate limit,
// the recent-stall ring and the text it produces. Pure logic: no game, GPU or window.

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

#include "../app/stall_log.hpp"

namespace SL = PetariNative::App::StallLog;
using PetariNative::App::FrameStats::Phase;

namespace {
int checks = 0;
void check(bool condition, const char* label) {
    ++checks;
    if (!condition) {
        std::fprintf(stderr, "FAIL: %s\n", label);
        std::exit(1);
    }
}

SL::FrameInput frameOf(std::uint64_t n, double ms, Phase phase = Phase::Gameplay) {
    SL::FrameInput in;
    in.frame = n;
    in.intervalMs = ms;
    in.phase = phase;
    return in;
}

std::string causes(const SL::Stall& s) {
    char buf[96];
    SL::causeList(s, buf, sizeof(buf));
    return buf;
}

void testClassify() {
    SL::FrameInput in = frameOf(1, 30.0);
    check(causes(SL::classify(in)) == "other", "no counters: other");
    in.pipelines = 3;
    in.pipelineUs = 25000;
    SL::Stall s = SL::classify(in);
    check(s.primary == SL::PipelineCompile && causes(s) == "pipeline-compile", "pipeline primary");
    in.loads = 1;
    in.loadUs = 4000;
    in.textures = 5;
    in.textureUs = 500;  // under kMinCauseUs: not a cause, still reported as a count
    s = SL::classify(in);
    check(causes(s) == "pipeline-compile+resource-load", "co-occurring causes, primary first");
    check(s.textures == 5 && (s.mask & (1u << SL::TextureUpload)) == 0, "small texture time is not a cause");
    in.loadUs = 26000;
    check(causes(SL::classify(in)) == "resource-load+pipeline-compile", "primary is the longest");
    SL::FrameInput noEvents = frameOf(2, 30.0);
    noEvents.pipelineUs = 50000;  // time without events does not count
    check(causes(SL::classify(noEvents)) == "other", "time without events");
}

void testThreshold() {
    check(SL::thresholdFromText(nullptr) == SL::kDefaultThresholdMs, "unset");
    check(SL::thresholdFromText("") == SL::kDefaultThresholdMs, "empty");
    check(SL::thresholdFromText("0") == 0.0, "zero is off");
    check(SL::thresholdFromText("33.5") == 33.5, "number");
    check(SL::thresholdFromText("abc") == SL::kDefaultThresholdMs, "junk");
    check(SL::thresholdFromText("-4") == SL::kDefaultThresholdMs, "negative");
    SL::Log off(0.0);
    const SL::Outcome o = off.observe(frameOf(1, 500.0));
    check(!o.stall && !o.summary && off.total() == 0, "off logs nothing");
    SL::Log log(20.0);
    check(!log.observe(frameOf(1, 20.0)).stall, "at the threshold is not a stall");
    check(log.observe(frameOf(2, 20.1)).stall, "over the threshold is a stall");
}

void testLoadingRateLimit() {
    SL::Log log(20.0);
    for (std::uint64_t n = 1; n <= 40; n++) {
        const SL::Outcome o = log.observe(frameOf(n, n == 17 ? 400.0 : 25.0, n < 20 ? Phase::Startup : Phase::Loading));
        check(!o.stall && !o.summary, "loading stalls are not printed one by one");
    }
    SL::FrameInput quiet = frameOf(41, 16.7);
    quiet.phase = Phase::Gameplay;
    SL::Outcome o = log.observe(quiet);
    check(o.summary && !o.stall, "summary when the load ends");
    check(o.loading.count == 40 && o.loading.worstMs == 400.0 && o.loading.worstFrame == 17, "summary content");
    check(!log.observe(frameOf(42, 16.7)).summary, "summary only once");
    // A gameplay stall right after a load prints the summary first, then its own line.
    for (std::uint64_t n = 50; n < 53; n++) log.observe(frameOf(n, 90.0, Phase::Loading));
    o = log.observe(frameOf(53, 33.0));
    check(o.summary && o.stall && o.loading.count == 3, "summary and stall together");
    for (std::uint64_t n = 60; n < 62; n++) log.observe(frameOf(n, 90.0, Phase::Loading));
    SL::LoadSummary pending;
    check(log.takePending(&pending) && pending.count == 2 && !log.takePending(&pending), "exit flush");
    char text[200];
    SL::formatSummary(o.loading, text, sizeof(text));
    check(std::strstr(text, "3 frames over the threshold during loading, worst 90.0 ms") != nullptr, "summary text");
}

void testRing() {
    SL::Log log(20.0);
    // 40 stalls, one every 10 frames: the ring keeps the last 32.
    for (std::uint64_t n = 1; n <= 400; n++) log.observe(frameOf(n, n % 10 == 0 ? 25.0 : 16.7));
    check(log.total() == 40 && log.lastFrame() == 400, "totals");
    SL::Stall list[SL::kRing];
    std::size_t n = log.recent(1000, list, SL::kRing);
    check(n == SL::kRing && list[0].frame == 90 && list[n - 1].frame == 400, "ring keeps the latest 32, oldest first");
    n = log.recent(300, list, SL::kRing);
    check(n == 30 && list[0].frame == 110 && list[n - 1].frame == 400, "300-frame window");
    n = log.recent(10, list, SL::kRing);
    check(n == 1 && list[0].frame == 400, "window includes the current frame");
    for (std::uint64_t f = 401; f <= 405; f++) log.observe(frameOf(f, 16.7));
    check(log.recent(5, list, SL::kRing) == 0, "no stall in the last 5 frames");
    check(log.recent(10, list, SL::kRing) == 1, "stall 5 frames back");
}

void testText() {
    SL::FrameInput in = frameOf(9973, 34.2);
    in.pipelines = 3;
    in.pipelineUs = 30000;
    in.loads = 1;
    in.loadUs = 2000;
    char line[300];
    SL::formatLine(SL::classify(in), "stage YosshiHomeGalaxy zone 5", line, sizeof(line));
    check(std::strcmp(line, "[stall] frame 9973 34.2 ms phase=gameplay cause=pipeline-compile+resource-load "
                            "(pipelines 3, textures 0, loads 1) stage YosshiHomeGalaxy zone 5") == 0, line);
    in = frameOf(7, 41.0);
    in.dominantPart = PetariNative::App::FrameStats::GameWork;
    in.dominantUs = 38000;
    SL::formatLine(SL::classify(in), "", line, sizeof(line));
    check(std::strstr(line, "cause=other (game_work 38.0 ms; pipelines 0") != nullptr && line[std::strlen(line) - 1] == ')', line);

    SL::Log log(20.0);
    char text[1024];
    SL::formatCapture(log, 300, text, sizeof(text));
    check(std::strstr(text, "none over 20 ms") != nullptr, text);
    log.observe(frameOf(100, 16.7));
    in = frameOf(101, 55.5);
    in.textures = 2;
    in.textureUs = 5000;
    log.observe(in);
    SL::formatCapture(log, 300, text, sizeof(text));
    check(std::strstr(text, "1 over 20 ms") != nullptr && std::strstr(text, "stall frame 101 55.5 ms phase=gameplay cause=texture-upload") != nullptr, text);
    SL::Log off(0.0);
    SL::formatCapture(off, 300, text, sizeof(text));
    check(std::strstr(text, "logger off") != nullptr, text);
}
}  // namespace

int main() {
    testClassify();
    testThreshold();
    testLoadingRateLimit();
    testRing();
    testText();
    std::printf("native stall log tests passed (%d checks)\n", checks);
    return 0;
}
