#pragma once
// Optional, exact first-use observations on the FIFO recording thread. No GPU
// readiness or completion time enters the captured renderer-frame deadlines.
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <deque>
#include <map>
#include <mutex>
#include <set>
#include <string>
#include <utility>

namespace PetariFirstUse {
inline const char* capturePath() {
    static const char* path = std::getenv("PETARI_PIPELINE_RECORD_FIRST_USE");
    return path && *path ? path : nullptr;
}
class Recorder {
    std::mutex mutex;
    std::FILE* file = nullptr;
    bool opened = false, finished = false, failed = false;
    std::string stage;
    std::uint64_t visit = 0, origin = UINT64_MAX, records = 0;
    std::uint64_t firstFrame = UINT64_MAX, lastFrame = 0, transitions = 0, lookups = 0;
    struct Request { std::string stage; std::uint64_t frame, visit; };
    std::deque<Request> pending;
    std::string requestedStage;
    std::uint64_t requests = 0, begins = 0, packetFrame = UINT64_MAX;
    std::set<std::uint64_t> seen;

    void open() {
        if (opened || finished) return;
        opened = true;
        file = std::fopen(capturePath(), "wx"); // never replace another capture
        if (!file) {
            failed = true;
            std::fputs("[gx first use] cannot create capture; observations unavailable\n", stderr);
            return;
        }
        std::fputs("{\"event\":\"header\",\"version\":3,\"gx_config_version\":13,"
                   "\"clock\":\"active-frame-packet-id\",\"source\":\"find_pipeline-before-memo\","
                   "\"origin_policy\":\"next-whole-packet\"}\n", file);
    }
public:
    void begin(const std::string& name, std::uint64_t requestFrame) {
        std::lock_guard lock{mutex};
        if (finished || requestedStage == name) return;
        requestedStage = name;
        pending.push_back({name, requestFrame, ++requests});
    }
    void packet(std::uint64_t frame) {
        std::lock_guard lock{mutex};
        if (finished || frame == UINT64_MAX) return;
        packetFrame = frame;
        while (!pending.empty() && (pending.front().frame == UINT64_MAX || frame > pending.front().frame)) {
            const auto request = std::move(pending.front()); pending.pop_front();
            stage = request.stage; visit = request.visit; origin = frame; seen.clear(); ++begins;
            open();
            if (!file) continue;
            const auto requestText = request.frame == UINT64_MAX ? std::string("null") : std::to_string(request.frame);
            std::fprintf(file, "{\"event\":\"stage_begin\",\"stage\":\"%s\",\"visit\":%llu,"
                         "\"request_packet\":%s,\"origin_frame\":%llu}\n", stage.c_str(),
                         (unsigned long long)visit, requestText.c_str(), (unsigned long long)origin);
            if (std::fflush(file) != 0 || std::ferror(file)) failed = true;
        }
    }
    void note(std::uint64_t config, std::uint64_t frame) {
        std::lock_guard lock{mutex};
        if (finished || stage.empty() || frame == UINT64_MAX) return;
        if (frame != packetFrame) { failed = true; return; }
        if (firstFrame == UINT64_MAX) firstFrame = frame;
        if (lookups && frame < lastFrame) failed = true;
        if (lookups && frame != lastFrame) ++transitions;
        lastFrame = frame; ++lookups;
        if (!seen.insert(config).second) return;
        open();
        if (!file) return;
        if (frame < origin) { failed = true; return; }
        std::fprintf(file, "{\"event\":\"first_use\",\"stage\":\"%s\",\"visit\":%llu,"
                     "\"origin_frame\":%llu,\"frame\":%llu,\"relative_frame\":%llu,\"config\":\"%016llx\"}\n",
                     stage.c_str(), (unsigned long long)visit, (unsigned long long)origin,
                     (unsigned long long)frame, (unsigned long long)(frame - origin), (unsigned long long)config);
        ++records;
        if (std::fflush(file) != 0 || std::ferror(file)) failed = true;
    }
    void finish() {
        std::lock_guard lock{mutex};
        if (finished) return;
        open(); finished = true;
        if (!file) return;
        if (std::ferror(file)) failed = true;
        const bool advanced = firstFrame != UINT64_MAX && lastFrame > firstFrame && transitions;
        failed |= !advanced || !pending.empty();
        std::fprintf(file, "{\"event\":\"end\",\"complete\":%s,\"records\":%llu,"
                     "\"clock_first_frame\":%llu,\"clock_last_frame\":%llu,\"clock_transitions\":%llu,\"lookup_calls\":%llu,"
                     "\"stage_requests\":%llu,\"stage_begins\":%llu,\"pending_stage_requests\":%zu}\n",
                     failed ? "false" : "true", (unsigned long long)records,
                     (unsigned long long)firstFrame, (unsigned long long)lastFrame,
                     (unsigned long long)transitions, (unsigned long long)lookups,
                     (unsigned long long)requests, (unsigned long long)begins, pending.size());
        if (std::fclose(file) != 0) failed = true;
        file = nullptr;
        std::fprintf(stderr, "[gx first use] complete=%u records=%llu\n", !failed, (unsigned long long)records);
    }
};
inline Recorder recorder;
inline void begin(const std::string& stage, std::uint64_t frame) { if (capturePath()) recorder.begin(stage, frame); }
inline void packet(std::uint64_t frame) { if (capturePath()) recorder.packet(frame); }
inline void note(std::uint64_t config, std::uint64_t frame) { if (capturePath()) recorder.note(config, frame); }
inline void finish() { if (capturePath()) recorder.finish(); }
} // namespace PetariFirstUse
