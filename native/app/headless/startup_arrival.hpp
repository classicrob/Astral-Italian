#pragma once
#include "trace.hpp"
#include <string>

namespace PetariNative::Headless {
class StartupArrival {
public:
    bool observe(const State& state, const std::string& stage, int placedScenario) {
        if (mObserved) return true;
        const auto equals = [&](const char* key, const std::string& expected) {
            const auto value = state.fields.find(key);
            return value != state.fields.end() && value->second == expected;
        };
        mObserved = placedScenario > 0 && equals("scene_valid", "1") &&
            equals("scene", "Game") && equals("init_state", "6") &&
            equals("player_valid", "1") && equals("stage", stage) &&
            equals("scenario", std::to_string(placedScenario));
        return mObserved;
    }
    bool observed() const { return mObserved; }
private:
    bool mObserved = false;
};
}
