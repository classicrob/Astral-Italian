// Finite host-time test stimulus for an unchanged perf executable. No game clock writes.
#include <atomic>
#include <chrono>
#include <cinttypes>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <dlfcn.h>
#include <fstream>
#include <mach-o/dyld.h>
#include <mach-o/loader.h>
#include <map>
#include <pthread.h>
#include <random>
#include <string>
#include <thread>
#include <time.h>
#include <sys/stat.h>

namespace {
using U64 = std::uint64_t;
using KeyEvent = void (*)(std::uint16_t, bool, bool);
using Quit = void (*)();
U64 now() { return clock_gettime_nsec_np(CLOCK_UPTIME_RAW); }
void sleepMs(int ms) { std::this_thread::sleep_for(std::chrono::milliseconds(ms)); }

void* run(void*) {
    pthread_setname_np("m1-latency-ledger");
    const char* out = std::getenv("PETARI_M1_LATENCY_LEDGER");
    const char* symbols = std::getenv("PETARI_M1_LATENCY_SYMBOLS");
    const char* exe = std::getenv("PETARI_M1_LATENCY_EXE");
    if (!out || !symbols || !exe) return nullptr;
    FILE* log = std::fopen(out, "w");
    if (!log) return nullptr;
    std::setvbuf(log, nullptr, _IOLBF, 0);
    auto error = [&](const char* reason) {
        std::fprintf(log, "{\"event\":\"error\",\"ns\":%" PRIu64 ",\"reason\":\"%s\"}\n", now(), reason);
    };
    // DYLD_INSERT_LIBRARIES may put the injected image before the main executable.
    std::uint32_t mainImage = _dyld_image_count();
    for (std::uint32_t i = 0; i < _dyld_image_count(); ++i)
        if (_dyld_get_image_header(i)->filetype == MH_EXECUTE) { mainImage = i; break; }
    struct stat expectedFile{}, mainFile{};
    if (mainImage == _dyld_image_count() || stat(exe, &expectedFile) != 0 ||
        stat(_dyld_get_image_name(mainImage), &mainFile) != 0 ||
        expectedFile.st_dev != mainFile.st_dev || expectedFile.st_ino != mainFile.st_ino) {
        error("main executable file mismatch"); std::fclose(log); return nullptr;
    }
    std::map<std::string, U64> addresses;
    std::ifstream input(symbols);
    std::string name, hex;
    while (input >> name >> hex) addresses[name] = std::strtoull(hex.c_str(), nullptr, 16);
    const auto slide = _dyld_get_image_vmaddr_slide(mainImage);
    auto address = [&](const char* name) -> void* {
        const auto it = addresses.find(name);
        return it == addresses.end() ? nullptr : reinterpret_cast<void*>(it->second + slide);
    };
    auto key = reinterpret_cast<KeyEvent>(address("key"));
    auto quit = reinterpret_cast<Quit>(address("quit"));
    auto gameplay = reinterpret_cast<std::atomic<bool>*>(address("gameplay"));
    auto stage = reinterpret_cast<std::atomic<int>*>(address("stage"));
    auto press = reinterpret_cast<std::atomic<U64>*>(address("press"));
    auto report = reinterpret_cast<std::atomic<U64>*>(address("report"));
    auto read = reinterpret_cast<std::atomic<U64>*>(address("read"));
    auto copy = reinterpret_cast<std::atomic<U64>*>(address("copy"));
    auto shown = reinterpret_cast<std::atomic<U64>*>(address("shown"));
    auto present = reinterpret_cast<std::atomic<U64>*>(address("present"));
    auto readFrame = reinterpret_cast<std::atomic<std::uint32_t>*>(address("read_frame"));
    auto shownFrame = reinterpret_cast<std::atomic<std::uint32_t>*>(address("shown_frame"));
    if (!key || !quit || !gameplay || !stage || !press || !report || !read || !copy || !shown || !present || !readFrame || !shownFrame) {
        error("missing symbol map"); std::fclose(log); return nullptr;
    }
    // Cross-check exported code/data against the SHA-bound nm map before any input.
    if (dlsym(RTLD_DEFAULT, "_ZN12PetariNative5Input8keyEventEtbb") != reinterpret_cast<void*>(key) ||
        dlsym(RTLD_DEFAULT, "_ZN12PetariNative3App4Host11requestQuitEv") != reinterpret_cast<void*>(quit) ||
        dlsym(RTLD_DEFAULT, "_ZN12PetariNative12LatencyProbe5stageE") != stage) {
        error("export/map mismatch"); std::fclose(log); return nullptr;
    }
    std::fprintf(log, "{\"event\":\"start\",\"ns\":%" PRIu64 ",\"clock\":\"CLOCK_UPTIME_RAW\",\"planned_presses\":27,\"seed\":12345,\"gap_min_ms\":1000,\"gap_max_ms\":1600,\"hold_ms\":60,\"symbol_exports_verified\":true,\"main_executable_file_verified\":true,\"main_image_index\":%u}\n", now(), mainImage);
    std::mt19937 random(12345);
    std::uniform_int_distribution<int> gap(1000, 1600);
    const U64 deadline = now() + 150'000'000'000ULL;
    bool failed = false;
    int sent = 0;
    U64 origin = now();
    for (int id = 1; id <= 27 && !failed;) {
        const int delay = gap(random);
        const U64 planned = origin + static_cast<U64>(delay) * 1'000'000;
        // Absolute host schedule: waiting for a previous read never re-anchors the next send to a game frame.
        for (;;) { const U64 current = now(); if (current >= planned) break; std::this_thread::sleep_for(std::chrono::nanoseconds(planned - current)); }
        if (now() > deadline) { error("driver deadline"); failed = true; break; }
        if (!gameplay->load(std::memory_order_relaxed)) {
            std::fprintf(log, "{\"event\":\"skip_not_gameplay\",\"id\":%d,\"ns\":%" PRIu64 ",\"planned_ns\":%" PRIu64 "}\n", id, now(), planned);
            if (sent) { failed = true; break; }
            origin = now();
            continue;
        }
        const int busy = stage->load();
        if (busy != 0) {
            std::fprintf(log, "{\"event\":\"skip_busy\",\"id\":%d,\"ns\":%" PRIu64 ",\"stage\":%d}\n", id, now(), busy);
            failed = true; break; // Never replace a lost attempt with another press.
        }
        const U64 send = now();
        origin = planned + 60'000'000ULL;
        key(44, true, false); // Public native API, same call as LatencyReport::startTestDriver.
        const U64 returned = now(), nativePress = press->load();
        ++sent;
        std::fprintf(log, "{\"event\":\"send\",\"id\":%d,\"ns\":%" PRIu64 ",\"return_ns\":%" PRIu64 ",\"native_press_ns\":%" PRIu64 ",\"planned_ns\":%" PRIu64 ",\"gap_ms\":%d,\"stage_before\":0}\n", id, send, returned, nativePress, planned, delay);
        sleepMs(60);
        const U64 release = now();
        key(44, false, false);
        std::fprintf(log, "{\"event\":\"release\",\"id\":%d,\"ns\":%" PRIu64 ",\"return_ns\":%" PRIu64 "}\n", id, release, now());
        const U64 matchDeadline = now() + 3'000'000'000ULL;
        while (now() < matchDeadline && (stage->load() != 0 || present->load() < nativePress || read->load() < nativePress)) sleepMs(1);
        const bool matched = nativePress >= send && nativePress <= returned && press->load() == nativePress &&
                             stage->load() == 0 && read->load() >= nativePress && present->load() >= nativePress;
        std::fprintf(log, "{\"event\":\"match\",\"id\":%d,\"ns\":%" PRIu64 ",\"matched\":%s,\"native_press_ns\":%" PRIu64 ",\"report_ns\":%" PRIu64 ",\"read_ns\":%" PRIu64 ",\"copy_ns\":%" PRIu64 ",\"shown_ns\":%" PRIu64 ",\"present_ns\":%" PRIu64 ",\"read_frame\":%u,\"shown_frame\":%u,\"stage\":%d}\n",
                     id, now(), matched ? "true" : "false", nativePress, report->load(), read->load(), copy->load(), shown->load(), present->load(), readFrame->load(), shownFrame->load(), stage->load());
        failed = !matched || shownFrame->load() != readFrame->load() + 2;
        ++id;
    }
    sleepMs(250); // Last native measurement already completed; allow a final presentation/log drain.
    std::fprintf(log, "{\"event\":\"quit\",\"ns\":%" PRIu64 ",\"sent\":%d,\"failed\":%s,\"drain_ms\":250}\n", now(), sent, failed ? "true" : "false");
    std::fclose(log);
    quit();
    return nullptr;
}
__attribute__((constructor)) void start() {
    if (!std::getenv("PETARI_M1_LATENCY_LEDGER")) return;
    pthread_t thread;
    if (pthread_create(&thread, nullptr, run, nullptr) == 0) pthread_detach(thread);
}
}
