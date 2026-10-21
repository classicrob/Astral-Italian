#include "draw_observer.hpp"
#include <cstring>
#include <limits>

namespace PetariNative::Visual {
namespace {
DrawObserver* active = nullptr;
SnapshotReader reader = nullptr;
bool dispatching = false;
bool terminated(const char (&s)[64]) noexcept { return std::memchr(s, 0, sizeof(s)) != nullptr; }
bool keyValid(const SceneKey& key) noexcept { return terminated(key.scene) && terminated(key.stage); }
bool sameKey(const SceneKey& a, const SceneKey& b) noexcept {
    return std::strncmp(a.scene,b.scene,64)==0 && std::strncmp(a.stage,b.stage,64)==0 &&
        a.placedScenario==b.placedScenario && a.selectedScenario==b.selectedScenario;
}
void dispatch(Phase phase, const GameSystem* system, const MainLoopFramework* framework) noexcept {
    if (!active || !reader || active->fault()!=Fault::None) return;
    if (dispatching) { active->observe(Phase::PostEndFrame, {}); return; }
    dispatching = true;
    const auto snapshot = reader(system,framework);
    active->observe(phase,snapshot);
    dispatching = false;
}
void loopEntry(const GameSystem* s,const MainLoopFramework* f) noexcept { dispatch(Phase::LoopEntry,s,f); }
void drawEntry(const GameSystem* s,const MainLoopFramework* f) noexcept { dispatch(Phase::DrawEntry,s,f); }
void drawExit(const GameSystem* s,const MainLoopFramework* f) noexcept { dispatch(Phase::DrawExit,s,f); }
const DrawCallbacks hooks{loopEntry,drawEntry,drawExit};
}
const DrawCallbacks* drawObserverCallbacks() noexcept { return active ? &hooks : nullptr; }

bool DrawObserver::begin(const SourceContext& source) noexcept {
    if (installed_ || source.valid!=1 || !keyValid(source.key)) return false;
    source_=source;size_=0;event_=loop_=draw_=0;fault_=Fault::None;
    previous_=Phase::PostEndFrame;begun_=true;
    return true;
}
bool DrawObserver::append(Phase phase,const Snapshot& snapshot) noexcept {
    if (size_==events_.size()) { fault_=Fault::Capacity;return false; }
    if (event_==std::numeric_limits<std::uint64_t>::max()) { fault_=Fault::SerialOverflow;return false; }
    DrawEvent event{};event.snapshot=snapshot;event.source=source_;event.eventSerial=++event_;
    event.loopSerial=loop_;event.drawSerial=draw_;event.phase=phase;
    event.sourceKeyMatches=phase!=Phase::PostEndFrame && sameKey(snapshot.key,source_.key);
    events_[size_++]=event;
    return true;
}
void DrawObserver::observe(Phase phase,const Snapshot& snapshot) noexcept {
    if (!begun_ || fault_!=Fault::None) return;
    const bool ordered=(phase==Phase::LoopEntry && previous_==Phase::PostEndFrame) ||
        (phase==Phase::DrawEntry && previous_==Phase::LoopEntry) ||
        (phase==Phase::DrawExit && previous_==Phase::DrawEntry);
    if (!ordered) { fault_=Fault::Protocol;return; }
    if (phase==Phase::LoopEntry) {
        if (loop_==std::numeric_limits<std::uint64_t>::max()) { fault_=Fault::SerialOverflow;return; }
        ++loop_;
    }
    if (phase==Phase::DrawEntry) {
        if (draw_==std::numeric_limits<std::uint64_t>::max()) { fault_=Fault::SerialOverflow;return; }
        ++draw_;
    }
    if (!append(phase,snapshot)) return;
    if (snapshot.invalid || !keyValid(snapshot.key)) { fault_=Fault::InvalidSnapshot;return; }
    previous_=phase;
}
void DrawObserver::postEndFrame(const SourceContext& source) noexcept {
    if (!begun_ || fault_!=Fault::None) return;
    if (previous_!=Phase::DrawExit || source.valid!=1 || !keyValid(source.key) ||
        source_.sourceFrame==std::numeric_limits<std::uint64_t>::max() || source.sourceFrame!=source_.sourceFrame+1) {
        fault_=Fault::Protocol;return;
    }
    source_=source;
    if (append(Phase::PostEndFrame,{})) previous_=Phase::PostEndFrame;
}
bool installDrawObserver(DrawObserver& observer,SnapshotReader sampler) noexcept {
    if (active || dispatching || !sampler || !observer.begun_ || observer.fault_!=Fault::None) return false;
    active=&observer;reader=sampler;observer.installed_=true;return true;
}
bool uninstallDrawObserver(DrawObserver& observer) noexcept {
    if (active!=&observer || dispatching) return false;
    active=nullptr;reader=nullptr;observer.installed_=false;return true;
}
} // namespace PetariNative::Visual
