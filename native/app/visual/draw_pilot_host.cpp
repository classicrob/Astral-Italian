#include "draw_pilot_bridge.hpp"
#include "draw_snapshot_no_xfb.hpp"
#include "Game/System/GameSystem.hpp"
#include "Game/System/MainLoopFramework.hpp"
#include "Game/Util/SingletonHolder.hpp"
#include <cstdlib>
namespace PetariNative::Visual {
namespace {
DrawPilotBridge* activePilot=nullptr;
PilotStopCallback stopCallback=nullptr;
bool sawHostSeam=false;
}
bool attachDrawPilotHost(DrawPilotBridge& pilot,PilotStopCallback stop) noexcept {
    if (sawHostSeam || activePilot || !stop || pilot.configured() || pilot.stopped()) return false;
    if (!pilot.configure(readDrawSnapshotNoXfb)) return false;
    activePilot=&pilot;stopCallback=stop;return true;
}
void drawPilotHostPostEndFrame() noexcept {
    sawHostSeam=true;
    if (!activePilot) return;
    const auto snapshot=readDrawSnapshotNoXfb(SingletonHolder<GameSystem>::get(),MainLoopFramework::getManager());
    const auto step=activePilot->postEndFrame(snapshot);
    if (step!=PilotStep::StopRequired && step!=PilotStep::FaultStopRequired) return;
    const auto* detached=activePilot;const auto stop=stopCallback;
    activePilot=nullptr;stopCallback=nullptr;
    stop(*detached);
    // A returning stop callback must never let Game execute interval617.
    std::abort();
}
} // namespace PetariNative::Visual
