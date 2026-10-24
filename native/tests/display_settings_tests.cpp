#include "display_settings.hpp"
#include <petari/ubershader_mode.hpp>

#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <string>
#include <unistd.h>

namespace DS = PetariNative::App::DisplaySettings;
namespace HM = PetariNative::HomeMenu;
namespace {
int checks = 0;
void check(bool ok, const char* label) {
    ++checks;
    if (!ok) { std::fprintf(stderr, "FAIL: %s\n", label); std::exit(1); }
}
class FakeWindow final : public DS::Window {
public:
    bool full = false, refuse = false;
    int fullscreenCalls = 0, sizeCalls = 0, width = 0, height = 0;
    bool fullscreen() override { return full; }
    bool setFullscreen(bool on) override {
        ++fullscreenCalls;
        if (refuse) return false;
        full = on;
        return true;
    }
    bool setSize(int w, int h) override {
        ++sizeCalls;
        if (refuse) return false;
        width = w; height = h;
        return true;
    }
};
}
int main() {
    char directory[] = "/tmp/petari-display-XXXXXX";
    check(mkdtemp(directory) != nullptr, "temporary directory");
    const std::filesystem::path root(directory), file = root / "user/display.txt";
    std::string error;
    DS::Settings settings;
    settings.fullscreen = true; settings.size = 3; settings.vsync = false; settings.vsyncAuto = false;
    check(DS::load(file, &settings, &error) && error.empty(), "missing file is normal");
    check(!settings.fullscreen && settings.size == 0 && settings.vsyncAuto && settings.vsync, "windowed defaults, VSync Auto by default");
    check(settings.ubershaders && !settings.ubershadersSaved, "fresh display defaults to hybrid ubershaders");
    namespace Uber = PetariNative::Ubershaders;
    const auto mode = [](const char* env, std::optional<bool> saved, Uber::Mode expected, const char* source) {
        const auto selected = Uber::resolve(env, saved);
        return selected.mode == expected && std::string(selected.source) == source;
    };
    check(mode(nullptr, std::nullopt, Uber::Mode::Hybrid, "default"), "mode default is hybrid");
    check(mode(nullptr, false, Uber::Mode::Off, "setting"), "saved off disables hybrid");
    check(mode(nullptr, true, Uber::Mode::Hybrid, "setting"), "saved on enables hybrid");
    check(mode("0", true, Uber::Mode::Off, "env"), "environment zero wins over saved on");
    check(mode("0", std::nullopt, Uber::Mode::Off, "env"), "environment zero wins over default on");
    check(mode("1", false, Uber::Mode::Hybrid, "env"), "environment one wins over saved off");
    check(mode("all", false, Uber::Mode::All, "env"), "diagnostic all wins over saved off");
    check(mode("", true, Uber::Mode::Off, "env") && mode("invalid", true, Uber::Mode::Off, "env"),
          "other explicit environment values retain prior off behavior");
    check(HM::kDisplaySizes[settings.size].width == 1280 && HM::kDisplaySizes[settings.size].height == 720,
          "unchanged 1280x720 default");
    settings = {true, 3, true};
    check(DS::save(file, settings, &error), "create user folder and save");
    check(DS::load(file, &settings, &error) && settings.fullscreen && settings.size == 3 && settings.vsync, "round trip");
    settings = {true, 3, false}; settings.vsyncAuto = false;
    check(DS::save(file, settings, &error) && DS::load(file, &settings, &error) && !settings.vsyncAuto && !settings.vsync, "VSync Off round trips (vsyncmode=0)");
    settings.vsync = true;
    check(DS::save(file, settings, &error) && DS::load(file, &settings, &error) && !settings.vsyncAuto && settings.vsync, "VSync On round trips (vsyncmode=1)");
    settings.vsyncAuto = true;
    check(DS::save(file, settings, &error) && DS::load(file, &settings, &error) && settings.vsyncAuto, "VSync Auto round trips (vsyncmode=2)");
    { std::ofstream out(file); out << "fullscreen=0\nsize=1\nvsync=0\nubershaders=1\n"; }
    check(DS::load(file, &settings, &error) && settings.size == 1 && settings.vsyncAuto, "a 5.3-era vsync=0 is ignored: the default Auto applies");
    { std::ofstream out(file); out << "fullscreen=0\nsize=1\nvsync2=0\nubershaders=1\n"; }
    check(DS::load(file, &settings, &error) && !settings.vsyncAuto && !settings.vsync, "a 5.4 explicit vsync2=0 stays Off");
    { std::ofstream out(file); out << "fullscreen=0\nsize=1\nvsync2=1\nubershaders=1\n"; }
    check(DS::load(file, &settings, &error) && settings.vsyncAuto, "a 5.4 vsync2=1 (its default) becomes Auto");
    { std::ofstream out(file); out << "vsync2=0\nvsyncmode=1\n"; }
    check(DS::load(file, &settings, &error) && !settings.vsyncAuto && settings.vsync, "vsyncmode wins over vsync2");
    settings.ubershaders = false;
    check(DS::save(file, settings, &error) && DS::load(file, &settings, &error) && !settings.ubershaders && settings.ubershadersSaved,
          "ubershaders off round trips with explicit saved marker");
    settings = {false, -20, false};
    check(DS::save(file, settings, &error) && DS::load(file, &settings, &error) && settings.size == 0, "save clamps low");
    { std::ofstream out(file); out << "size=999\r\nfullscreen=1\r\nvsync=0\r\nfuture=anything\n"; }
    check(DS::load(file, &settings, &error) && settings.size == 3 && settings.fullscreen, "load clamps high; CRLF and unknown keys");
    for (const char* invalid : {"fullscreen=2", "vsync=-1", "vsyncmode=3", "vsyncmode=-1", "ubershaders=2", "ubershaders=-1", "ubershaders=", "size=2147483648", "size=", "size=2junk", "broken", "vsync=1\nsize=bad"}) {
        { std::ofstream out(file); out << invalid << '\n'; }
        check(!DS::load(file, &settings, &error) && !error.empty(), "malformed file reports action");
        check(!settings.fullscreen && settings.size == 0 && settings.vsyncAuto, "malformed file never partially applies");
    }
    { std::ofstream out(file); out << "# preferences\n\nsize=-99\n"; }
    check(DS::load(file, &settings, &error) && settings.size == 0 && error.empty(), "negative size clamps low");
    check(settings.ubershaders && !settings.ubershadersSaved, "old display files keep the default-on mode source");
    check(!DS::save(root / "user", {}, &error) && !error.empty(), "rename failure is reported");
    { std::ofstream out(root / "block"); out << "file"; }
    check(!DS::save(root / "block/display.txt", {}, &error), "unwritable parent is reported");
    for (const auto& entry : std::filesystem::directory_iterator(root)) {
        check(entry.path().filename().string().find(".tmp-") == std::string::npos, "failed atomic write cleans sibling");
    }

    check(DS::save(file, {}, &error), "reset fixture");
    DS::Controller controller;
    check(controller.configure(file, &error), "controller loads");
    FakeWindow window;
    controller.change(0, 1);
    check(window.fullscreenCalls == 0 && controller.settings().busy, "menu queues without touching window");
    controller.change(2, 1);
    controller.tick(window);
    check(window.full && window.fullscreenCalls == 1 && controller.settings().vsync, "one pending request; host applies fullscreen");
    check(DS::load(file, &settings, &error) && settings.fullscreen, "accepted fullscreen persists");
    controller.change(1, 1); controller.tick(window);
    check(window.sizeCalls == 0 && controller.settings().size == 0, "fullscreen prevents resizing");
    window.full = false; controller.tick(window);
    check(!controller.settings().fullscreen && DS::load(file, &settings, &error) && !settings.fullscreen,
          "OS shortcut state is observed and saved");
    controller.change(1, -1); controller.tick(window);
    check(window.width == 2560 && window.height == 1440 && controller.settings().size == 3, "left wraps to largest preset");
    controller.change(1, 1); controller.tick(window);
    check(window.width == 1280 && window.height == 720, "right wraps to smallest preset");
    controller.change(2, 1); controller.tick(window);
    check(!controller.settings().vsyncAuto && controller.settings().vsync && controller.settings().restartNeeded && window.fullscreenCalls == 1,
          "VSync Auto -> On is persisted only; needs relaunch");
    DS::Controller relaunched;
    check(relaunched.configure(file, &error) && !relaunched.settings().vsyncAuto && relaunched.settings().vsync &&
          !relaunched.startupVsyncAuto() && !relaunched.settings().restartNeeded,
          "new launch consumes vsync and clears restart note");
    controller.change(2, 1); controller.tick(window);
    check(!controller.settings().vsyncAuto && !controller.settings().vsync && controller.settings().restartNeeded, "On -> Off");
    controller.change(2, 1); controller.tick(window);
    check(controller.settings().vsyncAuto && !controller.settings().restartNeeded, "Off -> Auto: back to the launch mode clears the restart note");
    controller.change(2, -1); controller.tick(window);
    check(!controller.settings().vsyncAuto && !controller.settings().vsync, "Left goes Auto -> Off");
    controller.change(2, 1); controller.tick(window);
    check(controller.settings().vsyncAuto && controller.startupVsyncAuto(), "and Right back to Auto");
    const int fullCalls = window.fullscreenCalls, resizeCalls = window.sizeCalls;
    controller.change(3, 1); controller.tick(window);
    check(!controller.settings().ubershaders && controller.settings().ubershadersRestartNeeded,
          "ubershader toggle persists off and requires a new launch");
    check(window.fullscreenCalls == fullCalls && window.sizeCalls == resizeCalls, "ubershader toggle does not change the live window");
    check(DS::load(file, &settings, &error) && !settings.ubershaders && settings.ubershadersSaved, "toggle saved in display.txt");
    DS::Controller newLaunch;
    check(newLaunch.configure(file, &error) && !newLaunch.settings().ubershaders && !newLaunch.settings().ubershadersRestartNeeded,
          "new launch consumes saved off preference");
    controller.change(3, -1); controller.tick(window);
    check(controller.settings().ubershaders && !controller.settings().ubershadersRestartNeeded,
          "reverting ubershaders clears its restart note");
    window.refuse = true;
    controller.change(0, 1); controller.tick(window);
    char message[192]; controller.read(&settings, message, sizeof(message));
    check(!settings.fullscreen && !settings.busy && std::string(message).find("Try again") != std::string::npos,
          "refused fullscreen remains off and offers action");
    controller.change(1, 1); controller.tick(window);
    check(controller.settings().size == 0, "refused resize does not claim a new preset");
    window.refuse = false;
    std::filesystem::remove(file);
    std::filesystem::create_directory(file);
    controller.change(2, 1); controller.tick(window);
    controller.read(&settings, message, sizeof(message));
    check(!settings.vsyncAuto && settings.vsync && std::string(message).find("Could not save") != std::string::npos, "save failure shown in menu");
    std::filesystem::remove_all(root);
    std::printf("display settings tests passed (%d checks)\n", checks);
}
