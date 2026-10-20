// Synthetic catalogs exercise the menu without SDK, wrapper, disc or save data.
#include <petari/home_menu.hpp>
#include <petari/launch_stage.hpp>
#include <petari/progress.hpp>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

namespace HM = PetariNative::HomeMenu;
namespace LS = PetariNative::App::LaunchStage;
namespace {
void check(bool ok, const char* label) {
    if (!ok) { std::fprintf(stderr, "FAIL: %s\n", label); std::exit(1); }
}
void press(HM::Menu& menu, std::uint32_t button) {
    HM::FrameInput input;
    input.hold = input.trigger = button;
    menu.update(input);
}
HM::Menu openMenu() {
    HM::Menu menu;
    menu.create({});
    menu.open();
    for (int i = 0; i < 10; ++i) menu.update({});
    check(menu.phase() == HM::Phase::List, "menu opens");
    return menu;
}
HM::Menu openLevels() {
    auto menu = openMenu();
    press(menu, HM::Button::Down); press(menu, HM::Button::Down); press(menu, HM::Button::A);
    press(menu, HM::Button::Down); press(menu, HM::Button::A);
    check(menu.phase() == HM::Phase::Levels, "Level Select opens with no folder mods");
    return menu;
}
void go(HM::Menu& menu) {
    press(menu, HM::Button::Down); press(menu, HM::Button::Down); press(menu, HM::Button::A);
    for (int i = 0; i < 10; ++i) menu.update({});
    check(menu.phase() == HM::Phase::Finished, "Go finishes closing");
}
}  // namespace

int main() {
    auto unavailable = openMenu();
    HM::ModsEntry pending;
    std::strcpy(pending.label, "Cappy");
    pending.on = true;
    pending.available = false;
    unavailable.setMods(&pending, 1);
    press(unavailable, HM::Button::Down); press(unavailable, HM::Button::Down); press(unavailable, HM::Button::A);
    check(!unavailable.view().items[0].enabled &&
          std::strcmp(unavailable.view().items[0].label, "Cappy (not yet ported)") == 0,
          "unported mod is greyed out without an On/Off promise");
    press(unavailable, HM::Button::A);
    check(unavailable.takeModToggles() == 0, "unported mod cannot emit a toggle");

    auto empty = openLevels();
    check(std::strstr(empty.view().items[0].label, "No SMG2 catalog") != nullptr, "empty catalog is explicit");
    press(empty, HM::Button::Left); press(empty, HM::Button::Right);
    go(empty);
    const char* stage = "unchanged";
    int scenario = -1;
    check(!empty.takeLevelRequest(&stage, &scenario) && scenario == -1 && std::strcmp(stage, "unchanged") == 0,
          "empty Go does not emit or overwrite a destination");

    std::string error;
    check(LS::installCatalog({{"MenuSyntheticGalaxy", "Menu Fixture Galaxy", 3,
        {{2, 1, "Normal", "", ""}, {7, 2, "Hidden", "", ""}, {12, 4, "Green", "", ""},
         {15, 8, "Normal", "SyntheticComet", ""}}, "synthetic menu test; NOT disc inventory"},
        {"WorldMap08Galaxy", "", 8, {{1, 0, "", "", ""}}, "synthetic nameless world; NOT disc inventory"}}, &error), "synthetic catalog installs");
    auto menu = openLevels();
    press(menu, HM::Button::Right);
    check(std::strstr(menu.view().items[0].label, "Menu Fixture Galaxy") != nullptr,
          "Level Select skips nameless world8 when cycling galaxies");
    check(std::strcmp(menu.view().items[1].label, "Mission 2 (1 of 4)") == 0 &&
          std::strstr(menu.view().message, "World 3") != nullptr, "first menu row uses actual ID and world");
    press(menu, HM::Button::Down);
    press(menu, HM::Button::Right);
    check(std::strcmp(menu.view().items[1].label, "Mission 7 (2 of 4) (hidden star)") == 0, "hidden metadata follows actual ID");
    press(menu, HM::Button::Right);
    check(std::strcmp(menu.view().items[1].label, "Mission 12 (3 of 4) (green star)") == 0, "green metadata uses complete multi-digit ID");
    press(menu, HM::Button::Right);
    check(std::strcmp(menu.view().items[1].label, "Mission 15 (4 of 4) (comet)") == 0, "comet metadata uses complete multi-digit ID");
    press(menu, HM::Button::Right);
    check(menu.levelMission() == 1 && std::strstr(menu.view().items[1].label, "Mission 2 ") != nullptr, "ordinal wraps to actual first ID");
    press(menu, HM::Button::Left);
    press(menu, HM::Button::Left);  // Selected ordinal 3 = actual ScenarioNo 12.
    check(!menu.takeLevelRequest(&stage, &scenario), "selection alone is not a request");
    press(menu, HM::Button::Down); press(menu, HM::Button::A);
    for (int i = 0; i < 10; ++i) menu.update({});
    check(menu.takeLevelRequest(&stage, &scenario) && std::strcmp(stage, "MenuSyntheticGalaxy") == 0 && scenario == 12,
          "Go reports selected actual ScenarioNo exactly once");
    check(!menu.takeLevelRequest(&stage, &scenario), "Go is consumed once");

    namespace PR = PetariNative::Progress;
    PR::resetForTesting();  // No user path: in-memory synthetic records only.
    check(PR::parse(R"({"version":1,"missions":{"MenuSyntheticGalaxy:12":{"stage":"MenuSyntheticGalaxy","mission":12,"clears":1,"best_time_s":5}}})", &error), "synthetic personal record parses");
    auto progress = openMenu();
    for (int i = 0; i < 3; ++i) press(progress, HM::Button::Down);
    press(progress, HM::Button::A);
    const auto view = progress.view();
    check(view.lineCount == 5 && std::strcmp(view.lines[3].action, "Mission 12 (green star)") == 0 &&
          std::strstr(view.lines[3].inputs, "Cleared") != nullptr &&
          std::strcmp(view.lines[4].action, "Mission 15 (comet)") == 0,
          "personal progress indexes actual sparse IDs with raw metadata tags");
    PR::resetForTesting();
    std::puts("synthetic level-select menu tests PASS");
    return 0;
}
