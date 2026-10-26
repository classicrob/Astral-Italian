// No game, disc, host events or user cache. Exercise the registry and the SAME
// supported row adapter used by hbm_bridge, including an actual Menu toggle.
#include <petari/home_menu.hpp>
#include <petari/mods.hpp>
#include <petari/mods_menu.hpp>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <unistd.h>

namespace M = PetariNative::Mods;
namespace HM = PetariNative::HomeMenu;
namespace {
int checks = 0;
void check(bool ok, const char* label) {
    ++checks;
    if (!ok) { std::fprintf(stderr, "FAIL: %s\n", label); std::exit(1); }
}
struct Env {
    const char* key;
    bool had;
    std::string before;
    explicit Env(const char* name) : key(name), had(std::getenv(name) != nullptr),
        before(had ? std::getenv(name) : "") { ::unsetenv(key); }
    ~Env() { if (had) ::setenv(key, before.c_str(), 1); else ::unsetenv(key); }
};
template<class Call> void refused(Call call) {
    std::FILE* log = std::tmpfile();
    check(log != nullptr, "private refusal capture");
    std::fflush(stderr);
    const int original = ::dup(STDERR_FILENO);
    check(original >= 0 && ::dup2(::fileno(log), STDERR_FILENO) >= 0, "capture stderr");
    call();
    std::fflush(stderr);
    check(::dup2(original, STDERR_FILENO) >= 0, "restore stderr");
    ::close(original);
    std::rewind(log);
    char text[512] = {};
    const auto bytes = std::fread(text, 1, sizeof(text) - 1, log);
    std::fclose(log);
    check(bytes > 0 && std::strstr(text, "SkipCutscenes enable refused: unavailable in SMG2"), "enable refusal has the product reason");
    check(!M::enabled(M::Mod::SkipCutscenes), "refused enable remains off");
}
void press(HM::Menu& menu, std::uint32_t button) {
    HM::FrameInput input; input.hold = input.trigger = button; menu.update(input);
}
}
int main() {
    static_assert(static_cast<int>(M::Mod::SkipCutscenes) == 5 &&
                  static_cast<int>(M::Mod::PointToMove) == 6 &&
                  static_cast<int>(M::Mod::LevelEditor) == 10 &&
                  static_cast<int>(M::Mod::Count) == 11, "raw-tape mod IDs must not shift");
    Env mods("PETARI_MODS"), odyssey("PETARI_ODYSSEY_MOVEMENT");
    M::resetForTesting();
    check(!M::supported(M::Mod::SkipCutscenes), "donor mod excluded from supported registry");
    check(!M::supported(M::Mod::Count) && !M::supported(static_cast<M::Mod>(-1)), "invalid IDs unsupported");
    check(M::supportedCount() == 10 && M::supportedAt(-1) == M::Mod::Count &&
          M::supportedAt(10) == M::Mod::Count, "bounded supported registry");
    int row = 0;
    for (int id = 0; id < static_cast<int>(M::Mod::Count); ++id) {
        const auto mod = static_cast<M::Mod>(id);
        if (mod == M::Mod::SkipCutscenes) continue;
        check(M::supported(mod) && M::supportedAt(row++) == mod, "all other mods retain order/support");
        M::setEnabled(mod, true); check(M::enabled(mod), "other mod enable unchanged");
        M::setEnabled(mod, false); check(!M::enabled(mod), "other mod disable unchanged");
    }
    refused([] { M::setEnabled(M::Mod::SkipCutscenes, true); });
    M::setEnabled(M::Mod::SkipCutscenes, false);
    std::string error;
    refused([&] { check(M::parse("SkipCutscenes=on\nCappy=on\nPointToMove=on\n", &error), "valid stale settings parse succeeds"); });
    check(M::enabled(M::Mod::Cappy) && M::enabled(M::Mod::PointToMove), "parse preserves other valid enables");
    check(!M::parse("Cappy=off\nUnknown=on\n", &error) && M::enabled(M::Mod::Cappy), "invalid file remains atomic");

    auto pattern = (std::filesystem::temp_directory_path() / "petari-supported-mods-XXXXXX").string();
    const char* created = ::mkdtemp(pattern.data());
    check(created != nullptr, "private settings root");
    const std::filesystem::path root(created), file = root / "mods.txt";
    { std::ofstream out(file); out << "SkipCutscenes=on\nCappy=on\nPointToMove=on\nFolder.Test=on\n"; }
    M::resetForTesting();
    refused([&] { check(M::load(file, &error), "stale file does not block startup"); });
    check(M::enabled(M::Mod::Cappy) && M::enabled(M::Mod::PointToMove) && M::enabled(M::Mod::OdysseyJumpDive),
          "file/default settings survive refused enable");
    check(M::save(&error), "save valid settings");
    std::ifstream saved(file); const std::string text((std::istreambuf_iterator<char>(saved)), std::istreambuf_iterator<char>()); saved.close();
    check(text.find("SkipCutscenes=off\n") != std::string::npos && text.find("Cappy=on\n") != std::string::npos &&
          text.find("Folder.Test=on\n") != std::string::npos, "save keeps compatibility key off and other settings intact");
    ::setenv("PETARI_MODS", "SkipCutscenes,GenericWorld,-OdysseyJumpDive", 1);
    refused([&] { check(M::load(file, &error), "env unsupported enable does not block startup"); });
    check(M::enabled(M::Mod::GenericWorld) && !M::enabled(M::Mod::Cappy) && !M::enabled(M::Mod::OdysseyJumpDive),
          "env precedence for supported mods unchanged");
    ::unsetenv("PETARI_MODS");

    M::resetForTesting();
    HM::Menu menu; menu.create({}); HM::refreshSupportedMods(menu);
    check(menu.mods()[3].needs == 2 && menu.mods()[4].needs == 2, "dependencies map to supported rows");
    menu.open(); for (int i = 0; i < 10; ++i) menu.update({});
    press(menu, HM::Button::Down); press(menu, HM::Button::Down); press(menu, HM::Button::A);
    check(menu.phase() == HM::Phase::Mods, "actual Mods page opens");
    for (int i = 0; i < M::supportedCount(); ++i) {
        check(std::strstr(menu.view().items[i].label, "skip cutscenes") == nullptr, "HBM supported list hides donor row");
    }
    check(std::strstr(menu.view().items[5].label, "Point to move"), "row after removed donor is PointToMove");
    for (int i = 0; i < 5; ++i) press(menu, HM::Button::Down);
    press(menu, HM::Button::A);
    check(HM::applySupportedModToggles(menu), "HBM row toggle consumed");
    for (int id = 0; id < static_cast<int>(M::Mod::Count); ++id) {
        check(M::enabled(static_cast<M::Mod>(id)) == (id == static_cast<int>(M::Mod::PointToMove)),
              "row after hidden donor toggles ONLY correct stable mod ID");
    }
    check(!HM::applySupportedModToggles(menu), "toggle consumed exactly once");
    // Also cover the last supported row: compaction must not lose its mapping.
    for (int i = 5; i < 9; ++i) press(menu, HM::Button::Down);
    press(menu, HM::Button::A);
    check(HM::applySupportedModToggles(menu) && M::enabled(M::Mod::LevelEditor), "last row keeps its stable ID");
    check(!M::enabled(M::Mod::SkipCutscenes), "menu never enables donor ID");
    M::resetForTesting();
    std::filesystem::remove(file); std::filesystem::remove(root);
    std::printf("supported SMG2 mods tests passed (%d checks)\n", checks);
}
