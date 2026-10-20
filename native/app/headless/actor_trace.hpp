#pragma once

#include <array>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <map>
#include <memory>
#include <optional>
#include <ostream>
#include <set>
#include <string>
#include <vector>

namespace PetariNative::Headless {
struct State;
constexpr std::size_t ActorNameStorageLimit = 1024; // Includes terminating NUL.
struct ActorSample {
    std::uintptr_t localPointer = 0; // Internal continuity token; NEVER serialized.
    bool isPlayer = false;           // Actual MarioHolder::mActor equality only.
    bool nameValid = false;
    std::string nameBytes;
    std::int16_t linkId = -1, linkZone = -1, linkContext = -1;
    bool dead = false;
    std::array<float, 3> position{}, rotation{}, scale{};
};
struct ActorFrame {
    bool ready = false;
    std::uint32_t registered = 0, living = 0;
    std::vector<ActorSample> actors;
};
const std::vector<std::string>& actorTraceColumns();
const std::vector<std::string>& actorFrameColumns();
bool actorFrameReady(const State& state);
// Typed game census, defined in observe.cpp. Call only at the accepted frame seam.
ActorFrame observeActors(const State& state);

class ActorTrace {
public:
    ActorTrace(std::ostream& actors, std::ostream& frames);
    void write(std::uint64_t frame, const State& state, const ActorFrame& actors);
    void finish();
private:
    struct Continuity {
        std::uint64_t handle = 0;
        std::string key;
        bool changed = false;
    };
    std::ostream& mActors;
    std::ostream& mFrames;
    std::uint64_t mNextFrame = 0, mNextHandle = 1;
    std::optional<std::uint64_t> mEpoch;
    std::map<std::uintptr_t, Continuity> mActive;
    std::map<std::string, std::set<std::uint64_t>> mKeyOwners;
    bool mFinished = false;
};

// Caller supplies an already validated, external trace base path. No destructor
// publishes partial captures. Main completion must be written AFTER finish().
class ActorTraceFiles {
public:
    explicit ActorTraceFiles(const std::filesystem::path& trace);
    void write(std::uint64_t frame, const State& state, const ActorFrame& actors);
    void finish();
private:
    std::filesystem::path mActorPath, mFramePath;
    std::ofstream mActors, mFrames;
    std::unique_ptr<ActorTrace> mTrace;
    bool mFinished = false;
};
} // namespace PetariNative::Headless
