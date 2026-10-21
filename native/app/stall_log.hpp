#pragma once
// Playtest stall logger: names the cause of every frame that took longer than a threshold (default 20 ms), so a
// player's "it hitched" can be tied to a reason in the play log. Pure logic, no game, GPU or I/O dependency (the
// frame seam, frame_seam.cpp, feeds it one FrameInput per frame and prints what it returns).
//
// Causes come from the per-frame deltas of FrameTelemetry counters that are bumped at the work itself (a blocking
// pipeline resolve in the GX pipeline runtime, a static texture upload in the Aurora texture path, a disc read in the
// DVD drive), so they cost nothing unless that work happens. A cause counts for a frame when it accounts for at least
// kMinCauseUs of time; the primary cause is the one with the most time. Frames with no such cause are "other", with
// the dominating FrameStats game-thread part named.
//
// Stalls while the game is Startup/Loading are not printed one by one: they are summarised once, when the load ends.
// The last kRing stalls are kept for the playtest capture's state.txt (PlaytestCapture). Observing a frame does not
// allocate, lock or log. Standard headers only.

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>

#include "frame_stats.hpp"

namespace PetariNative::App::StallLog {

constexpr double kDefaultThresholdMs = 20.0;  // PETARI_STALL_MS overrides; 0 turns the logger off
constexpr std::uint32_t kMinCauseUs = 1000;
constexpr std::size_t kRing = 32;

enum Cause : unsigned { PipelineCompile, TextureUpload, ResourceLoad, Other, CauseCount };
inline const char* causeName(unsigned cause) {
    static const char* const names[CauseCount] = {"pipeline-compile", "texture-upload", "resource-load", "other"};
    return cause < CauseCount ? names[cause] : "?";
}

// One frame, from the seam. Counts are events in the frame, times are the matching microseconds.
struct FrameInput {
    std::uint64_t frame = 0;
    double intervalMs = 0.0;
    FrameStats::Phase phase = FrameStats::Phase::Gameplay;
    bool unfocused = false;
    std::uint32_t pipelines = 0, textures = 0, loads = 0;
    std::uint32_t pipelineUs = 0, textureUs = 0, loadUs = 0;
    unsigned dominantPart = FrameStats::Unattributed;  // largest game-thread FrameStats part
    std::uint32_t dominantUs = 0;
    // CPU time the game thread and Aurora's render worker actually ran in the frame (-1: unknown). A late
    // frame whose threads ran far less than its wall time was waiting or preempted (a busy host), not working.
    std::int32_t gameCpuUs = -1, renderCpuUs = -1;
};

struct Stall {
    std::uint64_t frame = 0;
    double ms = 0.0;
    FrameStats::Phase phase = FrameStats::Phase::Gameplay;
    bool unfocused = false;
    unsigned primary = Other;
    std::uint8_t mask = 0;  // bit per Cause (not Other) that counted
    std::uint32_t pipelines = 0, textures = 0, loads = 0;
    unsigned dominantPart = FrameStats::Unattributed;
    std::uint32_t dominantUs = 0;
    std::int32_t gameCpuUs = -1, renderCpuUs = -1;
};

inline Stall classify(const FrameInput& in) {
    Stall s;
    s.frame = in.frame;
    s.ms = in.intervalMs;
    s.phase = in.phase;
    s.unfocused = in.unfocused;
    s.pipelines = in.pipelines;
    s.textures = in.textures;
    s.loads = in.loads;
    s.dominantPart = in.dominantPart;
    s.dominantUs = in.dominantUs;
    s.gameCpuUs = in.gameCpuUs;
    s.renderCpuUs = in.renderCpuUs;
    const std::uint32_t us[3] = {in.pipelineUs, in.textureUs, in.loadUs};
    const std::uint32_t events[3] = {in.pipelines, in.textures, in.loads};
    std::uint32_t best = 0;
    for (unsigned c = 0; c < 3; c++) {
        if (events[c] == 0 || us[c] < kMinCauseUs) continue;
        s.mask = static_cast<std::uint8_t>(s.mask | (1u << c));
        if (us[c] > best) {  // ties go to the earlier cause
            best = us[c];
            s.primary = c;
        }
    }
    return s;
}

// "pipeline-compile+resource-load" (primary first), or "other".
inline void causeList(const Stall& s, char* out, std::size_t size) {
    std::size_t n = 0;
    out[0] = '\0';
    auto append = [&](unsigned c) {
        n += static_cast<std::size_t>(std::snprintf(out + n, n < size ? size - n : 0, "%s%s", n > 0 ? "+" : "", causeName(c)));
    };
    if (s.mask == 0) {
        append(Other);
        return;
    }
    append(s.primary);
    for (unsigned c = 0; c < 3; c++) {
        if ((s.mask & (1u << c)) != 0 && c != s.primary) append(c);
    }
}

// The summary of the stalls seen while loading.
struct LoadSummary {
    std::uint64_t count = 0;
    double worstMs = 0.0;
    std::uint64_t worstFrame = 0;
    std::uint8_t mask = 0;
    unsigned worstPrimary = Other;
};

struct Outcome {
    bool summary = false;  // print summary (loading ended)
    LoadSummary loading;
    bool stall = false;    // print stall
    Stall detail;
};

class Log {
public:
    Log() = default;
    explicit Log(double thresholdMs) : mThresholdMs(thresholdMs) {}

    double thresholdMs() const { return mThresholdMs; }
    void setThresholdMs(double ms) { mThresholdMs = ms; }
    bool enabled() const { return mThresholdMs > 0.0; }

    static bool loadingPhase(FrameStats::Phase p) {
        return p == FrameStats::Phase::Startup || p == FrameStats::Phase::Loading;
    }

    // The hot path: for a frame under the threshold, one store, one compare and (while a load summary is pending) one more.
    Outcome observe(const FrameInput& in) {
        Outcome out;
        mLastFrame = in.frame;
        if (mPending.count > 0 && !loadingPhase(in.phase)) {
            out.summary = true;
            out.loading = mPending;
            mPending = LoadSummary{};
        }
        if (!enabled() || in.intervalMs <= mThresholdMs) return out;
        const Stall s = classify(in);
        mRing[mRingNext] = s;
        mRingNext = (mRingNext + 1) % kRing;
        if (mRingCount < kRing) mRingCount++;
        mTotal++;
        if (loadingPhase(in.phase)) {
            mPending.count++;
            mPending.mask = static_cast<std::uint8_t>(mPending.mask | s.mask);
            if (in.intervalMs > mPending.worstMs) {
                mPending.worstMs = in.intervalMs;
                mPending.worstFrame = in.frame;
                mPending.worstPrimary = s.primary;
            }
        } else {
            out.stall = true;
            out.detail = s;
        }
        return out;
    }

    // Flushes a summary still pending at exit.
    bool takePending(LoadSummary* out) {
        if (mPending.count == 0) return false;
        *out = mPending;
        mPending = LoadSummary{};
        return true;
    }

    std::uint64_t lastFrame() const { return mLastFrame; }
    std::uint64_t total() const { return mTotal; }

    // Stalls in the window frames up to and including the last observed one, oldest first. Returns how many were copied.
    std::size_t recent(std::uint64_t window, Stall* out, std::size_t capacity) const {
        const std::uint64_t from = mLastFrame >= window ? mLastFrame - window + 1 : 0;
        std::size_t n = 0;
        for (std::size_t k = 0; k < mRingCount && n < capacity; k++) {
            const Stall& s = mRing[(mRingNext + kRing - mRingCount + k) % kRing];
            if (s.frame >= from) out[n++] = s;
        }
        return n;
    }

private:
    double mThresholdMs = kDefaultThresholdMs;
    std::uint64_t mLastFrame = 0;
    std::uint64_t mTotal = 0;
    LoadSummary mPending;
    Stall mRing[kRing];
    std::size_t mRingNext = 0;
    std::size_t mRingCount = 0;
};

// The process's log: written by the frame seam and read by the playtest capture, both on the game thread.
inline Log gLog;

// Threshold from PETARI_STALL_MS text: unset or empty is the default, 0 is off, junk is the default.
inline double thresholdFromText(const char* text) {
    if (text == nullptr || text[0] == '\0') return kDefaultThresholdMs;
    char* end = nullptr;
    const double v = std::strtod(text, &end);
    if (end == text || *end != '\0' || v < 0.0) return kDefaultThresholdMs;
    return v;
}

// One "[stall] ..." line (no newline). context is "" or e.g. "stage YosshiHomeGalaxy zone 5".
inline void formatLine(const Stall& s, const char* context, char* out, std::size_t size) {
    char causes[96];
    causeList(s, causes, sizeof(causes));
    char extra[96] = "";
    if (s.mask == 0) {
        std::snprintf(extra, sizeof(extra), "%s %.1f ms; ", FrameStats::partName(s.dominantPart), s.dominantUs / 1000.0);
    }
    char cpu[64] = "";
    if (s.gameCpuUs >= 0) {
        std::snprintf(cpu, sizeof(cpu), "; cpu game %.1f render %.1f ms", s.gameCpuUs / 1000.0,
                      s.renderCpuUs >= 0 ? s.renderCpuUs / 1000.0 : -1.0);
    }
    std::snprintf(out, size, "[stall] frame %llu %.1f ms phase=%s cause=%s (%spipelines %u, textures %u, loads %u%s)%s%s%s",
                  static_cast<unsigned long long>(s.frame), s.ms, FrameStats::phaseName(s.phase), causes, extra,
                  s.pipelines, s.textures, s.loads, cpu, s.unfocused ? " unfocused" : "", context[0] != '\0' ? " " : "", context);
}

inline void formatSummary(const LoadSummary& l, char* out, std::size_t size) {
    Stall tmp;
    tmp.mask = l.mask;
    tmp.primary = l.worstPrimary;
    char causes[96];
    causeList(tmp, causes, sizeof(causes));
    std::snprintf(out, size, "[stall] %llu frames over the threshold during loading, worst %.1f ms (frame %llu); causes %s",
                  static_cast<unsigned long long>(l.count), l.worstMs, static_cast<unsigned long long>(l.worstFrame), causes);
}

// The state.txt line(s) for the playtest capture: the stalls in the last window frames.
inline void formatCapture(const Log& log, std::uint64_t window, char* out, std::size_t size) {
    if (!log.enabled()) {
        std::snprintf(out, size, "stalls (last %llu frames): logger off (PETARI_STALL_MS=0)\n", static_cast<unsigned long long>(window));
        return;
    }
    Stall list[kRing];
    const std::size_t n = log.recent(window, list, kRing);
    std::size_t used = 0;
    auto put = [&](const char* fmt, auto... args) {
        if (used < size) used += static_cast<std::size_t>(std::snprintf(out + used, size - used, fmt, args...));
    };
    if (n == 0) {
        put("stalls (last %llu frames): none over %.0f ms\n", static_cast<unsigned long long>(window), log.thresholdMs());
        return;
    }
    put("stalls (last %llu frames): %s%zu over %.0f ms%s:\n", static_cast<unsigned long long>(window), n == kRing ? "at least " : "", n,
        log.thresholdMs(), n == kRing ? " (the newest are listed)" : "");
    for (std::size_t k = 0; k < n; k++) {
        char causes[96];
        causeList(list[k], causes, sizeof(causes));
        put("  stall frame %llu %.1f ms phase=%s cause=%s (pipelines %u, textures %u, loads %u)\n",
            static_cast<unsigned long long>(list[k].frame), list[k].ms, FrameStats::phaseName(list[k].phase), causes,
            list[k].pipelines, list[k].textures, list[k].loads);
    }
}

}  // namespace PetariNative::App::StallLog
