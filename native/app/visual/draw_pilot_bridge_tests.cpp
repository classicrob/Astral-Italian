#include "draw_pilot_bridge.hpp"
#include <cassert>
#include <cstring>
#include <memory>
using namespace PetariNative::Visual;
namespace {
Snapshot s{};
Snapshot fake(const GameSystem*,const MainLoopFramework*) noexcept { return s; }
Snapshot ready() {
    Snapshot v{};std::strcpy(v.key.scene,"Game");std::strcpy(v.key.stage,"RedBlueExGalaxy");
    v.key.placedScenario=v.key.selectedScenario=1;v.initState=6;
    v.controllerPresent=v.currentSceneNonnull=v.initializedGame=v.canonicalPlayerValid=1;return v;
}
void draw() { const auto* cb=drawObserverCallbacks();assert(cb);cb->loopEntry(nullptr,nullptr);cb->drawEntry(nullptr,nullptr);cb->drawExit(nullptr,nullptr); }
}
int main() {
    auto bridge=std::make_unique<DrawPilotBridge>();assert(bridge->postEndFrame({})==PilotStep::Disabled);
    assert(!bridge->configure(nullptr));assert(bridge->configure(fake));assert(!bridge->configure(fake));
    s=ready();s.initState=5;s.initializedGame=0;assert(bridge->postEndFrame(s)==PilotStep::WaitingReady);
    s=ready();assert(bridge->postEndFrame(s)==PilotStep::Continue);assert(bridge->anchor()==1);
    assert(bridge->inputIntervals()==0);assert(drawObserverCallbacks());
    for (unsigned i=1;i<=616;++i) {
        draw();const auto step=bridge->postEndFrame(s);
        assert(step==(i==616 ? PilotStep::StopRequired : PilotStep::Continue));
        assert(bridge->inputIntervals()==i);
    }
    assert(!drawObserverCallbacks());assert(bridge->observer().size()==2464);
    assert(bridge->observer().fault()==Fault::None);
    assert(bridge->postEndFrame(s)==PilotStep::StopRequired && bridge->inputIntervals()==616);
    bridge=std::make_unique<DrawPilotBridge>();assert(bridge->configure(fake));
    s=ready();s.key.placedScenario=2;assert(bridge->postEndFrame(s)==PilotStep::WaitingReady);
    s=ready();assert(bridge->postEndFrame(s)==PilotStep::Continue);
    assert(bridge->postEndFrame(s)==PilotStep::FaultStopRequired);assert(!drawObserverCallbacks());
}
