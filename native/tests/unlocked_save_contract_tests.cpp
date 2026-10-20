// Native protocol tests. These synthetic records are NOT game save verification.
#include <petari/unlocked_save.hpp>
#include <petari/unlocked_save_paths.hpp>
#include <fstream>
#include <cassert>
#include <functional>
#include <iostream>

namespace Save = PetariNative::UnlockedSave;

static Save::Inventory fixture() {
    Save::Inventory inventory{true, 4, {
        {{"TestA", 1}, 1, false, true, {1, 4}},
        {{"TestA", 2}, 1, true, true, {7}},
        {{"TestB", 1}, 6, false, true, {2}},
        {{"TestS", 1}, 7, false, true, {8}}
    }, {}};
    for (auto profile : {Save::Profile::Worlds1To6, Save::Profile::Postgame, Save::Profile::AllStars}) {
        Save::ProfileSupport support{true, {}};
        for (const auto& id : Save::requiredChecks(profile))
            support.checks.push_back({id, true, "synthetic test getter", {}});
        inventory.profiles[profile] = support;
    }
    return inventory;
}

static void refuses(const std::function<void()>& operation) {
    bool rejected = false;
    try { operation(); } catch (const std::invalid_argument&) { rejected = true; }
    assert(rejected);
}

class TestBackend final : public Save::Backend {
public:
    Save::Inventory inventory = fixture();
    Save::Snapshot current{{}, 0, {}};
    Save::RoundTrip trip{true, true, true, true, true, true, true, false, false};
    bool applied = false;
    bool saved = false;
    Save::Inventory enumerate() override { return inventory; }
    Save::Snapshot readSnapshot() override { return current; }
    void apply(const Save::Plan& plan) override {
        applied = true;
        current.owned.clear();
        for (const auto& star : plan.stars) current.owned[star] = false;
        current.gameOwnedCount = static_cast<int>(current.owned.size());
        for (const auto& check : plan.checks)
            current.checks[check.id] = check.applicable ? Save::CheckResult::Pass : Save::CheckResult::NotApplicable;
    }
    Save::RoundTrip saveAndReload() override { saved = true; return trip; }
};

int main(int argc, char** argv) {
    assert(argc == 2); // Caller supplies a fresh scratch path; no personal filesystem access.
    namespace fs = std::filesystem;
    const fs::path root = argv[1];
    assert(!fs::exists(root));
    fs::create_directories(root / "isolated/NAND");
    const auto home = root / "fake-home";
    const auto user = root / "isolated";
    std::string error;
    assert(!Save::validateUserDirectory(user, home, &error));
    std::ofstream(user / ".petari-make-unlocked-save") << "test marker, NOT a game run";
    assert(Save::validateUserDirectory(user, home, &error));
    fs::create_directory_symlink(root, user / "NAND/escape");
    assert(!Save::validateUserDirectory(user, home, &error));
    fs::remove(user / "NAND/escape");
    fs::create_directory_symlink(user, root / "linked");
    assert(!Save::validateUserDirectory(root / "linked", home, &error));
    assert(!Save::validateUserDirectory(home / "Library/Application Support/Petari/subdir", home, &error));
    assert(!Save::privateServerDataPath(root / "build/save"));
    assert(!Save::privateServerDataPath(root / "MacioGalaxy2-team/catalog"));
    fs::create_directory(root / "repo");
    std::ofstream(root / "repo/.git") << "gitdir: synthetic worktree";
    assert(!Save::privateServerDataPath(root / "repo/private-save"));
    fs::remove_all(root);

    const auto inv = fixture();
    assert(Save::makePlan(inv, Save::Profile::Worlds1To6).stars.size() == 2);
    assert(Save::makePlan(inv, Save::Profile::Postgame).stars.size() == 3);
    assert(Save::makePlan(inv, Save::Profile::AllStars).stars.size() == 4);
    auto bad = inv;
    bad.stars[3] = bad.stars[0];
    refuses([&] { Save::makePlan(bad, Save::Profile::AllStars); });
    bad = inv;
    bad.profiles[Save::Profile::Postgame].checks.clear();
    refuses([&] { Save::makePlan(bad, Save::Profile::Postgame); });
    bad = inv;
    bad.profiles[Save::Profile::AllStars].supported = false;
    refuses([&] { Save::makePlan(bad, Save::Profile::AllStars); });

    TestBackend backend;
    assert(Save::execute(backend, Save::Profile::AllStars));
    assert(backend.applied && backend.saved);
    backend = TestBackend{};
    backend.current = {{{{"TestA", 2}, false}}, 1, {}};
    refuses([&] { Save::execute(backend, Save::Profile::Worlds1To6); });
    assert(!backend.applied && !backend.saved); // Reject high-progress seeds BEFORE mutation.
    backend = TestBackend{};
    backend.trip.regularLoaderSucceeded = false;
    assert(!Save::execute(backend, Save::Profile::AllStars));
    backend = TestBackend{};
    backend.trip.unrelatedSlotsUnchanged = false;
    assert(!Save::execute(backend, Save::Profile::AllStars));

    backend = TestBackend{};
    const auto plan = Save::makePlan(inv, Save::Profile::AllStars);
    backend.apply(plan);
    auto after = backend.current;
    after.owned.begin()->second = true;
    assert(!Save::verify(inv, plan, {{}, 0, {}}, after, backend.trip));
    after = backend.current;
    after.gameOwnedCount++;
    assert(!Save::verify(inv, plan, {{}, 0, {}}, after, backend.trip));
    auto forged = plan;
    forged.checks.clear();
    assert(!Save::verify(inv, forged, {{}, 0, {}}, backend.current, backend.trip));
    std::cout << "unlocked-save synthetic contract checks PASS (NOT game verification)\n";
}
