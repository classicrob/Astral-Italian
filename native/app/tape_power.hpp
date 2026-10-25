#pragma once
// Session-control extension to schema2. It records a real host power request;
// it does not move scene/input anchors or omit teardown rows.
#include <cstdint>
#include <string>
namespace PetariNative::App::TapePower {
inline constexpr const char* kProtocol = "native-post-seam-power-v1";
inline constexpr const char* kBoundary = "after-route-seam-before-next-update";
struct Event {
    std::uint64_t id=0, phase=0, afterRow=0, beforeInputFrame=0;
    std::string kind;
    bool callbackObserved=false;
    double neutralElapsedSeconds=0;
};
} // namespace PetariNative::App::TapePower
