#pragma once
#include "draw_observer.hpp"
namespace PetariNative::Visual {
enum class PilotStep : std::uint8_t { Disabled, WaitingReady, Continue, StopRequired, FaultStopRequired };
class DrawPilotBridge {
public:
    bool configure(SnapshotReader) noexcept;
    PilotStep postEndFrame(const Snapshot&) noexcept;
    const DrawObserver& observer() const noexcept { return observer_; }
    bool configured() const noexcept { return reader_!=nullptr; }
    bool stopped() const noexcept { return stopped_; }
    std::uint64_t anchor() const noexcept { return anchor_; }
    std::uint64_t inputIntervals() const noexcept { return intervals_; }
private:
    PilotStep stop(bool fault) noexcept;
    DrawObserver observer_;
    SnapshotReader reader_=nullptr;
    SourceContext context_{};
    std::uint64_t seams_=0,anchor_=0,intervals_=0;
    bool capturing_=false,stopped_=false,faulted_=false;
};
// Opt-in owner call BEFORE first Game loop. No registration/environment hook is installed.
using PilotStopCallback = void (*)(const DrawPilotBridge&) noexcept;
bool attachDrawPilotHost(DrawPilotBridge&, PilotStopCallback) noexcept;
// Same owner, post-endFrame before CPU release. Default inactive path reads no Game objects.
void drawPilotHostPostEndFrame() noexcept;
} // namespace PetariNative::Visual
