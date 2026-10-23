#include "display_settings.hpp"

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
    settings.fullscreen = true; settings.size = 3; settings.vsync = true;
    check(DS::load(file, &settings, &error) && error.empty(), "missing file is normal");
    check(!settings.fullscreen && settings.size == 0 && !settings.vsync, "unchanged windowed/vsync-off defaults");
    check(HM::kDisplaySizes[settings.size].width == 1280 && HM::kDisplaySizes[settings.size].height == 720,
          "unchanged 1280x720 default");
    settings = {true, 3, true};
    check(DS::save(file, settings, &error), "create user folder and save");
    check(DS::load(file, &settings, &error) && settings.fullscreen && settings.size == 3 && settings.vsync, "round trip");
    settings = {false, -20, false};
    check(DS::save(file, settings, &error) && DS::load(file, &settings, &error) && settings.size == 0, "save clamps low");
    { std::ofstream out(file); out << "size=999\r\nfullscreen=1\r\nvsync=0\r\nfuture=anything\n"; }
    check(DS::load(file, &settings, &error) && settings.size == 3 && settings.fullscreen, "load clamps high; CRLF and unknown keys");
    for (const char* invalid : {"fullscreen=2", "vsync=-1", "size=2147483648", "size=", "size=2junk", "broken", "vsync=1\nsize=bad"}) {
        { std::ofstream out(file); out << invalid << '\n'; }
        check(!DS::load(file, &settings, &error) && !error.empty(), "malformed file reports action");
        check(!settings.fullscreen && settings.size == 0 && !settings.vsync, "malformed file never partially applies");
    }
    { std::ofstream out(file); out << "# preferences\n\nsize=-99\n"; }
    check(DS::load(file, &settings, &error) && settings.size == 0 && error.empty(), "negative size clamps low");
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
    check(window.full && window.fullscreenCalls == 1 && !controller.settings().vsync, "one pending request; host applies fullscreen");
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
    check(controller.settings().vsync && controller.settings().restartNeeded && window.fullscreenCalls == 1,
          "vsync is persisted only; needs relaunch");
    DS::Controller relaunched;
    check(relaunched.configure(file, &error) && relaunched.settings().vsync && !relaunched.settings().restartNeeded,
          "new launch consumes vsync and clears restart note");
    controller.change(2, 1); controller.tick(window);
    check(!controller.settings().restartNeeded, "reverting vsync clears restart note");
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
    check(settings.vsync && std::string(message).find("Could not save") != std::string::npos, "save failure shown in menu");
    std::filesystem::remove_all(root);
    std::printf("display settings tests passed (%d checks)\n", checks);
}
