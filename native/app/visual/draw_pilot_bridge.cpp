#include "draw_pilot_bridge.hpp"
#include <cstring>
#include <limits>
namespace PetariNative::Visual {
namespace {
bool sameKey(const SceneKey& a,const SceneKey& b) noexcept {
    return std::strncmp(a.scene,b.scene,64)==0 && std::strncmp(a.stage,b.stage,64)==0 &&
        a.placedScenario==b.placedScenario && a.selectedScenario==b.selectedScenario;
}
bool ready(const Snapshot& s) noexcept {
    return s.controllerPresent==1 && s.currentSceneNonnull==1 && s.initializedGame==1 &&
        s.canonicalPlayerValid==1 && s.initState==6 && std::strcmp(s.key.scene,"Game")==0 &&
        std::strcmp(s.key.stage,"RedBlueExGalaxy")==0 && s.key.placedScenario==1;
}
}
bool DrawPilotBridge::configure(SnapshotReader reader) noexcept {
    if (reader_ || seams_ || !reader || stopped_) return false;
    reader_=reader;return true;
}
PilotStep DrawPilotBridge::stop(bool fault) noexcept {
    if (observer_.installed() && !uninstallDrawObserver(observer_)) fault=true;
    capturing_=false;stopped_=true;faulted_=faulted_ || fault;
    return faulted_ ? PilotStep::FaultStopRequired : PilotStep::StopRequired;
}
PilotStep DrawPilotBridge::postEndFrame(const Snapshot& snapshot) noexcept {
    if (!reader_) return PilotStep::Disabled;
    if (stopped_) return faulted_ ? PilotStep::FaultStopRequired : PilotStep::StopRequired;
    if (snapshot.invalid || !std::memchr(snapshot.key.scene,0,64) || !std::memchr(snapshot.key.stage,0,64) ||
        seams_==std::numeric_limits<std::uint64_t>::max()) return stop(true);
    if (!context_.valid || !sameKey(snapshot.key,context_.key)) {
        if (context_.sampledEpoch==std::numeric_limits<std::uint64_t>::max()) return stop(true);
        ++context_.sampledEpoch;
    }
    context_.key=snapshot.key;context_.sourceFrame=seams_++;context_.valid=1;
    if (!capturing_) {
        if (!ready(snapshot)) return PilotStep::WaitingReady;
        anchor_=context_.sourceFrame;
        if (!observer_.begin(context_) || !installDrawObserver(observer_,reader_)) return stop(true);
        capturing_=true;return PilotStep::Continue;
    }
    observer_.postEndFrame(context_);
    if (observer_.fault()!=Fault::None) return stop(true);
    ++intervals_;
    if (intervals_==616) return stop(false);
    return PilotStep::Continue;
}
} // namespace PetariNative::Visual
