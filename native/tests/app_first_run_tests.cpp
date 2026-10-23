#include "app_locations.hpp"
#include <cstdio>
#include <chrono>
#include <cstdlib>
#include <fstream>
#include <string>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#ifdef __APPLE__
#include <sys/xattr.h>
#endif

namespace fs = std::filesystem;
namespace App = PetariNative::App;
namespace {
int failures = 0;
void check(bool condition, const char* message) {
    if (!condition) { std::fprintf(stderr, "FAIL: %s\n", message); ++failures; }
}
void put(const fs::path& path, const std::string& text = "synthetic") {
    fs::create_directories(path.parent_path());
    std::ofstream(path) << text;
}
std::string get(const fs::path& path) { std::ifstream f(path); return {std::istreambuf_iterator<char>(f), {}}; }
void configTests(const fs::path& root) {
    const App::AppLocations locations{root / "config space"};
    check(App::AppLocations::defaultRoot(root) == root / "Library/Application Support/Astral Italian", "new default root includes the space");
    check(locations.gameData() == locations.root / "Game Data/SB4E01", "game-data location");
    check(App::readAppConfig(locations).empty(), "missing config is a first run");
    App::AppConfig config{{"disc", "/synthetic/folder = one"}, {"disc_source", "folder"}, {"future", "kept"}};
    App::writeAppConfig(locations, config);
    check(App::readAppConfig(locations) == config, "config preserves spaces, equals signs and unknown choices");
    config["disc"] = "second";
    App::writeAppConfig(locations, config);
    check(App::readAppConfig(locations) == config, "config atomic replacement");
    config["disc"] = "bad\npath";
    bool rejected = false;
    try { App::writeAppConfig(locations, config); } catch (...) { rejected = true; }
    check(rejected && App::readAppConfig(locations).at("disc") == "second", "invalid config write preserves previous complete file");
    for (const auto& entry : fs::directory_iterator(locations.root)) check(entry.path().filename() == "app.cfg", "no temporary config left behind");
    put(locations.config(), "damaged settings\n");
    rejected = false;
    try { App::readAppConfig(locations); } catch (...) { rejected = true; }
    check(rejected, "malformed config reports damage instead of silently forgetting choices");
    setenv("ASTRAL_SUPPORT_DIR", locations.root.c_str(), 1);
    check(App::AppLocations::fromEnvironment().root == locations.root, "isolated support override");
    unsetenv("ASTRAL_SUPPORT_DIR");
    const auto home = root / "isolation-home";
    const auto other = home / "Library/Application Support/Petari";
    check(App::isOtherGameUserPath(other, home) && App::isOtherGameUserPath(other / "subfolder", home), "other game root and descendants are refused");
    check(!App::isOtherGameUserPath(home / "Library/Application Support/Petari-other", home), "refusal compares path components");
    fs::create_directories(other);
    fs::create_directory_symlink(other, home / "linked-other");
    check(App::isOtherGameUserPath(home / "linked-other/NAND", home), "other game symlinks are refused");
}
void discTests(const fs::path& root) {
    const auto valid = root / "remembered";
    put(valid / "synthetic-disc");
    const auto dev = root / "build/game-data/SB4E01";
    fs::create_directories(dev);
    App::AppConfig config{{"disc", valid.string()}};
    check(App::resolveDisc("argument", "environment", config, root).disc == "argument", "--disc wins");
    check(App::resolveDisc({}, "environment", config, root).disc == "environment", "environment wins over remembered");
    check(App::resolveDisc({}, nullptr, config, root).disc == valid, "validated remembered path wins over developer default");
    config["disc"] = (root / "missing").string();
    const auto bad = App::resolveDisc({}, nullptr, config, root);
    check(bad.disc.empty() && bad.problem.find("missing") != std::string::npos, "invalid remembered path reports why and forces setup");
    check(App::resolveDisc({}, nullptr, {}, root).disc == dev, "developer default only when present");
    fs::remove_all(dev);
    check(App::resolveDisc({}, nullptr, {}, root).disc.empty(), "no disc means first-run flow");
}
void migrationTests(const fs::path& root) {
    const auto home = root / "home";
    const App::AppLocations locations{App::AppLocations::defaultRoot(home)};
    const auto source = home / "MacioGalaxy2-play/user";
    const auto legacy = home / "Library/Application Support/AstralItalian";
    put(source / "NAND/save.bin", "save");
    put(legacy / "NAND/other.bin", "legacy");
    put(source / "mods.txt", "old mods"); put(source / "settings.txt", "old settings");
    put(source / "mods-saves/example.json", "old sidecar");
    put(locations.root / "mods.txt", "new mods");
    put(locations.root / "mods-saves/example.json", "new sidecar");
    // An existing symlink must not be followed, replaced or written through.
    put(root / "keep-controls", "keep");
    fs::create_symlink(root / "keep-controls", locations.root / "controls.txt");
    put(source / "controls.txt", "old controls");
#ifdef __APPLE__
    const char metadata[] = "synthetic metadata";
    setxattr((source / "NAND/save.bin").c_str(), "com.petari.nand", metadata, sizeof(metadata), 0, 0);
#endif
    App::AppConfig config;
    check(App::migrationSource(locations, home, config) == source, "development saves have priority");
    App::migrateUserState(locations, source, true, &config);
    check(get(locations.root / "NAND/save.bin") == "save" && get(source / "NAND/save.bin") == "save", "migration copies saves and preserves the source");
    check(get(locations.root / "mods.txt") == "new mods" && get(locations.root / "mods-saves/example.json") == "new sidecar", "migration cannot replace existing files or merge existing directories");
    check(get(root / "keep-controls") == "keep" && fs::is_symlink(locations.root / "controls.txt"), "migration leaves existing symlink alone");
    check(get(locations.root / "settings.txt") == "old settings", "migration copies absent settings");
    check(get(locations.root / "migrated-from.txt") == source.string() + "\n", "migration provenance receipt");
    check(App::migrationSource(locations, home, App::readAppConfig(locations)).empty(), "migration asks only once after copy");
#ifdef __APPLE__
    char copy[64]{};
    check(getxattr((locations.root / "NAND/save.bin").c_str(), "com.petari.nand", copy, sizeof(copy), 0, 0) == sizeof(metadata), "NAND extended attributes survive copy");
#endif
    const App::AppLocations declined{root / "declined"};
    config.clear();
    App::migrateUserState(declined, source, false, &config);
    check(!fs::exists(declined.root / "NAND") && App::migrationSource(declined, home, App::readAppConfig(declined)).empty(), "declined migration is recorded without copying");
    const App::AppLocations existing{root / "existing"};
    fs::create_directories(existing.root / "NAND");
    check(App::migrationSource(existing, home, {}).empty(), "existing NAND is never migrated over");
    const auto brokenSource = root / "incomplete-source";
    put(brokenSource / "NAND/first.bin");
    fs::create_symlink(root / "outside-save", brokenSource / "NAND/linked.bin");
    const App::AppLocations failedCopy{root / "failed-copy"};
    config.clear();
    bool rejected = false;
    try { App::migrateUserState(failedCopy, brokenSource, true, &config); } catch (...) { rejected = true; }
    check(rejected && !fs::exists(failedCopy.root / "NAND"), "a failed copy never publishes partial NAND");
    for (const auto& entry : fs::directory_iterator(failedCopy.root)) check(entry.path().filename() == "app.cfg", "failed migration removes staging files");
    fs::remove_all(source);
    check(App::migrationSource({root / "legacy-destination"}, home, {}) == legacy, "legacy default root is also discoverable");
}
void artifactTests(const fs::path& root) {
    const App::AppLocations locations{root / "artifacts"};
    for (int i = 0; i < 25; ++i) {
        char stamp[32]; std::snprintf(stamp, sizeof(stamp), "20261022-1200%02d", i);
        const auto log = locations.logs() / (std::string("astral-") + stamp + ".log");
        const auto tape = locations.tapes() / (std::string("session-") + stamp);
        put(log); put(tape / "route.kpd");
        fs::last_write_time(log, fs::file_time_type::clock::now() + std::chrono::seconds(i));
        fs::last_write_time(tape, fs::file_time_type::clock::now() + std::chrono::seconds(i));
    }
    put(locations.logs() / "keep.log"); put(locations.tapes() / "my-kept-tape/route.kpd");
    fs::create_directory_symlink(root / "outside", locations.tapes() / "session-20000101-000000-link");
    App::pruneRunArtifacts(locations.logs(), false, 20);
    App::pruneRunArtifacts(locations.tapes(), true, 10);
    check(!fs::exists(locations.logs() / "astral-20261022-120004.log") && fs::exists(locations.logs() / "astral-20261022-120005.log"), "logs keep the newest 20");
    check(!fs::exists(locations.tapes() / "session-20261022-120014") && fs::exists(locations.tapes() / "session-20261022-120015"), "tapes keep the newest 10");
    check(fs::exists(locations.logs() / "keep.log") && fs::exists(locations.tapes() / "my-kept-tape/route.kpd") && fs::is_symlink(locations.tapes() / "session-20000101-000000-link"), "retention ignores unrelated names and symlinks");
    setenv("PETARI_CAPTURE_DIR", "explicit-captures", 1);
    setenv("PETARI_TAPE_RECORD", "explicit-tape", 1);
    App::prepareRunArtifacts(locations);
    check(std::string(std::getenv("PETARI_CAPTURE_DIR")) == "explicit-captures" && std::string(std::getenv("PETARI_TAPE_RECORD")) == "explicit-tape", "explicit capture and tape variables win");
    unsetenv("PETARI_CAPTURE_DIR"); unsetenv("PETARI_TAPE_RECORD");
    setenv("ASTRAL_TAPE", "off", 1);
    App::prepareRunArtifacts(locations);
    check(!std::getenv("PETARI_TAPE_RECORD") && fs::is_directory(locations.captures()), "ASTRAL_TAPE off suppresses the default tape");
    unsetenv("ASTRAL_TAPE");
    setenv("PETARI_ROUTE_RECORD", "explicit-route.csv", 1);
    App::prepareRunArtifacts(locations);
    check(!std::getenv("PETARI_TAPE_RECORD"), "explicit full-route recording is not displaced by default tape");
    unsetenv("PETARI_ROUTE_RECORD");
    App::prepareRunArtifacts(locations);
    check(fs::is_directory(std::getenv("PETARI_TAPE_RECORD")), "default tape directory is created at startup");
    unsetenv("PETARI_CAPTURE_DIR"); unsetenv("PETARI_TAPE_RECORD");
    check(App::redirectAppLog(locations).empty(), "existing ctest log redirection is preserved");
    const pid_t child = fork();
    if (child == 0) {
        std::freopen("/dev/null", "w", stdout); std::freopen("/dev/null", "w", stderr);
        try {
            const auto path = App::redirectAppLog({root / "finder"});
            std::puts("stdout receipt"); std::fputs("stderr receipt\n", stderr);
            std::fflush(nullptr);
            _exit(path.empty() ? 1 : 0);
        } catch (...) { _exit(2); }
    }
    int status = -1;
    if (child > 0) waitpid(child, &status, 0);
    check(child > 0 && WIFEXITED(status) && WEXITSTATUS(status) == 0, "Finder-style descriptors get a log");
    std::string logged;
    for (const auto& entry : fs::directory_iterator(root / "finder/logs")) logged += get(entry.path());
    check(logged.find("stdout receipt") != std::string::npos && logged.find("stderr receipt") != std::string::npos, "Finder log contains stdout and stderr");
}
}
int main() {
    char pattern[] = "/tmp/astral-first-run-tests-XXXXXX";
    const auto temp = mkdtemp(pattern);
    if (!temp) return 2;
    const fs::path root(temp);
    try { configTests(root); discTests(root); migrationTests(root); artifactTests(root); }
    catch (const std::exception& exception) { std::fprintf(stderr, "FAIL: exception: %s\n", exception.what()); ++failures; }
    fs::remove_all(root);
    std::printf("app first-run tests: %s (%d failures)\n", failures ? "FAIL" : "PASS", failures);
    return failures ? 1 : 0;
}
