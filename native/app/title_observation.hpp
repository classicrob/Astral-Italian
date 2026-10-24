#pragma once
// The tester's actual first launch: no input, real preparation completion,
// thirty seconds after completion, hard cap fifteen minutes. No game clock use.
namespace PetariNative::App {
class TitleObservation {
    double completedAt = -1;
    double minimumSeconds = 0;
    double stopArmedAt = -1;
    bool stopped = false;
public:
    explicit TitleObservation(double minimum = 0) : minimumSeconds(minimum) {}
    enum class Step { Running, Complete, TimedOut };
    Step tick(double elapsed, bool complete) {
        if (stopped) return Step::Running;
        if (elapsed >= 900) { stopped = true; return Step::TimedOut; }
        if (complete && completedAt < 0) completedAt = elapsed;
        if (completedAt >= 0 && elapsed - completedAt >= 30 && elapsed >= minimumSeconds) {
            // Finish on a subsequent seam, so the CSV has a complete interval
            // beyond the thirty-second boundary before normal exit flushes it.
            if (stopArmedAt >= 0 && elapsed > stopArmedAt) { stopped = true; return Step::Complete; }
            stopArmedAt = elapsed;
        }
        return Step::Running;
    }
    double completionSeconds() const { return completedAt; }
};
} // namespace PetariNative::App
