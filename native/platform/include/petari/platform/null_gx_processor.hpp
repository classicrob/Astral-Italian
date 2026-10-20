#pragma once
// Ordered processor for the headless adapter's byte-counted FIFO. The caller
// serializes writers, breakpoint changes and pump() with the OS interrupt lock.
// Writing and processing are distinct even when both happen on the calling
// thread: a breakpoint can hold processing while later writes/events queue up.
#include <algorithm>
#include <cstdint>
#include <deque>
#include <stdexcept>

namespace PetariNative::Platform {
class NullGxProcessor {
public:
    enum class Kind { Token, DrawDone };
    struct Event { Kind kind; std::uint16_t token; std::uint64_t position; };
    static constexpr std::uint64_t NoBreakpoint = UINT64_MAX;

    void publish(std::uint64_t written) {
        if (written < mWritten) throw std::logic_error("null GX written position regressed");
        mWritten = written;
    }
    void append(Kind kind, std::uint16_t token, std::uint64_t position) {
        if (position <= mProcessed || position > mWritten ||
            (!mEvents.empty() && position < mEvents.back().position))
            throw std::logic_error("null GX event is not an unprocessed stream command");
        mEvents.push_back({kind, token, position});
    }
    template<class Deliver, class Progress, class Reached>
    void pump(std::uint64_t breakpoint, Deliver deliver, Progress progress, Reached reached) {
        // Report an event ending at the breakpoint before reporting that the
        // processor stopped there. An address BEHIND the cursor is not a hit.
        while (mProcessed < mWritten) {
            if (breakpoint != NoBreakpoint && breakpoint == mProcessed) break;
            auto target = mEvents.empty() ? mWritten : mEvents.front().position;
            if (breakpoint > mProcessed && breakpoint < target) target = breakpoint;
            mProcessed = target;
            if (!mEvents.empty() && mEvents.front().position == mProcessed) {
                const auto event = mEvents.front();
                mEvents.pop_front();
                deliver(event);
            }
            if (breakpoint != NoBreakpoint && breakpoint == mProcessed) break;
        }
        // Progress and any reached callback follow all events through this
        // cursor. The platform still enforces monotonic event-report order.
        progress(mProcessed, mWritten);
        if (breakpoint != NoBreakpoint && breakpoint == mProcessed) reached(mProcessed);
    }
    void abort(std::uint64_t written) {
        publish(written);
        mEvents.clear();
        mProcessed = mWritten;
    }
    std::uint64_t processed() const { return mProcessed; }
    std::uint64_t written() const { return mWritten; }
    std::size_t pending() const { return mEvents.size(); }
private:
    std::uint64_t mWritten = 0, mProcessed = 0;
    std::deque<Event> mEvents;
};
} // namespace PetariNative::Platform
