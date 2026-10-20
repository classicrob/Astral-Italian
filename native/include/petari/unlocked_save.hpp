#pragma once

#include <algorithm>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace PetariNative::UnlockedSave {
// CLI transport only. Empty means inactive. Legacy SMG1 variants are unsupported.
inline std::string variant;
inline int slot = 1;
inline bool acceptsAction(const std::string& value) {
    return value == "inventory" || value == "worlds1-6" || value == "postgame" || value == "all-stars";
}

enum class Profile { Worlds1To6, Postgame, AllStars };
using StarKey = std::pair<std::string, int>; // Actual (galaxy, starId), NOT ScenarioNo or PowerStarId mask.
struct Star {
    StarKey key;
    int world;
    bool green;
    bool engineValidated;
    std::vector<int> scenarios; // Actual catalog ScenarioNo associations, possibly multiple per star.
};
struct Check {
    std::string id;
    bool applicable;
    std::string provenance;
    std::string notApplicableReason;
};
struct ProfileSupport {
    bool supported;
    std::vector<Check> checks;
};
struct Inventory {
    bool complete;
    int gameMaximum; // Actual GameDataFunction::getPowerStarNumMax(), cross-checked with unique save records.
    std::vector<Star> stars;
    std::map<Profile, ProfileSupport> profiles;
};
struct Plan {
    Profile profile;
    std::set<StarKey> stars;
    std::vector<Check> checks;
};
using Owned = std::map<StarKey, bool>; // bool = bronze; selected stars must become real (false).
enum class CheckResult { Pass, Fail, NotApplicable, Unknown };
struct Snapshot {
    Owned owned;
    int gameOwnedCount;
    std::map<std::string, CheckResult> checks;
};
struct RoundTrip {
    bool saveCompleted;
    bool regularLoaderSucceeded;
    bool headerValid;
    bool versionValid;
    bool checksumValid;
    bool bytesEqual;
    bool unrelatedSlotsUnchanged;
    bool gameDataCorrupted;
    bool configDataCorrupted;
};

inline std::set<std::string> requiredChecks(Profile profile) {
    if (profile == Profile::Worlds1To6) return {"worlds-1-6-open"};
    if (profile == Profile::Postgame) return {"worlds-1-6-open", "postgame", "world-s-open"};
    if (profile == Profile::AllStars)
        return {"worlds-1-6-open", "postgame", "world-s-open", "green-stars", "luigi", "master-luma"};
    throw std::invalid_argument("unknown unlocked-save profile");
}

inline Plan makePlan(const Inventory& inventory, Profile profile) {
    if (!inventory.complete || inventory.gameMaximum <= 0 ||
        inventory.stars.size() != static_cast<std::size_t>(inventory.gameMaximum))
        throw std::invalid_argument("incomplete engine star inventory");
    const auto support = inventory.profiles.find(profile);
    if (support == inventory.profiles.end() || !support->second.supported)
        throw std::invalid_argument("unresolved progression profile");
    Plan plan{profile, {}, support->second.checks};
    std::set<StarKey> seen;
    for (const auto& star : inventory.stars) {
        if (star.key.first.empty() || star.key.second < 1 || star.key.second > 32 ||
            star.world < 1 || star.world > 7 || !star.engineValidated || star.scenarios.empty() ||
            !seen.insert(star.key).second)
            throw std::invalid_argument("invalid/duplicate resolved star identity");
        std::set<int> scenarios;
        for (const int number : star.scenarios)
            if (number <= 0 || !scenarios.insert(number).second)
                throw std::invalid_argument("invalid scenario associations");
        if (profile == Profile::AllStars || (!star.green &&
            (profile == Profile::Postgame || star.world <= 6))) plan.stars.insert(star.key);
    }
    std::set<std::string> checks;
    for (const auto& check : plan.checks) {
        if (check.id.empty() || check.provenance.empty() || !checks.insert(check.id).second ||
            (!check.applicable && (check.notApplicableReason.empty() ||
             (check.id != "luigi" && check.id != "master-luma"))))
            throw std::invalid_argument("unresolved/duplicate progression check");
    }
    for (const auto& id : requiredChecks(profile))
        if (!checks.count(id)) throw std::invalid_argument("missing required progression check");
    if (plan.stars.empty()) throw std::invalid_argument("empty save plan");
    return plan;
}

inline bool verify(const Inventory& inventory, const Plan& plan, const Snapshot& before,
                   const Snapshot& after, const RoundTrip& trip) {
    if (!trip.saveCompleted || !trip.regularLoaderSucceeded || !trip.headerValid || !trip.versionValid ||
        !trip.checksumValid || !trip.bytesEqual || !trip.unrelatedSlotsUnchanged ||
        trip.gameDataCorrupted || trip.configDataCorrupted) return false;
    try {
        const auto canonical = makePlan(inventory, plan.profile);
        if (plan.stars != canonical.stars || plan.checks.size() != canonical.checks.size()) return false;
        for (std::size_t i = 0; i < plan.checks.size(); ++i) {
            const auto& a = plan.checks[i];
            const auto& b = canonical.checks[i];
            if (a.id != b.id || a.applicable != b.applicable || a.provenance != b.provenance ||
                a.notApplicableReason != b.notApplicableReason) return false;
        }
    } catch (const std::invalid_argument&) {
        return false;
    }
    std::set<StarKey> valid;
    for (const auto& star : inventory.stars) valid.insert(star.key);
    for (const auto& item : before.owned)
        if (!valid.count(item.first) || !plan.stars.count(item.first)) return false;
    Owned expected;
    for (const auto& star : plan.stars) {
        if (!valid.count(star)) return false;
        expected[star] = false;
    }
    if (before.gameOwnedCount != static_cast<int>(before.owned.size()) ||
        after.owned != expected || after.gameOwnedCount != static_cast<int>(expected.size()) ||
        after.checks.size() != plan.checks.size()) return false;
    for (const auto& check : plan.checks) {
        const auto result = after.checks.find(check.id);
        if (result == after.checks.end() || result->second !=
            (check.applicable ? CheckResult::Pass : CheckResult::NotApplicable)) return false;
    }
    return true;
}

// Runtime provider must use actual GameDataHolder/GameDataFunction APIs, then
// SaveDataHandleSequence persistent save and regular loader + corruption checks.
// No provider is installed by this header; an absent adapter must fail, not emit
// a VERIFIED log/receipt. A test double is never game save verification.
class Backend {
public:
    virtual ~Backend() = default;
    virtual Inventory enumerate() = 0;
    virtual Snapshot readSnapshot() = 0;
    virtual void apply(const Plan&) = 0;
    virtual RoundTrip saveAndReload() = 0;
};
inline bool execute(Backend& backend, Profile profile) {
    const auto inventory = backend.enumerate();
    const auto plan = makePlan(inventory, profile);
    const auto before = backend.readSnapshot();
    if (before.gameOwnedCount != static_cast<int>(before.owned.size()))
        throw std::invalid_argument("inconsistent seed snapshot");
    for (const auto& entry : before.owned)
        if (!plan.stars.count(entry.first))
            throw std::invalid_argument("seed has out-of-profile star identity");
    backend.apply(plan);
    const auto trip = backend.saveAndReload();
    const auto after = backend.readSnapshot();
    return verify(inventory, plan, before, after, trip);
}
} // namespace PetariNative::UnlockedSave
