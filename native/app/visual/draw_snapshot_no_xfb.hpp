#pragma once
#include "draw_observer.hpp"
namespace PetariNative::Visual {
// XFB/VI-owned state is explicitly unavailable; no pixel/provider qualification.
Snapshot readDrawSnapshotNoXfb(const GameSystem*, const MainLoopFramework*) noexcept;
}
