#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <iosfwd>
#include <type_traits>

class GameSystem;
class MainLoopFramework;

namespace PetariNative::Visual {
struct DrawCallbacks {
    void (*loopEntry)(const GameSystem*, const MainLoopFramework*) noexcept;
    void (*drawEntry)(const GameSystem*, const MainLoopFramework*) noexcept;
    void (*drawExit)(const GameSystem*, const MainLoopFramework*) noexcept;
};
const DrawCallbacks* drawObserverCallbacks() noexcept;

constexpr std::size_t DrawEventCapacity = 4096;
enum class Phase : std::uint8_t { LoopEntry, DrawEntry, DrawExit, PostEndFrame };
enum class Fault : std::uint8_t { None, Protocol, Capacity, InvalidSnapshot, SerialOverflow };
struct SceneKey {
    char scene[64];
    char stage[64];
    std::int32_t placedScenario;
    std::int32_t selectedScenario;
};
struct SourceContext {
    SceneKey key;
    std::uint64_t sourceFrame;
    std::uint64_t sampledEpoch;
    std::uint8_t valid;
};
struct Snapshot {
    SceneKey key;
    std::int32_t initState;
    std::int32_t bufferMode;
    std::int32_t renderInterval;
    std::int32_t renderCounter;
    std::int32_t drawingIndex;
    std::int32_t drawnIndex;
    std::int32_t displayingIndex;
    std::int32_t singleDrawingFlag;
    std::uint8_t controllerPresent;
    std::uint8_t currentSceneNonnull;
    std::uint8_t executedSceneNonnull;
    std::uint8_t executedSceneIsCurrent;
    std::uint8_t initializedGame;
    std::uint8_t canonicalPlayerValid;
    std::uint8_t frameworkPresent;
    std::uint8_t xfbPresent;
    std::uint8_t renderFlag;
    std::uint8_t invalid;
};
struct DrawEvent {
    Snapshot snapshot;
    SourceContext source;
    std::uint64_t eventSerial;
    std::uint64_t loopSerial;
    std::uint64_t drawSerial;
    Phase phase;
    std::uint8_t sourceKeyMatches;
};
static_assert(std::is_trivial_v<DrawEvent> && std::is_standard_layout_v<DrawEvent>);

using SnapshotReader = Snapshot (*)(const GameSystem*, const MainLoopFramework*) noexcept;
Snapshot readDrawSnapshot(const GameSystem*, const MainLoopFramework*) noexcept;

class DrawObserver {
public:
    bool begin(const SourceContext&) noexcept;
    void observe(Phase, const Snapshot&) noexcept;
    void postEndFrame(const SourceContext&) noexcept;
    Fault fault() const noexcept { return fault_; }
    std::size_t size() const noexcept { return size_; }
    const DrawEvent* data() const noexcept { return events_.data(); }
    bool installed() const noexcept { return installed_; }
private:
    friend bool installDrawObserver(DrawObserver&, SnapshotReader) noexcept;
    friend bool uninstallDrawObserver(DrawObserver&) noexcept;
    bool append(Phase, const Snapshot&) noexcept;
    std::array<DrawEvent, DrawEventCapacity> events_{};
    SourceContext source_{};
    std::uint64_t event_ = 0, loop_ = 0, draw_ = 0;
    std::size_t size_ = 0;
    Fault fault_ = Fault::None;
    Phase previous_ = Phase::PostEndFrame;
    bool begun_ = false, installed_ = false;
};

// Registration, callbacks, seam observations and removal have one Game-CPU owner.
// No concurrent access, registration from a callback, or observer destruction while installed.
bool installDrawObserver(DrawObserver&, SnapshotReader) noexcept;
bool uninstallDrawObserver(DrawObserver&) noexcept;
// Export only after removal, from detached storage; does not publish a provider receipt.
bool writeDrawObservations(std::ostream&, const DrawObserver&);
} // namespace PetariNative::Visual
