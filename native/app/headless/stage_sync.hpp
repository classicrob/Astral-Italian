#pragma once
#include "trace.hpp"
#include <functional>
#include <optional>

namespace PetariNative::Headless {
struct StageSyncOptions {
    std::string stage;
    std::uint32_t scenario = 0;
    std::optional<std::uint32_t> seed;
};
std::uint32_t syncInteger(const std::string& value);
class StageSync {
public:
    explicit StageSync(StageSyncOptions options = {});
    using ForceSeed = std::function<State(std::uint32_t)>;
    std::optional<std::uint64_t> select(State& state, std::uint64_t sourceFrame,
                                      std::uint64_t ticks, std::uint64_t viCount,
                                      const ForceSeed& forceSeed = {});
    bool enabled() const { return !mOptions.stage.empty(); }
    bool started() const { return mOrigin.has_value(); }
    void writeOrigin(std::ostream& out) const;
private:
    bool matches(const State& state) const;
    struct Origin {
        std::uint64_t frame, epoch, ticks, viCount;
        std::uint32_t seed, seedBefore;
    };
    StageSyncOptions mOptions;
    std::optional<Origin> mOrigin;
};
void setGameRngSeed(std::uint32_t seed);
}
